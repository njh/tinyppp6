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

    fprintf(stderr, "tinyppp6 send: Sending LCP Conf-Ack\n");

    // Change LCP code to from ConfReq to ConfAck
    BUF_SET_UINT8(buffer, 4, LCP_CONF_ACK);

    hdlc_write_frame(stream, buffer, len);
}

void lcp_echo_reply(FILE *stream)
{
    uint8_t buffer[32];

    fprintf(stderr, "tinyppp6 send: Sending Echo-Reply\n");

    BUF_SET_UINT8(buffer, 0, 0xFF);
    BUF_SET_UINT8(buffer, 1, 0x03);
 
    BUF_SET_UINT16(buffer, 2, 0xc021); // LCP Protocol
    BUF_SET_UINT8(buffer, 4, LCP_ECHO_REPLY);
    BUF_SET_UINT8(buffer, 5, 0x00); // Id
    BUF_SET_UINT16(buffer, 6, 8);   // Length
    BUF_SET_UINT32(buffer, 10, our_magic);

    hdlc_write_frame(stream, buffer, 12);
}

void lcp_handle_frame(uint8_t *buffer, int len)
{
    int code = BUF_GET_UINT8(buffer, 4);
    int id = BUF_GET_UINT8(buffer, 5);

    fprintf(stderr, "tinyppp6 recv: Link Control Protocol (%d, len=%d, id=%x)\n", code, len, id);

    switch (code) {
        case LCP_CONF_REQ:
            fprintf(stderr, "tinyppp6 recv: LCP Configure-Request\n");
            lcp_reply_conf_req(stdout, buffer, len);
            break;

        case LCP_CONF_ACK:
            fprintf(stderr, "tinyppp6 recv: LCP Configure-Ack\n");
            ipv6cp_send_conf_req(stdout);
            break;

        case LCP_CONF_NAK:
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
            lcp_echo_reply(stdout);
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

    fprintf(stderr, "tinyppp6 send: Sending LCP Conf-Req\n");

    BUF_SET_UINT8(buffer, 0, 0xFF);
    BUF_SET_UINT8(buffer, 1, 0x03);
 
    BUF_SET_UINT16(buffer, 2, 0xc021); // LCP Protocol
    BUF_SET_UINT8(buffer, 4, LCP_CONF_REQ);
    BUF_SET_UINT8(buffer, 5, 0x01);  // Id
    BUF_SET_UINT16(buffer, 6, 10);   // Length

    BUF_SET_UINT8(buffer, 8, 0x05); // Option: Magic Number
    BUF_SET_UINT8(buffer, 9, 6);    // Length
    BUF_SET_UINT32(buffer, 10, our_magic);

    hdlc_write_frame(stream, buffer, 14);
}
