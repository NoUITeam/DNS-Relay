#ifndef __SERVER_H__
#define __SERVER_H__

#include <main.h>

#define DEFAULT_LOCAL_DNS_ADDR  "10.3.9.44" 

/* Static Records */
static char* RECORDS[][2]={
    {"bing.com","204.79.197.200"},
    {"bing.com","13.107.21.200"},
    {"bupt.edu.cn","10.3.9.161"},
    {"noui.cloud","101.43.201.20"},
    {"unique.com","220.177.198.124"}
};
static int R_NUM = 5;   //Records Num

/* MAX BUFFER SIZE */
#define BUFFER_SIZE     1024

/* TYPE CODE*/
#define TYPE_QNAME      -1
#define TYPE_A          1
#define TYPE_NA         2
#define TYPE_CNAME      5
#define TYPE_MX         15
#define TYPE_TXT        16
#define TYPE_AAAA       28

/* TYPE SIZE */
#define SIZE_TYPE_A     4
#define SIZE_TYPE_AAAA  16

/* RCODE */
#define RCODE_NO_ERROR          0x0
#define RCODE_FORMAT_ERROR      0x1
#define RCODE_SERVER_FAILURE    0x2
#define RCODE_NAME_ERROR        0x3
#define RCODE_NOT_IMPLEMENTED   0x4
#define RCODE_REFUSED           0x5

/* Set FLAGS */
#define SET_QR(FLAG)            FLAG |= 0x8000
#define SET_AA(FLAG)            FLAG |= 0x400
#define SET_RD(FLAG)            FLAG |= 0x100
#define SET_RA(FLAG)            FLAG |= 0x80
#define SET_RCODE(FLAG,CODE)    FLAG = (FLAG & 0xfff0) | CODE

/* Reset FLAGS */
#define RESET_QR(FLAG)      FLAG &= !0x8000
#define RESET_AA(FLAG)      FLAG &= !0x400
#define RESET_RD(FLAG)      FLAG &= !0x100
#define RESET_RA(FLAG)      FLAG &= !0x80

/* Get FLAGS */
#define GET_QR(FLAG)        ({((FLAG & 0x8000) >> 15);})
#define GET_RD(FLAG)        ({((FLAG & 0x100) >> 8);})
#define GET_RA(FLAG)        ({((FLAG & 0x80) >> 7);})
#define GET_RCODE(FLAG)     ({(FLAG & 0x000f);})
#define GET_QNAME_PTR(NAME) ({NAME & 0x3fff;})


/* DNS Server Struct */
typedef struct Server{
    SOCKET  socket;
    struct  sockaddr_in sock_addr;
    struct  sockaddr_in local_dns;
    char    local_dns_addr[17];

}Server;

/* Question Section Struct */
typedef struct Quest{
    char      *QNAME;
    uint16_t  QTYPE;
    uint16_t  QCLASS;

}Quest;

/* Answer Section Struct */
typedef struct Answer{
    uint16_t  NAME;
    uint16_t  TYPE;
    uint16_t  CLASS;
    uint16_t  RDLEN;
    uint32_t  TTL;
    char      *RDATA;

}Answer;

/* Packet form */
typedef struct Packet{
    /* Origin Infomation */
    char     *req_buf;
    int       buf_len;
    /* Header Section */
    uint16_t  ID;
    uint16_t  FLAGS;
    uint16_t  QDCOUNT;
    uint16_t  ANCOUNT;
    /* Question Section */
    Quest     *QUESTS;
    /* Answer Section */
    Answer    *ANS;

}Packet;


/* Server Base */
void    Start(Server *);
int     ServerInit(Server *);
void    RecvHandle(char *buf, int len, struct sockaddr);

/* Address Query */
//int   AddrReadin();
int     UrlQuery(Packet*, char ***records, int num);
int     QnameSearch(char *src, int *res,char ***records, int num);


/* Packet Handle */
Packet  *PacketParse(char *buf, int len);
char    *ResponseFormat(int *len, Packet *);
void    PacketFree(Packet *);

/* Url Resolve */
int     UrlFormat(char *src, void *dest, int mode);
int     UrlParse(void *src, char *dest, int mode);


/* Packet Check */
void    PacketCheck(Packet *);
void    BuffCheck(char *buf, int len);


#endif
