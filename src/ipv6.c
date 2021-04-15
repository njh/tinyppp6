#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "tinyppp6.h"


void ipv6_handle_packet(int fd, uint8_t *buffer, int buffer_len)
{
    int vers = (BUF_GET_UINT8(buffer, 0) & 0xF0) >> 4;
    int len = BUF_GET_UINT16(buffer, 4);
    int proto = BUF_GET_UINT8(buffer, 6);

    if (vers != 6) {
        fprintf(stderr, "tinyppp6 recv: IPv6 protocol version is not 6\n");
        return;
    }

    // FIMXE: check that the packet is longer than IPv6 header
    // FIXME: check that the packet isn't longer than the buffer?

    if (ipv6_calculate_checksum(buffer, buffer_len) != 0) {
        fprintf(stderr, "tinyppp6 recv: IPv6 protocol checksum error\n");
        return;
    }

    switch (proto) {
        case PROTO_ICMPV6:
            ipv6_handle_icmpv6(fd, buffer, buffer_len);
            break;

        case PROTO_UDP:
            ipv6_handle_udp(fd, buffer, buffer_len);
            break;

        case PROTO_TCP:
            ipv6_handle_tcp(fd, buffer, buffer_len);
            break;

        default:
            fprintf(stderr, "tinyppp6 recv: IPv6 unknown protocol (proto=%d, len=%d)\n", proto, len);
            break;
    }
}

void icmpv6_echo_reply(int fd, uint8_t *buffer, int buffer_len)
{
    uint8_t replybuf[PACKET_BUF_SIZE];

    fprintf(stderr, "tinyppp6 send: received ICMPv6 Echo-Request, sending ICMPv6 Echo-Reply\n");

    BUF_SET_UINT8(replybuf, 0, 0x60); // IPv6 header
    BUF_SET_UINT8(replybuf, 1, 0x00);
    BUF_SET_UINT8(replybuf, 2, 0x00);
    BUF_SET_UINT8(replybuf, 3, 0x00);

    // FIXME: use constant for IPv6 header length
    BUF_SET_UINT16(replybuf, 4, buffer_len - 40); // Length
    BUF_SET_UINT8(replybuf, 6, PROTO_ICMPV6);     // Protocol
    BUF_SET_UINT8(replybuf, 7, 64);               // Hop Limit

    memcpy(&replybuf[8], &buffer[24], 16);  // Source Address
    memcpy(&replybuf[24], &buffer[8], 16);  // Destination Address

    BUF_SET_UINT8(replybuf, 40, ICMPV6_TYPE_ECHO_REPLY);  // ICMPv6 Type
    BUF_SET_UINT8(replybuf, 41, 0);   // ICMPv6 Code
    BUF_SET_UINT16(replybuf, 42, 0);  // ICMPv6 Checksum

    // Copy over the rest of the packet
    memcpy(&replybuf[44], &buffer[44], buffer_len - 44);

    // Calculate ICMPv6 Checksum
    BUF_SET_UINT16(replybuf, 42, ipv6_calculate_checksum(replybuf, buffer_len));

    hdlc_write_frame(fd, PPP_PROTO_IPV6, replybuf, buffer_len);
}

void ipv6_handle_icmpv6(int fd, uint8_t *buffer, int buffer_len)
{
    uint8_t type = BUF_GET_UINT8(buffer, 40);
    uint8_t code = BUF_GET_UINT8(buffer, 41);

    switch (type) {
        case ICMPV6_TYPE_NS:
            fprintf(stderr, "tinyppp6 recv: ICMPv6 Neighbour Solicitation\n");
            // FIXME: handle this
            return;

        case ICMPV6_TYPE_ECHO:
            icmpv6_echo_reply(fd, buffer, buffer_len);
            return;

        case ICMPV6_TYPE_RA:
            // FIXME: handle this
            fprintf(stderr, "tinyppp6 recv: ICMPv6 Router Advertisment\n");
            return;

        default:
            // We didn't handle the packet
            fprintf(stderr, "tinyppp6 recv: ICMPv6 (type=%d, code=%d, len=%d)\n", type, code, buffer_len);
            return;
    }
}

void ipv6_handle_udp(int fd, uint8_t *buffer, int buffer_len)
{
    uint16_t src_port = BUF_GET_UINT16(buffer, 40);
    uint16_t dest_port = BUF_GET_UINT16(buffer, 42);
    uint16_t len = BUF_GET_UINT16(buffer, 44) - 8;
    char *payload = (char *)&buffer[48];

    fprintf(stderr, "tinyppp6 recv: UDP (src=%d, dest=%d, len=%d)\n", src_port, dest_port, len);

    if (dest_port == 1234) {
        fwrite(payload, 1, len, stderr);
        fprintf(stderr, "\n");
    } else {
        // FIXME: send back an ICMPv6 message
    }
}

void ipv6_handle_tcp(int fd, uint8_t *buffer, int buffer_len)
{
    fprintf(stderr, "tinyppp6 recv: TCP (len=%d)\n", buffer_len);
    // FIXME: we don't support TCP, send ICMPv6 back
}


// This function comes from Contiki's uip6.c
uint16_t chksum(uint16_t sum, const uint8_t *data, uint16_t len)
{
    uint16_t t;
    const uint8_t *dataptr;
    const uint8_t *last_byte;

    dataptr = data;
    last_byte = data + len - 1;

    while (dataptr < last_byte) {  /* At least two more bytes */
        t = (dataptr[0] << 8) + dataptr[1];
        sum += t;
        if (sum < t) {
            sum++;      /* carry */
        }
        dataptr += 2;
    }

    if (dataptr == last_byte) {
        t = (dataptr[0] << 8) + 0;
        sum += t;
        if (sum < t) {
            sum++;      /* carry */
        }
    }

    /* Return sum in host byte order. */
    return sum;
}

// This function is derived from Contiki's uip6.c / upper_layer_chksum()
uint16_t ipv6_calculate_checksum(uint8_t *buffer, int buffer_len)
{
    uint16_t len = BUF_GET_UINT16(buffer, 4);
    uint8_t proto = BUF_GET_UINT8(buffer, 6);
    uint8_t *src_addr = &buffer[8];
    uint8_t *dest_addr = &buffer[24];
    uint8_t *payload = &buffer[40];

    /* First sum pseudoheader. */
    /* IP protocol and length fields. This addition cannot carry. */
    volatile uint16_t newsum = len + proto;

    /* Sum IP source and destination addresses. */
    newsum = chksum(newsum, src_addr, 16);
    newsum = chksum(newsum, dest_addr, 16);

    /* Sum the payload header and data */
    newsum = chksum(newsum, payload, len);

    return ~newsum;
}
