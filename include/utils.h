//
// Created by bilin on 11/29/18.
//

#ifndef BILINSHI_UTILS_H
#define BILINSHI_UTILS_H
#include <stdlib.h>
#include <sys/socket.h>

ssize_t recvALL(int sock_index, char *buffer, ssize_t nbytes);
ssize_t sendALL(int sock_index, char *buffer, ssize_t nbytes);

#endif //BILINSHI_UTILS_H
