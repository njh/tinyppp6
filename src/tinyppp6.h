#include <stdio.h>


#ifndef TINYPPP6_H
#define TINYPPP6_H

// Macros to get integers in and out of buffers in network byte order
#define BUF_GET_UINT8(buf, idx)        buf[idx]
#define BUF_SET_UINT8(buf, idx, val)   buf[idx] = (uint8_t)(val);
#define BUF_GET_UINT16(buf, idx)       (buf[idx] << 8) | buf[idx+1]
#define BUF_SET_UINT16(buf, idx, val)  buf[idx] =   ((val) & 0xFF00u) >> 8; \
                                       buf[idx+1] = ((val) & 0x00FFu);
#define BUF_GET_UINT32(buf, idx)       (buf[idx] << 24) | (buf[idx+1] << 16) | \
                                       (buf[idx+2] << 8) | buf[idx+3]
#define BUF_SET_UINT32(buf, idx, val)  buf[idx] =   ((val) & 0xFF000000u) >> 24; \
                                       buf[idx+1] = ((val) & 0x00FF0000u) >> 16; \
                                       buf[idx+2] = ((val) & 0x0000FF00u) >> 8; \
                                       buf[idx+3] = ((val) & 0x000000FFu);


// PPP Protocol Numbers
// https://www.iana.org/assignments/ppp-numbers/
enum {
    PPP_PROTO_PAD = 0x0001,
    PPP_PROTO_LCP = 0xc021,
    PPP_PROTO_IPV6CP = 0x8057,
    PPP_PROTO_IPV6 = 0x0057
};


// fcs.c
uint16_t fcs16_add_byte(uint16_t current_fcs, uint8_t byte);
uint16_t fcs16_calculate(const uint8_t *cp, int len);

// frame.c
int hdlc_read_frame(FILE *stream, uint8_t *buffer);
int hdlc_check_frame(const uint8_t *buffer, int len);
int hdlc_write_frame(FILE *stream, uint16_t protocol, const uint8_t *buffer, int len);


// ipv6.c
void ipv6_handle_packet(FILE *stream, uint8_t *buffer, int buffer_len);
void ipv6_handle_icmpv6(FILE *stream, uint8_t *buffer, int buffer_len);
void ipv6_handle_udp(FILE *stream, uint8_t *buffer, int buffer_len);
void ipv6_handle_tcp(FILE *stream, uint8_t *buffer, int buffer_len);
uint16_t ipv6_calculate_checksum(uint8_t *buffer, int buffer_len);

enum ip_protocol {
    PROTO_TCP = 6,      ///< IP protocol number for TCP
    PROTO_UDP = 17,     ///< IP protocol number for UDP
    PROTO_ICMPV6 = 58   ///< IP protocol number for ICMPv6
};

enum icmpv6_type {
    ICMPV6_TYPE_UNREACHABLE = 1,
    ICMPV6_TYPE_PARAM_PROB = 4,
    ICMPV6_TYPE_ECHO = 128,
    ICMPV6_TYPE_ECHO_REPLY = 129,
    ICMPV6_TYPE_RS = 133,
    ICMPV6_TYPE_RA = 134,
    ICMPV6_TYPE_NS = 135,
    ICMPV6_TYPE_NA = 136
};


// ipv6cp.c
void ipv6cp_init();
void ipv6cp_handle_frame(FILE *stream, uint8_t *buffer, int len);
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
void lcp_handle_frame(FILE *stream, uint8_t *buffer, int len);
void lcp_echo_reply(FILE *stream);
void lcp_send_conf_req(FILE *stream);
void lcp_reject_protocol(FILE *stream, uint8_t *buffer, int len);

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
