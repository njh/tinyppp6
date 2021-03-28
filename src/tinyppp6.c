#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>

#include "tinyppp6.h"


void handle_frame(FILE *stream, uint8_t *buffer, int len)
{
    uint16_t protocol = (buffer[2] << 8) | buffer[3];
    uint8_t *payload = &buffer[4];
    int payload_len = len - 6;

    switch (protocol) {
        case PPP_PROTO_PAD:
            fprintf(stderr, "tinyppp6 recv: padding (len=%d)\n", payload_len);
            break;
        case PPP_PROTO_LCP:
            lcp_handle_frame(stream, payload, payload_len);
            break;
        case PPP_PROTO_IPV6CP:
            ipv6cp_handle_frame(stream, payload, payload_len);
            break;
        case PPP_PROTO_IPV6:
            ipv6_handle_packet(stream, payload, payload_len);
            break;
        default:
            fprintf(stderr, "tinyppp6 recv: Unknown 0x%4.4x (len=%d)\n", protocol, payload_len);
            lcp_reject_protocol(stream, protocol, payload, payload_len);
            break;
    }

    fprintf(stderr, "\n");
}

int main(int argc, char **argv)
{
    FILE *input = stdin;
    FILE *output = stdout;

    // FIXME: Seed the random number generator
    // FIXME: how to seed? srandom();

    lcp_init();
    lcp_send_conf_req(output);

    while (!feof(input)) {
        uint8_t buffer[PACKET_BUF_SIZE];
        int len = hdlc_read_frame(input, buffer);
        if (len > 0 && hdlc_check_frame(buffer, len)) {
            handle_frame(output, buffer, len);
        }
    }

    fclose(input);
    fclose(output);
}
