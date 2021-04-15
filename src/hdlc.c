#include <stdio.h>
#include <stdint.h>
#include <sys/select.h>
#include <string.h>
#include <unistd.h>

#include "tinyppp6.h"


// PPP in HDLC-like Framing is described in:
// https://tools.ietf.org/html/rfc1662

enum {
    HDLC_STATE_HUNTING,
    HDLC_START_FRAME,
    HDLC_MID_FRAME,
    HDLC_IN_ESCAPE
};

int hdlc_state = HDLC_STATE_HUNTING;
uint8_t read_buffer[PACKET_BUF_SIZE * 2];
int bytes_read = 0;

void hdlc_init()
{
    hdlc_state = HDLC_STATE_HUNTING;
    bytes_read = 0;
}


// Returns:
//  - negative number on error
//  - 0 if nothing available
//  - 1 or more if bytes are available
int hdlc_bytes_available(FILE *stream)
{
    int fd = fileno(stream);
    fd_set rfds;
    struct timeval tv;
    int retval;

    // Do we already have bytes in the buffer?
    if (bytes_read > 0) {
        return bytes_read;
    }

    FD_ZERO(&rfds);
    FD_SET(fd, &rfds);

    // Wait up to 1 second
    tv.tv_sec = 1;
    tv.tv_usec = 0;

    retval = select(fd + 1, &rfds, NULL, NULL, &tv);
    if (retval < 0) {
        perror("select() failed");
        return retval;
    }

    return retval;
}

int hdlc_read_byte(FILE *stream)
{
    if (bytes_read <= 0) {
        // Fill up the read buffer
        int fd = fileno(stream);
        bytes_read = read(fd, read_buffer, sizeof(read_buffer));
    }

    if (bytes_read > 0) {
        int byte = read_buffer[0];
        bytes_read--;

        if (bytes_read > 0) {
            // FIXME: this is really inefficient
            memmove(read_buffer, &read_buffer[1], bytes_read);
        }

        return byte;
    } else {
        // Nothing available
        return -1;
    }
}

// This function is blocking
int hdlc_read_frame(FILE *stream, uint8_t *buffer)
{
    int pos = 0;
    int in_escape = 0;

    do {
        int chr = hdlc_read_byte(stream);
        if (chr == EOF) break;

        if (chr == 0x7e) {
            if (hdlc_state == HDLC_STATE_HUNTING) {
                // Start of frame
                hdlc_state = HDLC_START_FRAME;
                continue;
            } else {
                // End of frame
                hdlc_state = HDLC_START_FRAME;
                break;
            }
        }

        if (hdlc_state != HDLC_STATE_HUNTING) {
            if (chr < 0x20) {
                fprintf(stderr, "Ignoring: 0x%2.2x\n", chr);
            } else if (chr == 0x7d) {
                // Next character is escaped
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
    uint16_t fcs = fcs16_calculate(buffer, len - 2);
    if ((fcs & 0xff) != buffer[len - 2] || ((fcs >> 8) & 0xff) != buffer[len - 1]) {
        fprintf(stderr, "Frame Check Sequence error\n");
        fprintf(stderr, "  Expected: 0x%2.2x%2.2x\n", buffer[len - 2], buffer[len - 1]);
        fprintf(stderr, "  Actual: 0x%2.2x%2.2x\n", fcs & 0xff, (fcs >> 8) & 0xff);
        return 0;
    }

    // Valid
    return 1;
}

void hdlc_append_frame_byte(uint8_t *buffer, int *pos, uint8_t chr)
{
    if (chr == 0x7D || chr == 0x7E || chr < 0x20) {
        buffer[(*pos)++] = 0x7D;
        buffer[(*pos)++] = chr ^ 0x20;
    } else {
        buffer[(*pos)++] = chr;
    }
}

// FIXME: avoid buffer overrun
// Return the length of the encoded frame
int hdlc_encode_frame(uint8_t *buffer, uint16_t protocol, const uint8_t *data, int data_len)
{
    register uint16_t fcs = 0xFFFF;
    int pos = 0;

    // Start of frame
    buffer[pos++] = 0x7E;

    // Write frame header
    // FIXME: make this code more compact
    hdlc_append_frame_byte(buffer, &pos, 0xFF);
    fcs = fcs16_add_byte(fcs, 0xFF);
    hdlc_append_frame_byte(buffer, &pos, 0x03);
    fcs = fcs16_add_byte(fcs, 0x03);

    // Write 16-bit protocol number
    hdlc_append_frame_byte(buffer, &pos, (protocol & 0xFF00) >> 8);
    fcs = fcs16_add_byte(fcs, (protocol & 0xFF00) >> 8);
    hdlc_append_frame_byte(buffer, &pos, protocol & 0x00FF);
    fcs = fcs16_add_byte(fcs, protocol & 0x00FF);

    // Write buffer to output (with byte stuffing)
    for (int i = 0; i < data_len; i++) {
        hdlc_append_frame_byte(buffer, &pos, data[i]);
        fcs = fcs16_add_byte(fcs, data[i]);
    }

    // Write FCS checksum
    // FIXME: is this different for big-endian/little-endian processors?
    fcs = fcs ^ 0xFFFF;
    hdlc_append_frame_byte(buffer, &pos, fcs & 0x00FF);
    hdlc_append_frame_byte(buffer, &pos, (fcs & 0xFF00) >> 8);

    // End of frame
    buffer[pos++] = 0x7E;

    return pos;
}


int hdlc_write_frame(FILE *stream, uint16_t protocol, const uint8_t *data, int data_len)
{
    uint8_t write_buffer[PACKET_BUF_SIZE * 2];
    int encoded_len;
    int result;

    encoded_len = hdlc_encode_frame(write_buffer, protocol, data, data_len);

    // Write to output
    result = fwrite(write_buffer, 1, encoded_len, stream);
    fflush(stream);

    return result;
}
