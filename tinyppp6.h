#include <stdio.h>


// fcs.c
uint16_t calculate_fcs16(const uint8_t *cp, int len);

// frame.c
int read_frame(FILE *stream, uint8_t *buffer);
int write_frame(FILE *stream, uint8_t *buffer, int buffer_len);

// lcp.c
void lcp_init();
void lcp_handle_frame(uint8_t *buffer, int len);
void lcp_send_conf_req(FILE *stream);

// LCP Codes
enum {
    LCP_CONF_REQ = 1,
    LCP_CONF_ACK = 2,
    LCP_CONF_NACK = 3,
    LCP_CONF_REJ = 4,
    LCP_TERM_REQ = 5,
    LCP_TERM_ACK = 6,
    LCP_CODE_REJ = 7,
    LCP_PROTO_REJ = 8,
    LCP_ECHO_REQ = 9,
    LCP_ECHO_REPLY = 10,
    LCP_DISCARD_REQ = 11,
};
