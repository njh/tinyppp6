#include <stdio.h>
#include <stdint.h>

#include "tinyppp6.h"


// PPP in HDLC-like Framing is described in:
// https://tools.ietf.org/html/rfc1662


int hdlc_read_frame(FILE *stream, uint8_t *buffer)
{
    int pos = 0;
    int in_escape = 0;
    int hunt = 1;

    do {
        int chr = fgetc(stream);
        if (chr == EOF) break;

        if (chr == 0x7e) {
            if (hunt == 1) {
                // Start of frame
                hunt = 0;
                continue;
            } else {
                // End of frame
                // FIXME: push back to file using ungetc() - for case where there is a single 0x7e between frames?
                break;
            }
        }

        if (hunt == 0) {
            if (chr < 0x20) {
                fprintf(stderr, "Ignoring: 0x%2.2x\n", chr);
            } else if (chr == 0x7d) {
                // next character is escaped
                in_escape = 1;
            } else if (in_escape) {
                // Control char
                buffer[pos++] = chr ^ 0x20;
                in_escape = 0;
            } else {
                buffer[pos++] = chr;
            }
        } else {
            fprintf(stderr, "Hunting for start of frame but got: %2.2x\n", chr);
        }

    } while (1);

    return pos;
}

void hdlc_write_frame_byte(FILE *stream, uint8_t chr)
{
    if (chr == 0x7d || chr == 0x7e || chr < 0x20) {
        fputc(0x7d, stream);
        fputc(chr ^ 0x20, stream);
    } else {
        fputc(chr, stream);
    }
}

int hdlc_write_frame(FILE *stream, uint8_t *buffer, int len)
{
    uint16_t fcs;

    for (int i = 0; i < len; i++) {
        fprintf(stderr, "%2.2x ", buffer[i]);
    }
    fprintf(stderr, "\n");

    // Start of frame
    fputc(0x7e, stream);

    // Write to output (with byte stuffing)
    for (int i = 0; i < len; i++) {
        hdlc_write_frame_byte(stream, buffer[i]);
    }

    // Calculate checksum and write to output
    fcs = calculate_fcs16(buffer, len);
    // FIXME: is this different for big-endian/little-endian processors?
    hdlc_write_frame_byte(stream, fcs & 0x00FF);
    hdlc_write_frame_byte(stream, (fcs & 0xFF00) >> 8);

    // End of frame
    fputc(0x7e, stream);
    fflush(stream);

    return 0;
}
