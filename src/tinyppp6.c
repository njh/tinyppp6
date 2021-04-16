#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "tinyppp6.h"


void handle_frame(int fd, uint8_t *buffer, int len)
{
    uint16_t protocol = (buffer[2] << 8) | buffer[3];
    uint8_t *payload = &buffer[4];
    int payload_len = len - 6;

    switch (protocol) {
        case PPP_PROTO_PAD:
            fprintf(stderr, "tinyppp6 recv: padding (len=%d)\n", payload_len);
            break;
        case PPP_PROTO_LCP:
            lcp_handle_frame(fd, payload, payload_len);
            break;
        case PPP_PROTO_IPV6CP:
            ipv6cp_handle_frame(fd, payload, payload_len);
            break;
        case PPP_PROTO_IPV6:
            ipv6_handle_packet(fd, payload, payload_len);
            break;
        default:
            fprintf(stderr, "tinyppp6 recv: Unknown 0x%4.4x (len=%d)\n", protocol, payload_len);
            lcp_reject_protocol(fd, protocol, payload, payload_len);
            break;
    }

    fprintf(stderr, "\n");
}

void usage()
{
    fprintf(stderr, "Usage: tinyppp6 [<serial device>]\n");
    exit(EXIT_FAILURE);
}

int main(int argc, char **argv)
{
    int input = STDIN_FILENO;
    int output = STDOUT_FILENO;

    // FIXME: Seed the random number generator
    // FIXME: how to seed? srandom();

    // Check command line arguments
    if (argc == 1) {
        input = STDIN_FILENO;
        output = STDOUT_FILENO;
    } else if (argc == 2 && strlen(argv[1]) > 1 && argv[1][0] != '-') {
        input = output = serial_open(argv[1]);
    } else {
        usage();
    }

    hdlc_init();
    lcp_init();
    lcp_send_conf_req(output);

    // FIXME: abort on EOF
    while (1) {
        if (hdlc_bytes_available(input) > 0) {
            uint8_t buffer[PACKET_BUF_SIZE];
            int len = hdlc_read_frame(input, buffer);
            if (len > 0 && hdlc_check_frame(buffer, len)) {
                handle_frame(output, buffer, len);
            }
        }
    }

    close(input);
    close(output);
}
