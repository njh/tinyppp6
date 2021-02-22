#include <stdio.h>
#include <stdint.h>

#include "tinyppp6.h"

void handle_lcp(uint8_t *buffer, int len)
{
  int code = buffer[0];
  int id = buffer[1];

   fprintf(stderr, " recv: Link Control Protocol (%d, id=%x)\n", code, id);
   
   switch(code) {
    case 1: // Configure-Request
      fprintf(stderr, " recv: LCP Configure-Request\n");
    break;

    case 2: // Configure-Ack
      fprintf(stderr, " recv: LCP Configure-Ack\n");
    break;

    case 3: // Configure-Nak
      fprintf(stderr, " recv: LCP Configure-Nak\n");
    break;

    case 4: // Configure-Reject
      fprintf(stderr, " recv: LCP Configure-Reject\n");
    break;

    case 5: // Terminate-Request
      fprintf(stderr, " recv: LCP Terminate-Request\n");
    break;

    case 6: // Terminate-Ack
      fprintf(stderr, " recv: LCP Terminate-Ack\n");
    break;

    case 7: // Code-Reject
      fprintf(stderr, " recv: LCP Code-Reject\n");
    break;

    case 8: // Protocol-Reject
      fprintf(stderr, " recv: LCP Protocol-Reject\n");
    break;

    case 9: // Echo-Request
      fprintf(stderr, " recv: LCP Echo-Request\n");
    break;

    case 10: // Echo-Reply
      fprintf(stderr, " recv: LCP Echo-Reply\n");
    break;

    case 11: // Discard-Request
      fprintf(stderr, " recv: LCP Discard-Request\n");
    break;
    
    default:
      fprintf(stderr, " recv: Unknown LCP code: %d\n", code);
    break;
   }
}

