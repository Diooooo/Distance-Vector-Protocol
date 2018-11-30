//
// Created by bilin on 11/28/18.
//
#include <string.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include "../include/header.h"
#include "../include/global.h"

char *create_response_header(int sock_index, uint8_t control_code, uint8_t response_code, uint16_t payload_len) {
    char *buffer;

    struct Control_Response_Header *control_response_header;
    struct sockaddr_in addr;
    socklen_t addr_size;

    buffer = (char *) malloc(sizeof(char) * CONTROL_HEADER_SIZE);

    control_response_header = (struct Control_Response_Header *) buffer;
    addr_size = sizeof(struct sockaddr_in);
    getpeername(sock_index, (struct sockaddr *) &addr, &addr_size);

    /* Controller IP Address */
    memcpy(&(control_response_header->controller_ip), &(addr.sin_addr), sizeof(struct in_addr));
    /* Control Code */
    control_response_header->control_code = control_code;
    /* Response Code */
    control_response_header->response_code = response_code;
    /* Payload Length */
    control_response_header->payload_len = htons(payload_len);

    return buffer;
}
