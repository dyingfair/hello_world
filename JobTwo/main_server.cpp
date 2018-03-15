#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <sys/time.h>
#include <pthread.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
using namespace std;

#define TCP_CONNECT_PORT    6666
#define UDP_CONNECT_PORT    7777

void *thread_handle_tcp_function(void *arg);
void *thread_handle_udp_function(void *arg);

#define TCP_CONNECT_PORT    6666
#define UDP_CONNECT_PORT    7777
#define SERVER_IP           "127.0.0.1"
#define GREET_WORD          "hello"
#define STOCK_CODE_LEN      6
#define LISTEN_MAC_NUM      10

struct StockTradeItem {
    char             stock_code[7];
    bool             trade_type;     //0 : buy; 1: sell
    unsigned int     trade_qulity;
    float            trade_price;
};

struct StockTradeItem allTradeItems[11] = {
    {"600000", 1, 10000, 12.34},
    {"600001", 0, 20000, 10.30},
    {"600002", 1, 30000, 4.4},
    {"600003", 0, 40000, 7.98},
    {"600004", 0, 50000, 9.30},
    {"600005", 0, 60000, 12.4},
    {"600006", 1, 70000, 12.34},
    {"600007", 1, 80000, 12.34},
    {"600008", 1, 90000, 700.34},
    {"600009", 0, 11000, 23.89},
    {"600010", 1, 12000, 30.10}
};

ssize_t netWrite(int sock_fd, const void *buf, size_t nbytes)
{
    const char *cbuf = (const char *)buf;
    ssize_t count;
    size_t rem = nbytes;
    while (rem > 0)
    {
        count = send (sock_fd, cbuf, rem, 0);
        if (count < 0)
        {
            if (errno == EINTR)
                continue;
            else
                return count;
        }
        cbuf += count;
        rem -= count;
    }
    return nbytes;
}

ssize_t netRead(int sock_fd, void *buf, size_t nbytes)
{
    char *cbuf = (char *)buf;
    ssize_t count;
    size_t rem = nbytes;
    while (rem > 0)
    {
        count = recv (sock_fd, cbuf, rem, 0);
        if (count < 0)
        {
            return count;
        }
        else if (count == 0)
        {
            return count;
        }
        cbuf += count;
        rem -= count;
    }
    return nbytes;
}

int getTheTradeInfo(const char *stock_code, struct StockTradeItem &item)
{
    if(NULL == stock_code)
        return -1;
    unsigned int id;
    const unsigned int item_size = sizeof(allTradeItems) / sizeof(StockTradeItem);
    for(id = 0; id < item_size; ++id)
    {
        if(!strncmp(stock_code, allTradeItems[id].stock_code, strlen(allTradeItems[id].stock_code)))
        {
            memcpy(&item, &allTradeItems[id], sizeof(allTradeItems[id]));
            return 0;
        }
    }
    if(item_size == id)
    {
        snprintf(item.stock_code, STOCK_CODE_LEN+1, stock_code);
        item.trade_price  = 0.0f;
        item.trade_qulity = 0;
        item.trade_type   = 0;
    }
    return -1;
}

int main(int argc, char **argv)
{
    int res;
    pthread_t tcp_thread;
    pthread_t udp_thread;

    res= pthread_create(&tcp_thread, NULL, thread_handle_tcp_function, NULL);
    if(0 != res)
    {
        cout<<"Thread creation(tcp) failed"<<endl;
        return -1;
    }
    res= pthread_create(&udp_thread, NULL, thread_handle_udp_function, NULL);
    if(0 != res)
    {
        cout<<"Thread creation(udp) failed"<<endl;
        return -1;
    }
    while(1)
    {
        sleep(10);
    }
    return 0;
}

void *thread_handle_tcp_function(void *arg)
{
    int server_sockfd, client_sockfd;
    socklen_t server_len, client_len;
    struct sockaddr_in server_address;
    struct sockaddr_in client_address;
    int result;

    fd_set readfds, testfds;
    server_sockfd = socket(AF_INET, SOCK_STREAM, 0);

    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = htonl(INADDR_ANY);
    server_address.sin_port = htons(TCP_CONNECT_PORT);
    server_len = sizeof(server_address);

    bind(server_sockfd, (struct sockaddr *)&server_address, server_len);
    listen(server_sockfd, LISTEN_MAC_NUM);

    FD_ZERO(&readfds);
    FD_SET(server_sockfd, &readfds);
    while(1)
    {
        int fd;
        int nread;
        testfds = readfds;
        cout<< "server waiting"<<endl;
        result = select(FD_SETSIZE, &testfds, (fd_set *)0, (fd_set *)0, (struct timeval *) 0);

        if(result < 1)
        {
            perror("server error");
            exit(1);
        }

        for(fd = 0; fd < FD_SETSIZE; fd++)
        {
            if(FD_ISSET(fd,&testfds))
            {
                if(fd == server_sockfd)
                {
                    client_len = sizeof(client_address);
                    client_sockfd = accept( server_sockfd,
                                            (struct sockaddr *)&client_address,
                                            &client_len);
                    //say hello to the client
                    netWrite(client_sockfd, GREET_WORD, strlen(GREET_WORD));
                    FD_SET(client_sockfd, &readfds);
                }
                else
                {
                    /* If close is received, the client has gone away and we remove
                       it from the descriptor set. Otherwise, we 'serve' the client
                       as in the previous examples.  */
                    ioctl(fd, FIONREAD, &nread);
                    if(nread == 0)
                    {
                        close(fd);
                        FD_CLR(fd, &readfds);
                        cout<<"removing client on fd "<<fd<<endl;
                    }
                    else
                    {
                        char stock_code[STOCK_CODE_LEN + 1] = {0};
                        netRead(fd, stock_code, STOCK_CODE_LEN);

                        struct StockTradeItem item;
                        getTheTradeInfo(stock_code, item);
                        netWrite(fd, &item, sizeof(item));
                    }
                }
            }
        }
    }
}

void *thread_handle_udp_function(void *arg)
{
    struct sockaddr_in servaddr, cliaddr;
    socklen_t cliaddr_len;
    int sockfd;
    char str[INET_ADDRSTRLEN];
    int i, len;

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(UDP_CONNECT_PORT);

    bind(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr));
    while(1)
    {
        char stock_code[STOCK_CODE_LEN + 1] = {0};
        cliaddr_len = sizeof(cliaddr);
        len = recvfrom(sockfd, stock_code, STOCK_CODE_LEN, 0, (struct sockaddr*)&cliaddr, &cliaddr_len);
        if(len == -1)
        {
            cerr << "recvfrom error" << endl;
        }
        printf("received from %s at PORT %d\n",
                inet_ntop(AF_INET, &cliaddr.sin_addr, str, sizeof(str)),
                ntohs(cliaddr.sin_port));

        struct StockTradeItem item;
        getTheTradeInfo(stock_code, item);
        len = sendto(sockfd, &item, sizeof(item), 0, (struct sockaddr*)&cliaddr, sizeof(cliaddr)) ;
        if(len == -1)
        {
            cerr << "sendto error!\n"<<endl;
        }
    }
    close(sockfd);
}
