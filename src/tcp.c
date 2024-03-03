/**
 * @file tcp.c
 * @author Le Duy Nguyen, xnguye27, VUT FIT
 * @date 03/03/2024
 * @brief Implementation for the tcp.h module
 */

#include "tcp.h"
#include "error.h"
#include "bytes.h"
#include <sys/socket.h>
#include <netdb.h>


/**
 * @brief Size of the buffer used for data transmission.
 *
 * This macro defines the size of the buffer used for data transmission.
 * A value of 4 KiB (4096 bytes) is considered reasonable for modern systems
 * and is sufficient to hold the longest packet in the IPK2024 protocol
 * due to limitations in its specification.
 */
#define BUFFER_SIZE (4 * 1024)

void tcp_connect(Connection *conn) {
    struct sockaddr *address = conn->address_info->ai_addr;
    socklen_t address_len = conn->address_info->ai_addrlen;

    if (connect(conn->sockfd, address, address_len) != 0) {
        set_error(Error_Connection);
        fprintf(stderr, "tcp: Cannot connect to the server %s \n", conn->address_info->ai_canonname);
    }
}

void tcp_send(Connection *conn, Payload payload) {
    Bytes bytes = payload_serialize(&payload, Mode_TCP);

    if (send(conn->sockfd, bytes.data, bytes.len, MSG_DONTWAIT) < 0) {
        set_error(Error_Connection);
        fprintf(stderr, "ERROR tcp: Cannot send packet to the server\n");
    }
}

Payload tcp_receive(Connection *conn) {
    uint8_t buffer[BUFFER_SIZE];
    PayloadDeserialization payload_deser = {0};

    ssize_t len = recv(conn->sockfd, buffer, BUFFER_SIZE, MSG_DONTWAIT);

    if (len < 0) {
        set_error(Error_Connection);
        fprintf(stderr, "ERROR tcp: Cannot receive packet from the server\n");
    } else {
        payload_deserialize(buffer, len, Mode_TCP, &payload_deser);
    }

    return payload_deser.payload;
}

void tcp_disconnect(Connection *conn) {
    Payload payload;
    payload.type = PayloadType_Bye;
    tcp_send(conn, payload);
    shutdown(conn->sockfd, SHUT_RDWR);
}
