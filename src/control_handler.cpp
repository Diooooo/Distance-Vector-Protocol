//
// Created by bilin on 11/29/18.
//

#include "../include/control_handler.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <strings.h>
#include <vector>
#include <unistd.h>
#include <string.h>
#include <algorithm>
#include "../include/global.h"
#include "../include/header.h"
#include "../include/utils.h"
#include "../include/connection_manager.h"
#include "../include/data_handler.h"

using namespace std;

vector<int> control_socket_list;
vector<Routing> table;
int router_socket;
uint32_t my_ip;
uint16_t my_router_port;
uint16_t my_data_port;
uint16_t my_id;
int create_control_sock(uint16_t control_port) {
    int sock;
    struct sockaddr_in control_addr;
    socklen_t addrlen = sizeof(control_addr);

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) ERROR("socket() failed");

    /* Make socket re-usable */
//    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (int[]) {1}, sizeof(int)) < 0) ERROR("setsockopt() failed");

    bzero(&control_addr, sizeof(control_addr));

    control_addr.sin_family = AF_INET;
    control_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    control_addr.sin_port = htons(control_port);

    if (bind(sock, (struct sockaddr *) &control_addr, sizeof(control_addr)) < 0) ERROR("bind() failed");

    if (listen(sock, 5) < 0) ERROR("listen() failed");

    return sock;
}

int create_route_sock(uint16_t router_port){
    int sock;
    struct sockaddr_in control_addr;
    socklen_t addrlen = sizeof(control_addr);

    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) ERROR("socket() failed");

    /* Make socket re-usable */
//    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (int[]) {1}, sizeof(int)) < 0) ERROR("setsockopt() failed");

    bzero(&control_addr, sizeof(control_addr));

    control_addr.sin_family = AF_INET;
    control_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    control_addr.sin_port = htons(router_port);

    if (bind(sock, (struct sockaddr *) &control_addr, sizeof(control_addr)) < 0) ERROR("bind() failed");

    return sock;
}

int new_control_conn(int sock_index) {
    int fdaccept, caddr_len;
    struct sockaddr_in remote_controller_addr;

    caddr_len = sizeof(remote_controller_addr);
    fdaccept = accept(sock_index, (struct sockaddr *) &remote_controller_addr, (socklen_t *) &caddr_len);
    if (fdaccept < 0) ERROR("accept() failed");

    /* Insert into list of active control connections */
    control_socket_list.push_back(fdaccept);

    return fdaccept;
}

void remove_control_conn(int sock_index) {
    for (int i = 0; i < control_socket_list.size(); i++) {
        if (control_socket_list[i] == sock_index) {
            control_socket_list.erase(control_socket_list.begin() + i);
            break;
        }
    }

    close(sock_index);
}

bool isControl(int sock_index) {
    vector<int>::iterator it = find(control_socket_list.begin(), control_socket_list.end(), sock_index);
    return it != control_socket_list.end();
}

bool control_recv_hook(int sock_index) {
    char *cntrl_header, *cntrl_payload;
    uint8_t control_code;
    uint16_t payload_len;

    /* Get control header */
    cntrl_header = (char *) malloc(sizeof(char) * CONTROL_HEADER_SIZE);
    bzero(cntrl_header, CONTROL_HEADER_SIZE);

    if (recvALL(sock_index, cntrl_header, CONTROL_HEADER_SIZE) < 0) {
        remove_control_conn(sock_index);
        free(cntrl_header);
        return false;
    }

    /* Get control code and payload length from the header */

    memcpy(&control_code, cntrl_header + CONTROL_CODE_OFFSET, sizeof(control_code));
    memcpy(&payload_len, cntrl_header + PAYLOAD_LEN_OFFSET, sizeof(payload_len));
    payload_len = ntohs(payload_len);


    free(cntrl_header);

    /* Get control payload */
    if (payload_len != 0) { // in this case, must greater than 0
        cntrl_payload = (char *) malloc(sizeof(char) * payload_len);
        bzero(cntrl_payload, payload_len);

        if (recvALL(sock_index, cntrl_payload, payload_len) < 0) {
            remove_control_conn(sock_index);
            free(cntrl_payload);
            return false;
        }
    }

    /* Triage on control_code */
    switch (control_code) {
        case AUTHOR:
            author(sock_index);
            break;
        case INIT:
            init(sock_index, cntrl_payload);
            break;
        case ROUTING_TABLE:
            routing_table();
            break;
        case UPDATE:
            update();
            break;
        case CRASH:
            crash();
            break;
        case SENDFILE:
            send_file();
            break;
        case SENDFILE_STATS:
            send_file_stats();
            break;
        case LAST_DATA_PACKET:
            last_data_packet();
            break;
        case PENULTIMATE_DATA_PACKET:
            penultimate_data_packet();
            break;

        default:
            break;
    }

    if (payload_len != 0)
        free(cntrl_payload);

    return true;
}

void author(int sock_index) {
    uint16_t payload_len, response_len;
    char *ctrl_response_header, *ctrl_response_payload, *ctrl_response;

    payload_len = sizeof(AUTHOR_STATEMENT) - 1; // Discount the NULL character
    ctrl_response_payload = (char *) malloc(payload_len);
    memcpy(ctrl_response_payload, AUTHOR_STATEMENT, payload_len);

    ctrl_response_header = create_response_header(sock_index, 0, 0, payload_len);

    response_len = CONTROL_HEADER_SIZE + payload_len;
    ctrl_response = (char *) malloc(response_len);
    /* Copy Header */
    memcpy(ctrl_response, ctrl_response_header, CONTROL_HEADER_SIZE);
    /* Copy Payload */
    memcpy(ctrl_response + CONTROL_HEADER_SIZE, ctrl_response_payload, payload_len);

    sendALL(sock_index, ctrl_response, response_len);
}

void init(int sock_index, char *payload) {
    char *ctrl_response;
    char *init_payload;
    struct Routing routing_content;
    uint16_t routers, time_interval;
    uint16_t router_id, router_port, data_port, cost;
    uint32_t router_ip;

    init_payload = (char *) malloc(sizeof(char) * INIT_PAYLOAD_SIZE);
    bzero(init_payload, INIT_PAYLOAD_SIZE);

    int head = 0;
    memcpy(&routers, payload + head, sizeof(routers));
    head += 2;
    memcpy(&time_interval, payload + head, sizeof(time_interval));
    head += 2;

    while (payload + head != NULL) {
        memcpy(init_payload, payload + head, INIT_PAYLOAD_SIZE);

        memcpy(&router_id, payload + head, sizeof(router_id));
        head += 2;
        memcpy(&router_port, payload + head, sizeof(router_port));
        head += 2;
        memcpy(&data_port, payload + head, sizeof(data_port));
        head += 2;
        memcpy(&cost, payload + head, sizeof(cost));
        head += 2;
        memcpy(&router_ip, payload + head, sizeof(router_ip));
        head += 4;
        if (cost == 0) {
            my_id = router_id;
            my_router_port = router_port;
            my_data_port = data_port;
            my_ip = router_ip;
            router_socket = create_route_sock(router_port);
            data_socket = create_data_sock(data_port);
            routing_content.next_hop_id = my_id;
        } else {
            routing_content.next_hop_id = -1;
        }
        routing_content.dest_id = router_id;
        routing_content.dest_route_port = router_port;
        routing_content.dest_data_port = data_port;
        routing_content.dest_cost = cost;
        routing_content.dest_ip = router_ip;
        table.push_back(routing_content);
        bzero(init_payload, INIT_PAYLOAD_SIZE);
    }

    ctrl_response = create_response_header(sock_index, 1, 0, CONTROL_HEADER_SIZE);
    sendALL(sock_index, ctrl_response, CONTROL_HEADER_SIZE);
}

void routing_table() {

}

void update() {

}

void crash() {

}

void send_file() {

}

void send_file_stats() {

}

void last_data_packet() {

}

void penultimate_data_packet() {

}
