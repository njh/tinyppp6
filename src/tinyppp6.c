#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "tinyppp6.h"


void handle_frame(FILE *stream, uint8_t *buffer, int len)
{
    // FIXME: how do we detect 1-byte protocol number?
    uint16_t protocol = (buffer[2] << 8) | buffer[3];
    switch (protocol) {
        case PPP_PROTO_PAD:
            fprintf(stderr, "tinyppp6 recv: padding (len=%d)\n", len);
            break;
        case PPP_PROTO_LCP:
            lcp_handle_frame(stream, buffer, len - 2);
            break;
        case PPP_PROTO_IPV6CP:
            ipv6cp_handle_frame(stream, buffer, len - 2);
            break;
        case PPP_PROTO_IPV6:
            fprintf(stderr, "tinyppp6 recv: IPv6 Packet (len=%d)\n", len);
            break;
        default:
            fprintf(stderr, "tinyppp6 recv: Unknown 0x%4.4x (len=%d)\n", protocol, len);
            lcp_reject_protocol(stream, buffer, len);
            break;
    }

    fprintf(stderr, "\n");
}

int main()
{
    FILE *stream = stdin;

    // FIXME: Seed the random number generator
    // FIXME: how to seed? srandom();

    lcp_init();
    lcp_send_conf_req(stdout);

    while (!feof(stream)) {
        uint8_t buffer[2048];
        int len = hdlc_read_frame(stream, buffer);
        if (len > 0 && hdlc_check_frame(buffer, len)) {
            handle_frame(stdout, buffer, len);
        }
    }

}
