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
