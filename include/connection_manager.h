//
// Created by bilin on 11/29/18.
//

#ifndef BILINSHI_CONNECTION_MANAGER_H
#define BILINSHI_CONNECTION_MANAGER_H

#include <stdint.h>

extern int control_socket, router_socket, data_socket;
extern uint32_t my_ip;
extern uint16_t my_router_port;
extern uint16_t my_data_port;
extern uint16_t my_id;
extern uint16_t routers_number, time_peroid;
void run(uint16_t control_port);

struct __attribute__((__packed__)) Route_Content {
    uint32_t router_ip;
    uint16_t router_port;
    uint16_t padding;
    uint16_t router_id;
    uint16_t link_cost;

};

#endif //BILINSHI_CONNECTION_MANAGER_H
