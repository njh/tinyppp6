#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

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

    fprintf(stderr, "tinyppp6 send: received LCP Echo-Request, sending LCP Echo-Reply\n");

    BUF_SET_UINT8(buffer, 0, 0xFF);
    BUF_SET_UINT8(buffer, 1, 0x03);
 
    BUF_SET_UINT16(buffer, 2, PPP_PROTO_LCP); // LCP Protocol
    BUF_SET_UINT8(buffer, 4, LCP_ECHO_REPLY);
    BUF_SET_UINT8(buffer, 5, 0x00); // Id
    BUF_SET_UINT16(buffer, 6, 8);   // Length
    BUF_SET_UINT32(buffer, 10, our_magic);

    hdlc_write_frame(stream, buffer, 12);
}

void lcp_handle_frame(FILE *stream, uint8_t *buffer, int len)
{
    int code = BUF_GET_UINT8(buffer, 4);

    switch (code) {
        case LCP_CONF_REQ:
            lcp_reply_conf_req(stream, buffer, len);
            break;

        case LCP_CONF_ACK:
            ipv6cp_send_conf_req(stream);
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
            lcp_echo_reply(stream);
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
    BUF_SET_UINT16(buffer, 2, PPP_PROTO_LCP); // LCP Protocol

    BUF_SET_UINT8(buffer, 4, LCP_CONF_REQ);
    BUF_SET_UINT8(buffer, 5, 0x01);  // Id
    BUF_SET_UINT16(buffer, 6, 10);   // Length

    BUF_SET_UINT8(buffer, 8, 0x05); // Option: Magic Number
    BUF_SET_UINT8(buffer, 9, 6);    // Length
    BUF_SET_UINT32(buffer, 10, our_magic);

    hdlc_write_frame(stream, buffer, 14);
}


void lcp_reject_protocol(FILE *stream, uint8_t *buffer, int len)
{
    uint8_t replybuf[2000];
    static uint8_t id = 1;

    uint16_t protocol = BUF_GET_UINT16(buffer, 2);
    fprintf(stderr, "tinyppp6 send: Sending LCP Protocol-Reject for 0x%4.4x\n", protocol);

    BUF_SET_UINT8(replybuf, 0, 0xFF);
    BUF_SET_UINT8(replybuf, 1, 0x03);
    BUF_SET_UINT16(replybuf, 2, PPP_PROTO_LCP); // LCP Protocol

    BUF_SET_UINT8(replybuf, 4, LCP_PROTO_REJ);
    BUF_SET_UINT8(replybuf, 5, id++);  // Id
    BUF_SET_UINT16(replybuf, 6, len+2);   // Length
    BUF_SET_UINT16(replybuf, 8, protocol);   // Length

    // FIXME: ensure length doesn't exceed the MRU
    memcpy(&replybuf[10], &buffer[4], len-4);

    hdlc_write_frame(stream, replybuf, len+6);
}
