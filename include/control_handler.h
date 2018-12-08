//
// Created by bilin on 11/29/18.
//

#ifndef BILINSHI_CONTROL_HANDLER_H
#define BILINSHI_CONTROL_HANDLER_H

#define AUTHOR_STATEMENT "I, bilinshi, have read and understood the course academic integrity policy."

#include <stdint.h>
#include <vector>


struct Routing{
    int dest_id;
    int dest_route_port;
    int dest_data_port;
    int dest_cost;
    uint32_t dest_ip;
    int next_hop_id;
};

int create_control_sock(uint16_t control_port);

int create_route_sock(uint16_t router_port);

int new_control_conn(int sock_index);

bool isControl(int sock_index);

bool control_recv_hook(int sock_index);

void author(int sock_index);

void init(int sock_index, char *payload);

void routing_table();

void update();

void crash();

void send_file();

void send_file_stats();

void last_data_packet();

void penultimate_data_packet();

#endif //BILINSHI_CONTROL_HANDLER_H
