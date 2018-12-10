//
// Created by bilin on 11/29/18.
//


#include "../include/connection_manager.h"
#include "../include/global.h"
#include "../include/control_handler.h"
#include "../include/data_handler.h"
#include "../include/utils.h"

#include <vector>

using namespace std;

int control_socket;

struct timeval tv;
struct timeval next_send_time;
bool first_time = true;

void main_loop();

fd_set master_list, watch_list;
int head_fd;

void run(uint16_t control_port) {
    control_socket = create_control_sock(control_port);

    //router_socket and data_socket will be initialized after INIT from controller

    FD_ZERO(&master_list);
    FD_ZERO(&watch_list);

    /* Register the control socket */
    FD_SET(control_socket, &master_list);
    head_fd = control_socket;

    main_loop();
}


void main_loop() {
    int selret, sock_index, fdaccept;

    while (true) {
        watch_list = master_list;
        if (first_time) {
            selret = select(head_fd + 1, &watch_list, NULL, NULL, NULL);
        } else {
            selret = select(head_fd + 1, &watch_list, NULL, NULL, &tv);
        }

        if (selret < 0) ERROR("select failed.");
        if (selret == 0) { // timeout, send or disconnect
            struct timeval cur;
            gettimeofday(&cur, NULL);
            struct timeval diff;
            diff = diff_tv(cur, next_send_time);
            if (diff.tv_sec >= 0 || diff.tv_usec >= 0) {
                next_send_time.tv_sec = cur.tv_sec + time_peroid;
                next_send_time.tv_usec = cur.tv_usec;
                send_dv();
                tv = diff_tv(next_send_time, cur);
            }
            // don't forget update tv

            // next, router expired setting
            for (int i = 0; i < routers_timeout.size(); i++) {
                if (!routers_timeout[i].is_connected) { // ignore disconnected routers
                    continue;
                }
                diff = diff_tv(cur, routers_timeout[i].expired_time);
                if (diff.tv_sec >= 0 || diff.tv_usec >= 0) { // some routers timeout
                    routers_timeout[i].is_connected = false;
                    // update routing table
                    for (int j = 0; j < table.size(); j++) {
                        if (table[j].dest_id == routers_timeout[i].router_id) {
                            table[j].dest_cost = INF;
                            table[j].next_hop_id = INF;
                            break;
                        }
                    }
                    remove_route_conn(routers_timeout[i].router_id);
                } else { // choose tv=min(expected-cur)
                    diff = diff_tv(tv, diff_tv(routers_timeout[i].expired_time, cur));
                    if (diff.tv_sec >= 0 || diff.tv_usec >= 0) {
                        tv = diff_tv(routers_timeout[i].expired_time, cur);
                    }
                }
            }
            continue;
        }
        /* Loop through file descriptors to check which ones are ready */
        for (sock_index = 0; sock_index <= head_fd; sock_index += 1) {

            if (FD_ISSET(sock_index, &watch_list)) {

                /* control_socket */
                if (sock_index == control_socket) {// TCP, need to create a new connection
                    fdaccept = new_control_conn(sock_index);

                    /* Add to watched socket list */
                    FD_SET(fdaccept, &master_list);
                    if (fdaccept > head_fd) head_fd = fdaccept;
                }

                    /* router_socket */
                else if (sock_index == router_socket) {
                    // UDP, don't need to create connect link, directly update routing table
                    // call handler that will call recvfrom() .....
                    update_routing_table(sock_index);
                }

                    /* data_socket */
                else if (sock_index == data_socket) {// TCP, need to create link
                    fdaccept = new_data_conn(sock_index);

                    /* Add to watched socket list */
                    FD_SET(fdaccept, &master_list);
                    if (fdaccept > head_fd) head_fd = fdaccept;
                }

                    /* Existing connection */
                else {
                    // sockfd is either link to controller(control message), or link to another router(switch datagram)
                    if (isControl(sock_index)) {
                        if (!control_recv_hook(sock_index)) FD_CLR(sock_index, &master_list);
                    } else if (isData(sock_index)) {
                        // when switch datagram, first check if connection exist, if not, connect() first, then send()
                        if (!data_recv_hook(sock_index)) FD_CLR(sock_index, &master_list);

                    } else {
                        ERROR("Unknown socket index");
                    }
                }
            }
        }
    }
}