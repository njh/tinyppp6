#include <stdio.h>
#include <stdint.h>

#include "tinyppp6.h"



void handle_frame(uint8_t *buffer, int len)
{
   for (int i=0; i<len; i++) {
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
   }

   // Control Field
   if (buffer[1] != 0x03) {
      fprintf(stderr, "Warning: HDLC control field is not 0x03\n");
      // FIXME: ignore frame
   }

   // FIXME: how do we detect 1-byte protocol number?
   int protocol = (buffer[2] << 8) | buffer[3];
   switch (protocol) {
     case 0x0001:
       fprintf(stderr, " recv: padding\n");
     break;
     case 0xc021:
       handle_lcp(buffer+4, len-4);
     break;
     case 0xc023:
       fprintf(stderr, " recv: Password Authentication Protocol\n");
     break;
     case 0xc025:
       fprintf(stderr, " recv: Link Quality Report\n");
     break;
     case 0xc223:
       fprintf(stderr, " recv: Challenge Handshake Authentication Protocol\n");
     break;
     case 0x8057:
       fprintf(stderr, " recv: IPv6 Control Protocol\n");
     break;
     default:
       fprintf(stderr, " recv: Unknown 0x%4.4x\n", protocol);
     break;
   }
   
   // FIXME: check FCS (checksum)
   uint16_t fcs = calculate_fcs16(buffer, len-2);
   if ((fcs & 0xff) != buffer[len-2] || ((fcs >> 8) & 0xff) != buffer[len-1]) {
      fprintf(stderr, "Frame Check Sequence error\n");
      fprintf(stderr, "  Expected: 0x%2.2x%2.2x\n", buffer[len-2], buffer[len-1]);
      fprintf(stderr, "  Actual: 0x%2.2x%2.2x\n", fcs & 0xff, (fcs >> 8) & 0xff);
   }
   
   fprintf(stderr, "\n");
}


int main()
{
   FILE* stream = stdin;

   while(!feof(stream)) {
     uint8_t buffer[2048];
     int len = read_frame(stream, buffer);
     fprintf(stderr, "Received Frame (Len=%d)\n", len);
     if (len > 0) {     
       handle_frame(buffer, len);
     }
   }

}