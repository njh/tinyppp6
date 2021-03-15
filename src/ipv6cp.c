#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "tinyppp6.h"


uint8_t our_interface_id[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02};
uint8_t their_interface_id[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01};


// https://tools.ietf.org/html/rfc5072

void ipv6cp_init()
{
}

void ipv6cp_reply_conf_req(FILE *stream, uint8_t *buffer, int len)
{
    // FIXME: check if there are any options we don't like

    // Better way of formatting this?
    fprintf(stderr, "tinyppp6 remote interface ID is: fe80::%2.2x%2.2x:%2.2x%2.2x:%2.2x%2.2x:%2.2x%2.2x\n",
            BUF_GET_UINT8(buffer, 10),
            BUF_GET_UINT8(buffer, 11),
            BUF_GET_UINT8(buffer, 12),
            BUF_GET_UINT8(buffer, 13),
            BUF_GET_UINT8(buffer, 14),
            BUF_GET_UINT8(buffer, 15),
            BUF_GET_UINT8(buffer, 16),
            BUF_GET_UINT8(buffer, 17)
           );

    fprintf(stderr, "tinyppp6 send: Sending IPV6CP Conf-Ack\n");

    // Change IPV6CP code to from ConfReq to ConfAck
    BUF_SET_UINT8(buffer, 4, IPV6CP_CONF_ACK);

    // FIXME: Store their interface ID

    hdlc_write_frame(stream, PPP_PROTO_IPV6CP, buffer, len);
}

void ipv6cp_handle_frame(FILE *stream, uint8_t *buffer, int len)
{
    int code = buffer[4];
    int id = buffer[5];

    fprintf(stderr, "tinyppp6 recv: IPv6CP (%d, len=%d, id=%x)\n", code, len, id);

    switch (code) {
        case IPV6CP_CONF_REQ:
            fprintf(stderr, "tinyppp6 recv: IPV6CP Configure-Request\n");
            ipv6cp_reply_conf_req(stream, buffer, len);
            break;

        case IPV6CP_CONF_ACK:
            fprintf(stderr, "tinyppp6 recv: IPV6CP Configure-Ack\n");
            // Store our interface ID?
            break;

        case IPV6CP_CONF_NAK:
            fprintf(stderr, "tinyppp6 recv: IPV6CP Configure-Nak\n");
            break;

        case IPV6CP_CONF_REJ:
            fprintf(stderr, "tinyppp6 recv: IPV6CP Configure-Reject\n");
            break;

        case IPV6CP_TERM_REQ:
            fprintf(stderr, "tinyppp6 recv: IPV6CP Terminate-Request\n");
            break;

        case IPV6CP_TERM_ACK:
            fprintf(stderr, "tinyppp6 recv: IPV6CP Terminate-Ack\n");
            break;

        case IPV6CP_CODE_REJ:
            fprintf(stderr, "tinyppp6 recv: IPV6CP Code-Reject\n");
            break;

        default:
            fprintf(stderr, "tinyppp6 recv: Unknown IPV6CP code: %d\n", code);
            break;
    }
}

void ipv6cp_send_conf_req(FILE *stream)
{
    uint8_t buffer[32];

    fprintf(stderr, "tinyppp6 send: Sending IPV6CP Conf-Req\n");

    BUF_SET_UINT8(buffer, 0, IPV6CP_CONF_REQ);
    BUF_SET_UINT8(buffer, 1, 0x01); // IPV6CP ID
    BUF_SET_UINT16(buffer, 2, 14);  // IPV6CP length

    BUF_SET_UINT8(buffer, 4, 0x01); // Interface Identifier Option
    BUF_SET_UINT8(buffer, 5, 10); // Length

    // Copy in the interface identifier
    memcpy(&buffer[6], our_interface_id, 8);

    hdlc_write_frame(stream, PPP_PROTO_IPV6CP, buffer, 14);
}
