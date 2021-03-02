#include <stdio.h>


#ifndef TINYPPP6_H
#define TINYPPP6_H

// Macros to get integers in and out of buffers in network byte order
#define BUF_GET_UINT8(buf, idx)        buf[idx]
#define BUF_SET_UINT8(buf, idx, val)   buf[idx] = (uint8_t)val;
#define BUF_GET_UINT16(buf, idx)       (buf[idx] << 8) | buf[idx+1]
#define BUF_SET_UINT16(buf, idx, val)  buf[idx] =   (val & 0xFF00u) >> 8; \
                                       buf[idx+1] = (val & 0x00FFu);
#define BUF_GET_UINT32(buf, idx)       (buf[idx] << 24) | (buf[idx+1] << 16) | \
                                       (buf[idx+2] << 8) | buf[idx+3]
#define BUF_SET_UINT32(buf, idx, val)  buf[idx] =   (val & 0xFF000000u) >> 24; \
                                       buf[idx+1] = (val & 0x00FF0000u) >> 16; \
                                       buf[idx+2] = (val & 0x0000FF00u) >> 8; \
                                       buf[idx+3] = (val & 0x000000FFu);


// fcs.c
uint16_t calculate_fcs16(const uint8_t *cp, int len);

// frame.c
int hdlc_read_frame(FILE *stream, uint8_t *buffer);
int hdlc_check_frame(const uint8_t *buffer, int len);
int hdlc_write_frame(FILE *stream, const uint8_t *buffer, int len);

// ipv6cp.c
void ipv6cp_init();
void ipv6cp_handle_frame(uint8_t *buffer, int len);
void ipv6cp_send_conf_req(FILE *stream);

enum {
    IPV6CP_CONF_REQ = 1,
    IPV6CP_CONF_ACK = 2,
    IPV6CP_CONF_NAK = 3,
    IPV6CP_CONF_REJ = 4,
    IPV6CP_TERM_REQ = 5,
    IPV6CP_TERM_ACK = 6,
    IPV6CP_CODE_REJ = 7,
};


// lcp.c
void lcp_init();
void lcp_handle_frame(uint8_t *buffer, int len);
void lcp_echo_reply(FILE *stream);
void lcp_send_conf_req(FILE *stream);
void lcp_reject_protocol(FILE* stream, uint8_t *buffer, int len);

// LCP Codes
enum {
    LCP_CONF_REQ = 1,
    LCP_CONF_ACK = 2,
    LCP_CONF_NAK = 3,
    LCP_CONF_REJ = 4,
    LCP_TERM_REQ = 5,
    LCP_TERM_ACK = 6,
    LCP_CODE_REJ = 7,
    LCP_PROTO_REJ = 8,
    LCP_ECHO_REQ = 9,
    LCP_ECHO_REPLY = 10,
    LCP_DISCARD_REQ = 11,
};

#endif
