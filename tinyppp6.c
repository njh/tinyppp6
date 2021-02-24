#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "tinyppp6.h"


void handle_frame(uint8_t *buffer, int len)
{
    for (int i = 0; i < len; i++) {
        fprintf(stderr, "%2.2x ", buffer[i]);
    }
    fprintf(stderr, "\n");

    if (len < 4) {
        fprintf(stderr, "Warning: Ignoring frame that is less than 4 bytes\n");
        return;
    }

    // Address
    if (buffer[0] != 0xff) {
        fprintf(stderr, "Warning: HDLC address is not 0xFF\n");
        // FIXME: ignore frame
        return;
    }

    // Control Field
    if (buffer[1] != 0x03) {
        fprintf(stderr, "Warning: HDLC control field is not 0x03\n");
        // FIXME: ignore frame
        return;
    }

    // Check FCS (checksum)
    uint16_t fcs = calculate_fcs16(buffer, len - 2);
    if ((fcs & 0xff) != buffer[len - 2] || ((fcs >> 8) & 0xff) != buffer[len - 1]) {
        fprintf(stderr, "Frame Check Sequence error\n");
        fprintf(stderr, "  Expected: 0x%2.2x%2.2x\n", buffer[len - 2], buffer[len - 1]);
        fprintf(stderr, "  Actual: 0x%2.2x%2.2x\n", fcs & 0xff, (fcs >> 8) & 0xff);
        return;
    }

    // FIXME: how do we detect 1-byte protocol number?
    uint16_t protocol = (buffer[2] << 8) | buffer[3];
    switch (protocol) {
        case 0x0001:
            fprintf(stderr, "tinyppp6 recv: padding (len=%d)\n", len);
            break;
        case 0xc021:
            lcp_handle_frame(buffer, len - 2);
            break;
        case 0xc023:
            fprintf(stderr, "tinyppp6 recv: Password Authentication Protocol (len=%d)\n", len);
            break;
        case 0xc025:
            fprintf(stderr, "tinyppp6 recv: Link Quality Report (len=%d)\n", len);
            break;
        case 0xc223:
            fprintf(stderr, "tinyppp6 recv: Challenge Handshake Authentication Protocol (len=%d)\n", len);
            break;
        case 0x8057:
            fprintf(stderr, "tinyppp6 recv: IPv6 Control Protocol (len=%d)\n", len);
            break;
        default:
            fprintf(stderr, "tinyppp6 recv: Unknown 0x%4.4x (len=%d)\n", protocol, len);
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
        if (len > 0) {
            handle_frame(buffer, len);
        }
    }

}
