//
// Created by bilin on 11/29/18.
//

#ifndef BILINSHI_CONNECTION_MANAGER_H
#define BILINSHI_CONNECTION_MANAGER_H

#include <stdint.h>
int control_socket, router_socket, data_socket;

void run();

struct __attribute__((__packed__)) Route_Content {
    uint32_t router_ip;
    uint16_t router_port;
    uint16_t padding;
    uint16_t router_id;
    uint16_t link_cost;

};

#endif //BILINSHI_CONNECTION_MANAGER_H
