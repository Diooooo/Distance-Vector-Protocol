//
// Created by bilin on 11/29/18.
//

#include "../include/data_handler.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <strings.h>
#include <vector>
#include <unistd.h>
#include <algorithm>
#include "../include/global.h"
#include "../include/header.h"
#include "../include/utils.h"
#include "../include/connection_manager.h"

using namespace std;

vector<int> data_socket_list;
int data_socket;

bool isData(int sock_index) {
    vector<int>::iterator it = find(data_socket_list.begin(), data_socket_list.end(), sock_index);
    return it != data_socket_list.end();
}

int create_data_sock(uint16_t data_port) {
    int sock;
    struct sockaddr_in data_addr;
    socklen_t addrlen = sizeof(data_addr);

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) ERROR("socket() failed");

    /* Make socket re-usable */
    int opt = 1;
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (const void *) &opt, sizeof(opt)) < 0) ERROR("setsockopt() failed");

    bzero(&data_addr, sizeof(data_addr));

    data_addr.sin_family = AF_INET;
    data_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    data_addr.sin_port = htons(data_port);

    if (bind(sock, (struct sockaddr *) &data_addr, sizeof(data_addr)) < 0) ERROR("bind() failed");

    if (listen(sock, 5) < 0) ERROR("listen() failed");

    return sock;
}

int new_data_conn(int sock_index) {
    int fdaccept, caddr_len;
    struct sockaddr_in remote_data_addr;

    caddr_len = sizeof(remote_data_addr);
    fdaccept = accept(sock_index, (struct sockaddr *) &remote_data_addr, (socklen_t *) &caddr_len);
    if (fdaccept < 0) ERROR("accept() failed");

    /* Insert into list of active control connections */
    data_socket_list.push_back(fdaccept);

    return fdaccept;
}

void remove_data_conn(int sock_index) {
    for (int i = 0; i < data_socket_list.size(); i++) {
        if (data_socket_list[i] == sock_index) {
            data_socket_list.erase(data_socket_list.begin() + i);
            break;
        }
    }

    close(sock_index);
}