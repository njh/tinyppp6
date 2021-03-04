#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "tinyppp6.h"


void ipv6_handle_packet(FILE *stream, uint8_t *buffer, int buffer_len)
{
    int vers = (BUF_GET_UINT8(buffer, 0) & 0xF0) >> 4;
    int len = BUF_GET_UINT16(buffer, 4);
    int proto = BUF_GET_UINT8(buffer, 6);

    if (vers != 6) {
        fprintf(stderr, "tinyppp6 recv: IPv6 protocol version is not 6\n");
        return;
    }

    // FIXME: check that the packet isn't longer than the buffer?

    switch (proto) {
        case PROTO_ICMPV6:
            ipv6_handle_icmpv6(stream, buffer, len);
            break;

        case PROTO_UDP:
            ipv6_handle_udp(stream, buffer, len);
            break;

        case PROTO_TCP:
            ipv6_handle_tcp(stream, buffer, len);
            break;

        default:
            fprintf(stderr, "tinyppp6 recv: IPv6 unknown protocol (proto=%d, len=%d)\n", proto, len);
            break;
    }
}

void ipv6_handle_icmpv6(FILE *stream, uint8_t *buffer, int buffer_len)
{
    uint8_t type = BUF_GET_UINT8(buffer, 40);
    uint8_t code = BUF_GET_UINT8(buffer, 41);

    switch (type) {
        case ICMPV6_TYPE_NS:
            fprintf(stderr, "tinyppp6 recv: ICMPv6 Neighbour Solicitation\n");
            // FIXME: handle this
            return;

        case ICMPV6_TYPE_ECHO:
            fprintf(stderr, "tinyppp6 recv: ICMPv6 Echo Request\n");
            // FIXME: handle this
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

void ipv6_handle_udp(FILE *stream, uint8_t *buffer, int buffer_len)
{
    uint16_t src_port = BUF_GET_UINT16(buffer, 40);
    uint16_t dest_port = BUF_GET_UINT16(buffer, 42);
    uint16_t len = BUF_GET_UINT16(buffer, 44) - 8;

    fprintf(stderr, "tinyppp6 recv: UDP (src=%d, dest=%d, len=%d)\n", src_port, dest_port, len);
}

void ipv6_handle_tcp(FILE *stream, uint8_t *buffer, int buffer_len)
{
    fprintf(stderr, "tinyppp6 recv: TCP (len=%d)\n", buffer_len);
    // FIXME: we don't support TCP, send ICMPv6 back
}
