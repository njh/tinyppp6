#include <stdio.h>
#include <stdint.h>

#include "tinyppp6.h"


int read_frame(FILE* stream, uint8_t *buffer)
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
        fprintf(stderr, "Hunting for start of frame");
      }
      
   } while (1);
  
   return pos;
}

int write_frame(uint8_t *buffer, int buffer_len)
{

    return 0;
}
