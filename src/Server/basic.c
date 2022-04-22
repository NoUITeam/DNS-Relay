#include <main.h>
#include <console.h>
#include <server.h>

struct sockaddr pre_from;

void Start(Server *server){
    /* basic server data */
    char buff[BUFFER_SIZE] = {0};
    int buf_len = 0;
    struct sockaddr from;
    int from_len = sizeof(server->sock_addr);
    /* server run */
    while(1){
        /* listening */
        buf_len = recvfrom(server->socket, buff, BUFFER_SIZE, 0, &from, &from_len);
        /* packet received */
        if(buf_len > 0){
            uint16_t flag = ntohs(*(uint16_t *)(buff + 2));
            printf("> QR %d\n", GET_QR(flag));

            /* Simple Relay Query */
            if(GET_QR(flag) == 1){
                ConsoleLog(0, DEBUG_L0, "> sendto client");
                sendto(server->socket, buff, buf_len, 0, &pre_from, from_len);
                memset(buff, 0, BUFFER_SIZE);
                continue;
            }
            else{
                memcpy(&pre_from, &from, sizeof(from));
            }

            /* packet handle */
            Packet *p = PacketParse(buff, buf_len);
            if(p != NULL){
                PacketCheck(p);
                if(UrlQuery(p, RECORDS, R_NUM) == 0){

                    /* Simple Relay Query */
                    ConsoleLog(0, DEBUG_L0, "> sendto server");
                    sendto(server->socket, buff, buf_len, 0, &server->local_dns, from_len);
                    memset(buff, 0, BUFFER_SIZE);
                    continue;
                }

                int len = 0;
                char *buf = ResponseFormat(&len, p);
                //Re-Parse
                //Packet *temp = PacketParse(buf, len);
                //PacketCheck(temp);
                sendto(server->socket, buf, len, 0, &from, from_len);
                ConsoleLog(0, DEBUG_L0, "> send from dnsrelay");
                PacketFree(p);
                //PacketFree(temp);
            }
            /* buff flush */
            memset(buff, 0, BUFFER_SIZE);
        }
    }
}



int ServerInit(Server *server){
    /* struct pointer judge */
    if(server == NULL){
        return ConsoleLog(ERROR, DEBUG_L0, "> Exit : Pointer Error");
    }

    /* server socket create */
    server->socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if(server->socket < 0){
        return ConsoleLog(ERROR, DEBUG_L0, "> Exit : Socket Error");
    }

    /* server init */
    server->sock_addr.sin_family = AF_INET;
    server->sock_addr.sin_addr.s_addr = INADDR_ANY;
    server->sock_addr.sin_port = htons(53);

    /* local dns server link */
    server->local_dns.sin_family = AF_INET;
    server->local_dns.sin_addr.s_addr = inet_addr(DEFAULT_LOCAL_DNS_ADDR);
    server->local_dns.sin_port = htons(53);

    /* reuser addr enable*/
    int temp = 1;
    setsockopt(server->socket, SOL_SOCKET, SO_REUSEADDR, &temp, sizeof(temp));

    /* bind port */
    if(bind(server->socket, (struct sockaddr *)&server->sock_addr, sizeof(server->sock_addr)) < 0){
        return ConsoleLog(ERROR, DEBUG_L0, "> Exit : Port Bind Error");
    }
    return ConsoleLog(SUCCESS, DEBUG_L0, "> Server Create Success");
}


