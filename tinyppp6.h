#include <stdio.h>


#ifndef TINYPPP6_H
#define TINYPPP6_H

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
