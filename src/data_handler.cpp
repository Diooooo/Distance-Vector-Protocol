//
// Created by bilin on 11/29/18.
//

#include "../include/data_handler.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <strings.h>
#include <string.h>
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

bool data_recv_hook(int sock_index) {
    char *data_header, *data_payload;
    uint32_t dest_ip;
    uint8_t transfer_id;
    uint8_t ttl;
    uint16_t seq_num;
    uint32_t fin;

    data_header = (char *) malloc(sizeof(char) * DATA_HEADER_SIZE);
    bzero(data_header, DATA_HEADER_SIZE);

    if (recvALL(sock_index, data_header, DATA_HEADER_SIZE) < 0) {
        remove_data_conn(sock_index);
        free(data_header);
        return false;
    }
    memcpy(&dest_ip, data_header, sizeof(dest_ip));
    dest_ip = ntohl(dest_ip);
    memcpy(&transfer_id, data_header + 4, sizeof(transfer_id));
    memcpy(&ttl, data_header + 5, sizeof(ttl));
    memcpy(&seq_num, data_header + 6, sizeof(seq_num));
    seq_num = ntohs(seq_num);
    memcpy(&fin, data_header + 8, sizeof(fin));

    free(data_header);

    data_payload = (char *) malloc(sizeof(char) * DATA_PAYLOAD);
    bzero(data_payload, DATA_PAYLOAD);

    if (recvALL(sock_index, data_payload, DATA_PAYLOAD) < 0) {
        remove_data_conn(sock_index);
        free(data_payload);
        return false;
    }

    if (fin == LAST_DATA_PACKET) {
        // store payload in last_packet_list
    } else {
        // replace payload in penultimate_packet_list
    }
    if (dest_ip == my_ip && ttl == 1) {
        free(data_payload);
        return true;
    }
    ttl--;

    char *next_data_header = create_data_header(dest_ip, transfer_id, ttl, seq_num, fin);
    char *next_packet = (char *)malloc(DATA_HEADER_SIZE + DATA_PAYLOAD);
    memcpy(next_packet, next_data_header, DATA_HEADER_SIZE);
    memcpy(next_packet + DATA_HEADER_SIZE, data_payload, DATA_PAYLOAD);

    // need to decide which router to send
    // if not connected yet, create new socket and add to master_list
    sendALL(expected_socket, next_packet, DATA_HEADER_SIZE + DATA_PAYLOAD);


    return true;
}