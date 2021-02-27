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
                // Push back to IO stream, so that we know we are at the start of a
                // function when we read the next frame
                ungetc(0x7e, stream);
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

int hdlc_check_frame(const uint8_t *buffer, int len)
{
    if (len < 8) {
        fprintf(stderr, "Warning: Ignoring frame that is less than 8 bytes\n");
        return 0;
    }

    // Address
    if (buffer[0] != 0xff) {
        fprintf(stderr, "Warning: HDLC address is not 0xFF\n");
        return 0;
    }

    // Control Field
    if (buffer[1] != 0x03) {
        fprintf(stderr, "Warning: HDLC control field is not 0x03\n");
        return 0;
    }

    // Check FCS (checksum)
    uint16_t fcs = calculate_fcs16(buffer, len - 2);
    if ((fcs & 0xff) != buffer[len - 2] || ((fcs >> 8) & 0xff) != buffer[len - 1]) {
        fprintf(stderr, "Frame Check Sequence error\n");
        fprintf(stderr, "  Expected: 0x%2.2x%2.2x\n", buffer[len - 2], buffer[len - 1]);
        fprintf(stderr, "  Actual: 0x%2.2x%2.2x\n", fcs & 0xff, (fcs >> 8) & 0xff);
        return 0;
    }

    // Valid
    return 1;
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

int hdlc_write_frame(FILE *stream, const uint8_t *buffer, int len)
{
    uint16_t fcs;

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
