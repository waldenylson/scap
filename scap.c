/*---------------------------------------------------------------------------

 sCap is simple application for scaning and capturing incomming packets
 useing sockets.

 Copyright (c) 2015 Sun Dro (a.k.a. 7th Ghost / kala13x)
 Web: http://off-sec.com/ ; E-Mail: kala0x13@gmail.com

 This is free software; you can redistribute it and/or
 modify it under the terms of the GNU Lesser General Public
 License as published by the Free Software Foundation; either
 version 3 of the License, or (at your option) any later version.

 This software is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 Lesser General Public License for more details.

---------------------------------------------------------------------------*/


#include "stdinc.h"
#include "packet.h"
#include "info.h"
#include "slog.h"

/* Max size of buffer */
#define MAXMSG 8196


/*---------------------------------------------
| Structure of packets
---------------------------------------------*/
typedef struct {
    int tcp;
    int udp; 
    int icmp;
    int igmp; 
    int total;
    int other;
} ScapPackets;


/*---------------------------------------------
| Initialise sCap packets
---------------------------------------------*/
void init_scap_packets(ScapPackets * scap) 
{
    scap->tcp = 0;
    scap->udp = 0;
    scap->icmp = 0;
    scap->igmp = 0;
    scap->total = 0;
    scap->other = 0;
}


/*---------------------------------------------
| Read signal
---------------------------------------------*/
void sig_handler(int sig) 
{
    /* Handle signals */
    if (sig == SIGILL || sig == SIGSEGV) 
        slog(0, "[ERROR] Can not process data");

    if (sig == SIGPIPE)
        slog(0, "[ERROR] Broken Pipe");

    if (sig == SIGINT) 
        slog(0, "[LIVE] Cleanup on exit");

    exit(-1);
}


/*---------------------------------------------
| Create socket
---------------------------------------------*/
int create_socket()
{
    /* Used variables */
    int sock;

    /* Create raw socket */
    sock = socket(AF_INET , SOCK_RAW , IPPROTO_TCP);
    if (sock < 0)
    {
        slog(0, "[ERROR] Can not create socket (maybe permissions?)");
        exit(EXIT_FAILURE);
    }

    return sock;
}


/*---------------------------------------------
| Process packets
---------------------------------------------*/
void read_scap_packet(ScapPackets * scap, 
                unsigned char* buf, int size)
{
    /* Used variables */
    struct iphdr* iph = (struct iphdr*)buf;
    char* out_line;

    /* Get IP header */
    switch (iph->protocol)
    {
        case 1:
            ++scap->icmp;
            break;
        case 2:
            ++scap->igmp;
            break;
        case 6:
            ++scap->tcp;
            log_tcp(buf, size);
            break;
        case 17:
            ++scap->udp;
            break;
        default:
            ++scap->other;
            break;
    }

    /* Total packet counter */
    ++scap->total;

    /* Return status in slog format */
    out_line = ret_slog("[LIVE] TCP: %d   UDP: %d   ICMP: %d   IGMP: %d   Other: %d   Total: %d",
        scap->tcp, scap->udp, scap->icmp, scap->igmp, scap->other, scap->total);

    /* Print status */
    printf("%s\r", out_line);
}


/*---------------------------------------------
| Main function
---------------------------------------------*/
int main(int argc, char **argv)
{
    /* Used variables */
    ScapPackets scap;
    unsigned char buf[MAXMSG];
    struct sockaddr addr;
    int sock, data, size;

    /* Read signals */
    signal(SIGPIPE, sig_handler);
    signal(SIGINT, sig_handler);
    signal(SIGSEGV, sig_handler);
    signal(SIGILL , sig_handler);

    /* Initialise scap */
    init_scap_packets(&scap);
    init_slog("scap", 2);

    /* Greet */
    greet();

    /* Create raw socket */
    sock = create_socket();

    /* Main loop (never ends) */
    while(1)
    {
        /* Get packet size */
        size = sizeof addr;

        /* Get packet */
        data = recvfrom(sock, buf, sizeof(buf), 0, &addr, (socklen_t *)&size);
        if(data < 0)
        {
            slog(0, "[ERROR] Can not get packets");
            break;
        }
        else read_scap_packet(&scap, buf, size);
    }

    /* Close socket */
    close(sock);

    return 0;
}