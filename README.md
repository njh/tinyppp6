tinyppp6
========

Tiny implementation of PPP for IPv6.

This project is a proof of concept to see how much work it is to build a minimal IPv6-only implementation of the [Point-to-Point Protocol].

It is completely stand-alone process and does not interact with the kernel's network stack.

It has been tested against version 2.4.7 of [Linux pppd] (running on Debian Linux).


Features
--------

* Responds to LCP pings
* Responds to IPv6 ICMP pings on link-local
* Rejects Unknown Protocols
* Prints out UDP packets sent to port 1234
* Connects using the `pty` option in [Linux pppd]


Not Implemented
---------------

* IPv4
* SLAAC
* DHCPv6
* Any header / protocol compression
* IPv6 Extension Headers are not supported
* TCP


Running tinyppp6
----------------

First compile tinyppp6 using the `make` command.

Use the following to start Linux pppd and connect it to a tinyppp6 sub-process:

```
sudo pppd \
 nodetach \
 noauth \
 noip \
 +ipv6 \
 ipv6 ::1,::2 \
 pty './src/tinyppp6'
```

The following Link-local IPv6 addresses will then be setup:

* `fe80::1` - the Linux pppd end of the PPP connection
* `fe80::2` - the tinyppp6 end of the PPP connection


You should then be able to send ICMPv6 pings to tinyppp6 using:

```
ping6 fe80::2%ppp0
PING fe80::2%ppp0(fe80::2%ppp0) 56 data bytes
64 bytes from fe80::2%ppp0: icmp_seq=1 ttl=64 time=0.588 ms
64 bytes from fe80::2%ppp0: icmp_seq=2 ttl=64 time=0.555 ms
64 bytes from fe80::2%ppp0: icmp_seq=3 ttl=64 time=0.645 ms
64 bytes from fe80::2%ppp0: icmp_seq=4 ttl=64 time=0.601 ms
```

And send UDP packets to tinyppp6 using:
```
echo -n "Hello World" | socat - UDP6:[fe80::2%ppp0]:1234
```


[Point-to-Point Protocol]:  https://en.wikipedia.org/wiki/Point-to-Point_Protocol
[Linux pppd]:  https://ppp.samba.org/
