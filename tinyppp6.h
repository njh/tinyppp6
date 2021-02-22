#include <stdio.h>

int read_frame(FILE* stream, uint8_t *buffer);
int write_frame(uint8_t *buffer, int buffer_len);

void handle_lcp(uint8_t *buffer, int len);

uint16_t calculate_fcs16(uint8_t *cp, int len);