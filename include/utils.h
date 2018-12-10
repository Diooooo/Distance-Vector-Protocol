//
// Created by bilin on 11/29/18.
//

#ifndef BILINSHI_UTILS_H
#define BILINSHI_UTILS_H
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/time.h>

ssize_t recvALL(int sock_index, char *buffer, ssize_t nbytes);
ssize_t sendALL(int sock_index, char *buffer, ssize_t nbytes);
struct timeval diff_tv(struct timeval tv1, struct timeval tv2);
struct timeval add_tv(struct timeval tv1, struct timeval tv2);

#endif //BILINSHI_UTILS_H
