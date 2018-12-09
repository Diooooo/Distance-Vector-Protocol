//
// Created by bilin on 11/29/18.
//

#ifndef BILINSHI_DATA_HANDLER_H
#define BILINSHI_DATA_HANDLER_H

#include <stdint.h>

bool isData(int sock_index);

int create_data_sock(uint16_t data_port);

int new_data_conn(int sock_index);

void remove_data_conn(int sock_index);

bool data_recv_hook(int sock_index);

#endif //BILINSHI_DATA_HANDLER_H
