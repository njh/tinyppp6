#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "tinyppp6.h"

// https://tools.ietf.org/html/rfc5072

void ipv6cp_reply_conf_req(FILE *stream, uint8_t *buffer, int len)
{
    // FIXME: check if there are any options we don't like

    fprintf(stderr, "tinyppp6 send: Sending IPV6CP Conf-Ack\n");

    // Change IPV6CP code to from ConfReq to ConfAck
    buffer[4] = IPV6CP_CONF_ACK;

    hdlc_write_frame(stream, buffer, len);
}

void ipv6cp_handle_frame(uint8_t *buffer, int len)
{
    int code = buffer[4];
    int id = buffer[5];

    fprintf(stderr, "tinyppp6 recv: IPv6CP (%d, len=%d, id=%x)\n", code, len, id);

    switch (code) {
        case IPV6CP_CONF_REQ:
            fprintf(stderr, "tinyppp6 recv: IPV6CP Configure-Request\n");
            ipv6cp_reply_conf_req(stdout, buffer, len);
            break;

        case IPV6CP_CONF_ACK:
            fprintf(stderr, "tinyppp6 recv: IPV6CP Configure-Ack\n");
            break;

        case IPV6CP_CONF_NAK:
            fprintf(stderr, "tinyppp6 recv: IPV6CP Configure-Nak\n");
            break;

        case IPV6CP_CONF_REJ:
            fprintf(stderr, "tinyppp6 recv: IPV6CP Configure-Reject\n");
            break;

        case IPV6CP_TERM_REQ:
            fprintf(stderr, "tinyppp6 recv: IPV6CP Terminate-Request\n");
            break;

        case IPV6CP_TERM_ACK:
            fprintf(stderr, "tinyppp6 recv: IPV6CP Terminate-Ack\n");
            break;

        case IPV6CP_CODE_REJ:
            fprintf(stderr, "tinyppp6 recv: IPV6CP Code-Reject\n");
            break;

        default:
            fprintf(stderr, "tinyppp6 recv: Unknown IPV6CP code: %d\n", code);
            break;
    }
}

void ipv6cp_send_conf_req(FILE *stream)
{
    uint8_t buffer[32];

    fprintf(stderr, "tinyppp6 send: Sending IPV6CP Conf-Req\n");

    buffer[0] = 0xff;
    buffer[1] = 0x03;

    buffer[2] = 0x80;  // IPV6CP Protocol
    buffer[3] = 0x57;
    buffer[4] = IPV6CP_CONF_REQ;
    buffer[5] = 0x01;  // IPV6CP Id
    buffer[6] = 0;     // IPV6CP length
    buffer[7] = 14;    // IPV6CP length

    buffer[8] = 0x01;  // Interface Identifier
    buffer[9] = 10;    // Length
    buffer[10] = 0x01;
    buffer[11] = 0x02;
    buffer[12] = 0x03;
    buffer[13] = 0x04;
    buffer[14] = 0x05;
    buffer[15] = 0x06;
    buffer[16] = 0x07;
    buffer[17] = 0x08;

    hdlc_write_frame(stream, buffer, 18);
}
