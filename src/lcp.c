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

void lcp_write_packet(FILE *stream, const uint8_t *buffer)
{
    int len = LCP_PACKET_LEN(buffer);
    // FIXME: check len isn't too long or too short
    hdlc_write_frame(stream, PPP_PROTO_LCP, buffer, len);
}

void lcp_append_buf(uint8_t *packet, uint8_t *buffer, uint16_t bufffer_len)
{
    uint8_t *end = packet + LCP_PACKET_LEN(packet);
    uint16_t new_packet_len = LCP_PACKET_LEN(packet) + bufffer_len;

    memcpy(end, buffer, bufffer_len);
    
    // Update the total packet length
    BUF_SET_UINT16(packet, 2, new_packet_len);
}

void lcp_append_option_uint16(uint8_t *packet, int id, uint16_t value)
{
    uint8_t option[4];
    
    option[0] = id;
    option[1] = sizeof(option);
    BUF_SET_UINT16(option, 2, value);
    
    lcp_append_buf(packet, option, sizeof(option));
}

void lcp_append_option_uint32(uint8_t *packet, int id, uint32_t value)
{
    uint8_t option[6];
    
    option[0] = id;
    option[1] = sizeof(option);
    BUF_SET_UINT32(option, 2, value);
    
    lcp_append_buf(packet, option, sizeof(option));
}

void lcp_handle_conf_req(FILE *stream, uint8_t *buffer)
{
    fprintf(stderr, "tinyppp6 send: Sending LCP Conf-Ack\n");

    // Change LCP code to from ConfReq to ConfAck
    BUF_SET_UINT8(buffer, 0, LCP_CONF_ACK);

    lcp_write_packet(stream, buffer);
}

void lcp_echo_reply(FILE *stream)
{
    uint8_t buffer[PACKET_BUF_SIZE];

    fprintf(stderr, "tinyppp6 send: received LCP Echo-Request, sending LCP Echo-Reply\n");

    BUF_SET_UINT8(buffer, 0, LCP_ECHO_REPLY);
    BUF_SET_UINT8(buffer, 1, 0x00); // Id
    BUF_SET_UINT16(buffer, 2, 8);   // Length
    BUF_SET_UINT32(buffer, 4, our_magic);

    lcp_write_packet(stream, buffer);
}

void lcp_handle_frame(FILE *stream, uint8_t *buffer, int buffer_len)
{
    int code = BUF_GET_UINT8(buffer, 0);

    if (LCP_PACKET_LEN(buffer) != buffer_len) {
        fprintf(stderr, "tinyppp6 warning: LCP length != frame length\n");
    }

    switch (code) {
        case LCP_CONF_REQ:
            lcp_handle_conf_req(stream, buffer);
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
    uint8_t buffer[PACKET_BUF_SIZE];

    fprintf(stderr, "tinyppp6 send: Sending LCP Conf-Req\n");

    BUF_SET_UINT8(buffer, 0, LCP_CONF_REQ);
    BUF_SET_UINT8(buffer, 1, 0x01);  // Id
    BUF_SET_UINT16(buffer, 2, 4);    // Length

    lcp_append_option_uint32(buffer, LCP_OPTION_MAGIC_NUM, our_magic);

    lcp_write_packet(stream, buffer);
}


void lcp_reject_protocol(FILE *stream, uint16_t protocol, uint8_t *buffer, int len)
{
    uint8_t replybuf[PACKET_BUF_SIZE];
    static uint8_t id = 1;

    fprintf(stderr, "tinyppp6 send: Sending LCP Protocol-Reject for 0x%4.4x\n", protocol);

    BUF_SET_UINT8(replybuf, 0, LCP_PROTO_REJ);
    BUF_SET_UINT8(replybuf, 1, id++);  // Id
    BUF_SET_UINT16(replybuf, 2, 6); // Length of header
    BUF_SET_UINT16(replybuf, 4, protocol); // Rejected protocol number

    // FIXME: ensure length doesn't exceed the MRU
    lcp_append_buf(replybuf, buffer, len);

    lcp_write_packet(stream, replybuf);
}
