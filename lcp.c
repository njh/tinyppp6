#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "tinyppp6.h"


uint32_t our_magic = 0;
uint32_t their_magic = 0;


void lcp_init()
{
    our_magic = random();

}

void lcp_reply_conf_req(FILE *stream, uint8_t *buffer, int len)
{
    // FIXME: check if there are any options we don't like

    fprintf(stderr, "tinyppp6 send: Sending Conf-Ack\n");

    // Change LCP code to from ConfReq to ConfAck
    buffer[4] = LCP_CONF_ACK;

    write_frame(stream, buffer, len);
}

void lcp_handle_frame(uint8_t *buffer, int len)
{
    int code = buffer[4];
    int id = buffer[5];

    fprintf(stderr, "tinyppp6 recv: Link Control Protocol (%d, len=%d, id=%x)\n", code, len, id);

    switch (code) {
        case LCP_CONF_REQ:
            fprintf(stderr, "tinyppp6 recv: LCP Configure-Request\n");
            lcp_reply_conf_req(stdout, buffer, len);
            break;

        case LCP_CONF_ACK:
            fprintf(stderr, "tinyppp6 recv: LCP Configure-Ack\n");
            break;

        case LCP_CONF_NACK:
            fprintf(stderr, "tinyppp6 recv: LCP Configure-Nak\n");
            break;

        case LCP_CONF_REJ:
            fprintf(stderr, "tinyppp6 recv: LCP Configure-Reject\n");
            break;

        case LCP_TERM_REQ:
            fprintf(stderr, "tinyppp6 recv: LCP Terminate-Request\n");
            break;

        case LCP_TERM_ACK:
            fprintf(stderr, "tinyppp6 recv: LCP Terminate-Ack\n");
            break;

        case LCP_CODE_REJ:
            fprintf(stderr, "tinyppp6 recv: LCP Code-Reject\n");
            break;

        case LCP_PROTO_REJ:
            fprintf(stderr, "tinyppp6 recv: LCP Protocol-Reject\n");
            break;

        case LCP_ECHO_REQ:
            fprintf(stderr, "tinyppp6 recv: LCP Echo-Request\n");
            break;

        case LCP_ECHO_REPLY:
            fprintf(stderr, "tinyppp6 recv: LCP Echo-Reply\n");
            break;

        case LCP_DISCARD_REQ:
            fprintf(stderr, "tinyppp6 recv: LCP Discard-Request\n");
            break;

        default:
            fprintf(stderr, "tinyppp6 recv: Unknown LCP code: %d\n", code);
            break;
    }
}

void lcp_send_conf_req(FILE *stream)
{
    uint8_t buffer[32];

    fprintf(stderr, "tinyppp6 send: Sending Conf-Req\n");

    buffer[0] = 0xff;
    buffer[1] = 0x03;

    buffer[2] = 0xc0;  // LCP Protocol
    buffer[3] = 0x21;
    buffer[4] = LCP_CODE_REJ;
    buffer[5] = 0x01;  // LCP id
    buffer[6] = 0;     // LCP length
    buffer[7] = 16;    // LCP length

    // lcp_add_async_control_map()
    buffer[8] = 0x02;  // Async Control Map
    buffer[9] = 6;  // Length
    buffer[10] = 0;
    buffer[11] = 0;
    buffer[12] = 0;
    buffer[13] = 0;

    // lcp_add_magic_number()
    buffer[14] = 0x05;  // Magic Number
    buffer[15] = 6;  // Length
    buffer[16] = (our_magic & 0xFF000000) >> 24;
    buffer[17] = (our_magic & 0x00FF0000) >> 16;
    buffer[18] = (our_magic & 0x0000FF00) >> 8;
    buffer[19] = (our_magic & 0x000000FF) >> 0;

    write_frame(stream, buffer, 20);
}
