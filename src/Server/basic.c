#include <map.h>
#include <console.h>
#include <server.h>

LRU_cache* _url_cache = NULL;

/**
 * @brief Start dns_realy  server
 *
 * @param server Socket pointer
 */
void start(Socket* server)
{
    if(server == NULL)
    {
        consoleLog(DEBUG_L0, "> no server created\n");
        exit(-1);
    }

    /* bind port */
    if(bind(server->_fd, (struct sockaddr*)&server->_addr, sizeof(server->_addr)) < 0)
    {
        consoleLog(DEBUG_L0, RED"> bind port %d failed. code %d\n", ntohs(server->_addr.sin_port),ERROR_CODE);
        exit(-1);
    }

    /* cache service init */
    if(LRU_cache_init(&_url_cache) != LRU_OP_SUCCESS){
        consoleLog(DEBUG_L0, RED"> cache service error.\n", ntohs(server->_addr.sin_port),ERROR_CODE);
        exit(-1);
    }

/*******************************  TEST  **********************************/
    // DNS_entry temp;
    // DNS_entry_set(&temp,"baidu.com","1.2.3.4",TYPE_A);
    // LRU_entry_add(_url_cache,&temp);
    // DNS_entry_set(&temp,"baidu.com","4.3.2.1",TYPE_A);
    // LRU_entry_add(_url_cache,&temp);

    // mylist_head* p;
    // mylist_for_each(p,&_url_cache->head){
    //     printf("p: %lld",p);
    // }

    // DNS_entry* dest;
    // int res = LRU_cache_find(_url_cache,&temp,&dest);
    // printf("result: %d\n",res);
    // for(int i = 0; i < res; i++){
    //     printf("ip:%s type:%d\n",dest[i].ip,dest[i].type);
    // }
    

/*******************************  TEST  **********************************/

    consoleLog(DEBUG_L0,BOLDWHITE"> cache service start. cache capacity: %d\n",LRU_CACHE_LENGTH);
    consoleLog(DEBUG_L0, BOLDWHITE"> server start. debug level L%d\n", __DEBUG__);
    consoleLog(DEBUG_L0, BOLDWHITE"> local dns server: %s\n",_local_dns_addr);

    int fromlen = sizeof(struct sockaddr_in);
    fd_set fds;

    for(;;)
    {
        FD_ZERO(&fds);
        FD_SET(server->_fd,&fds);
        SOCKET ret = select(server->_fd+1,&fds,NULL,NULL,NULL);
        if(ret <= 0){
            continue;
        }
        if(FD_ISSET(server->_fd,&fds)){
            /* udp wait for new connection */
            thread_args* args = malloc(sizeof(thread_args));
            args->server = server;
            args->buf_len = recvfrom(server->_fd, args->buf, BUFFER_SIZE, 0, (struct sockaddr*)&args->connect._addr, &fromlen);

            if(args->buf_len > 0)
            {
                threadCreate(connectHandle, (void*)args);
            }
            else
            {
                free(args);
            }
        }
        
    }

    /* close server */
    socketClose(server);

}


/**
 * @brief new connect thread handler
 *
 * @param param new thread param
 * @return void*
 */
void* connectHandle(void* param)
{

    /* thread params parse */
    thread_args* args = (thread_args*)param;

    Socket* server = args->server;      //dns-relay server
    Socket* from = &args->connect;      //dns reqeust/response from

    Packet* p = packetParse(args->buf, args->buf_len);
    if(p == NULL)
    {
        free(args);
        threadExit(10);
    }

    int ret;

    if(GET_QR(p->FLAGS) == 1)
    {
        /* recv from local dns server -- query result */
        consoleLog(DEBUG_L0, BOLDBLUE"> recv query result\n");

        /* check packet info */
        packetCheck(p);

        /* add to cache */

        consoleLog(DEBUG_L0, BOLDMAGENTA"> cache len %d\n",urlStore(p));


        uint16_t origin;
        struct sockaddr_in from;

        /* query addr */
        if(queryMap(p->ID, &origin, &from) == 0)
        {
            SET_ID(args->buf, &origin);
            ret = sendto(server->_fd, args->buf, args->buf_len, 0, (struct sockaddr*)&from, sizeof(from));
        }
    }
    else
    {
        /* recv from client -- query request */
        consoleLog(DEBUG_L0, BOLDBLUE"> recv query request\n");
        /* check packet info */
        packetCheck(p);

        if(urlQuery(p) == 0)    //no result from url table
        {
            consoleLog(DEBUG_L0, BOLDYELLOW"> query from dns server\n");

            /* add to map */
            uint16_t converted = addToMap(p->ID,&args->connect._addr);
            SET_ID(args->buf,&converted);

            /* send to dns server */
            ret = sendto(server->_fd, args->buf, args->buf_len, 0, (struct sockaddr*)&_dns_server._addr, sizeof(struct sockaddr));

        }
        else
        {
            consoleLog(DEBUG_L0, BOLDGREEN"> query OK. send back\n");

            /* generate response package */
            int buff_len;
            char* buff = responseFormat(&buff_len, p);

            /* send back to client */
            ret = sendto(server->_fd, buff, buff_len, 0, (struct sockaddr*)&args->connect._addr, sizeof(args->connect._addr));
            free(buff);
        }
    }

    if(ret < 0)
    {
        consoleLog(DEBUG_L0, BOLDRED"> send failed. code %d\n", ERROR_CODE);
    }

    /* mem free */
    packetFree(p);
    free(args);

    /* close socket */
    socketClose(&args->connect);
    threadExit(10);
}

