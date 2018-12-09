//
// Created by bilin on 11/29/18.
//


#include "../include/connection_manager.h"
#include "../include/global.h"
#include "../include/control_handler.h"
#include "../include/data_handler.h"

#include <vector>

using namespace std;

int control_socket;
extern vector<Routing> table;
struct timeval tv;

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
        selret = select(head_fd + 1, &watch_list, NULL, NULL, NULL);

        if (selret < 0) ERROR("select failed.");

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
                    //call handler that will call recvfrom() .....
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