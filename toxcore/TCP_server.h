/*
* TCP_server.h -- Implementation of the TCP relay server part of Tox.
*
*  Copyright (C) 2014 Tox project All Rights Reserved.
*
*  This file is part of Tox.
*
*  Tox is free software: you can redistribute it and/or modify
*  it under the terms of the GNU General Public License as published by
*  the Free Software Foundation, either version 3 of the License, or
*  (at your option) any later version.
*
*  Tox is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License
*  along with Tox.  If not, see <http://www.gnu.org/licenses/>.
*
*/

#ifndef TCP_SERVER_H
#define TCP_SERVER_H

#include "net_crypto.h"
#include "onion.h"

#if defined(_WIN32) || defined(__WIN32__) || defined(WIN32) || defined(__MACH__)
#define MSG_NOSIGNAL 0
#endif

#define MAX_INCOMMING_CONNECTIONS 32

#define TCP_MAX_BACKLOG MAX_INCOMMING_CONNECTIONS

#define MAX_PACKET_SIZE 2048

#define TCP_HANDSHAKE_PLAIN_SIZE (crypto_box_PUBLICKEYBYTES + crypto_box_NONCEBYTES)
#define TCP_SERVER_HANDSHAKE_SIZE (crypto_box_NONCEBYTES + TCP_HANDSHAKE_PLAIN_SIZE + crypto_box_MACBYTES)
#define TCP_CLIENT_HANDSHAKE_SIZE (crypto_box_PUBLICKEYBYTES + TCP_SERVER_HANDSHAKE_SIZE)

#define NUM_RESERVED_PORTS 16
#define NUM_CLIENT_CONNECTIONS (256 - NUM_RESERVED_PORTS)

#define TCP_PACKET_ROUTING_REQUEST  0
#define TCP_PACKET_ROUTING_RESPONSE 1
#define TCP_PACKET_CONNECTION_NOTIFICATION 2
#define TCP_PACKET_DISCONNECT_NOTIFICATION 3
#define TCP_PACKET_PING 4
#define TCP_PACKET_PONG 5
#define TCP_PACKET_ONION_REQUEST  8
#define TCP_PACKET_ONION_RESPONSE 9

#define ARRAY_ENTRY_SIZE 6

#define TCP_ONION_FAMILY (AF_INET6 + 1)

/* frequency to ping connected nodes and timeout in seconds */
#define TCP_PING_FREQUENCY 30
#define TCP_PING_TIMEOUT 20

enum {
    TCP_STATUS_NO_STATUS,
    TCP_STATUS_CONNECTED,
    TCP_STATUS_UNCONFIRMED,
    TCP_STATUS_CONFIRMED,
};

typedef struct TCP_Secure_Connection {
    uint8_t status;
    sock_t  sock;
    uint8_t public_key[crypto_box_PUBLICKEYBYTES];
    uint8_t recv_nonce[crypto_box_NONCEBYTES]; /* Nonce of received packets. */
    uint8_t sent_nonce[crypto_box_NONCEBYTES]; /* Nonce of sent packets. */
    uint8_t shared_key[crypto_box_BEFORENMBYTES];
    uint16_t next_packet_length;
    struct {
        uint8_t status; /* 0 if not used, 1 if other is offline, 2 if other is online. */
        uint8_t public_key[crypto_box_PUBLICKEYBYTES];
        uint32_t index;
        uint8_t other_id;
    } connections[NUM_CLIENT_CONNECTIONS];
    uint8_t last_packet[2 + MAX_PACKET_SIZE];
    uint16_t last_packet_length;
    uint16_t last_packet_sent;

    uint64_t identifier;

    uint64_t last_pinged;
    uint64_t ping_id;
} TCP_Secure_Connection;


typedef struct {
    Onion *onion;
    sock_t *socks_listening;
    unsigned int num_listening_socks;

    uint8_t public_key[crypto_box_PUBLICKEYBYTES];
    uint8_t secret_key[crypto_box_SECRETKEYBYTES];
    TCP_Secure_Connection incomming_connection_queue[MAX_INCOMMING_CONNECTIONS];
    uint16_t incomming_connection_queue_index;
    TCP_Secure_Connection unconfirmed_connection_queue[MAX_INCOMMING_CONNECTIONS];
    uint16_t unconfirmed_connection_queue_index;

    TCP_Secure_Connection *accepted_connection_array;
    uint32_t size_accepted_connections;
    uint32_t num_accepted_connections;

    uint64_t counter;
} TCP_Server;

/* Create new TCP server instance.
 */
TCP_Server *new_TCP_server(uint8_t ipv6_enabled, uint16_t num_sockets, uint16_t *ports, uint8_t *public_key,
                           uint8_t *secret_key, Onion *onion);

/* Run the TCP_server
 */
void do_TCP_server(TCP_Server *TCP_server);

/* Kill the TCP server
 */
void kill_TCP_server(TCP_Server *TCP_server);

/* Read the next two bytes in TCP stream then convert them to
 * length (host byte order).
 *
 * return length on success
 * return 0 if nothing has been read from socket.
 * return ~0 on failure.
 */
uint16_t read_TCP_length(sock_t sock);

/* Read length bytes from socket.
 *
 * return length on success
 * return -1 on failure/no data in buffer.
 */
int read_TCP_packet(sock_t sock, uint8_t *data, uint16_t length);

/* return length of recieved packet on success.
 * return 0 if could not read any packet.
 * return -1 on failure (connection must be killed).
 */
int read_packet_TCP_secure_connection(sock_t sock, uint16_t *next_packet_length, uint8_t *shared_key,
                                      uint8_t *recv_nonce, uint8_t *data, uint16_t max_len);


#endif
