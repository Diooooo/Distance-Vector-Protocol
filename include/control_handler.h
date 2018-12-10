//
// Created by bilin on 11/29/18.
//

#ifndef BILINSHI_CONTROL_HANDLER_H
#define BILINSHI_CONTROL_HANDLER_H

#define AUTHOR_STATEMENT "I, bilinshi, have read and understood the course academic integrity policy."

#include <stdint.h>
#include <vector>
#include <sys/time.h>

struct Routing {
    uint16_t dest_id;
    uint16_t dest_route_port;
    uint16_t dest_data_port;
    uint16_t dest_cost;
    uint32_t dest_ip;
    uint16_t next_hop_id;
};

struct Timeout {
    int router_id;
    bool is_connected;
    struct timeval expired_time;
};

struct Route_Neighbor {
    uint16_t router_id;
    int socket;
};

extern std::vector<Routing> table;
extern std::vector<Timeout> routers_timeout;

int create_control_sock(uint16_t control_port);

int create_route_sock(uint16_t router_port);

int new_control_conn(int sock_index);

void remove_control_conn(int sock_index);

int new_route_conn(uint32_t remote_ip, uint16_t remote_port, uint16_t remote_id);

void remove_route_conn(uint16_t router_id);

bool isControl(int sock_index);

bool control_recv_hook(int sock_index);

void author(int sock_index);

void init(int sock_index, char *payload);

void routing_table(int sock_index);

void update(int sock_index, char *payload);

void crash(int sock_index);

void send_file();

void send_file_stats();

void last_data_packet();

void penultimate_data_packet();

void update_routing_table(int sock_index);

void send_dv();

char *create_distance_vector();

#endif //BILINSHI_CONTROL_HANDLER_H
