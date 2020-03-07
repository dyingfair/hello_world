#include <iostream>
#include <fstream>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
using namespace std;

#define TCP_CONNECT_PORT       6666
#define SERVER_IP              "127.0.0.1"
#define TRADE_RECORD_FILE      "trade_record_tcp.txt"
#define STOCK_CODE_LEN          6
#define GREET_WORD              "hello"

struct StockTradeItem {
    char             stock_code[7];
    bool             trade_type;     //0: buy; 1: sell
    unsigned int     trade_qulity;
    float            trade_price;
};

void saveTradeRecord(const struct StockTradeItem &item)
{
    ofstream out_file(TRADE_RECORD_FILE, ios::app | ios::out);
    if(!out_file)
    {
        cerr<<"Open file error: "<<TRADE_RECORD_FILE<<endl;
        return;
    }
    out_file<<item.stock_code<<"    "
            <<item.trade_type<<"    "
            <<item.trade_qulity<<"    "
            <<item.trade_price<<"    "<<endl;
    out_file.close();
}

bool isValidStockCode(const char *stock_code)
{
    unsigned int len = strlen(stock_code);

    if(NULL == stock_code || STOCK_CODE_LEN != len)
        return false;

    for(unsigned int id = 0; id < len; ++id)
    {
        if('0' > stock_code[id] || '9' < stock_code[id])
            return false;
    }
    return true;
}

void usage(const char* app_name)
{
    cout<<"Usage:"<<endl;
    cout<<" "<<app_name<<"  stock_code(number with 6 digitals)"<<endl;
}

ssize_t netWrite(int sock_fd, const void *buf, size_t bytes)
{
    const char *cbuf = (const char *)buf;
    ssize_t count;
    size_t rem = bytes;
    while (rem > 0)
    {
        count = send(sock_fd, cbuf, rem, 0);
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
    return bytes;
}

ssize_t netRead(int sock_fd, void *buf, size_t bytes)
{
    char *cbuf = (char *)buf;
    ssize_t count;
    size_t rem = bytes;
    while (rem > 0)
    {
        count = recv(sock_fd, cbuf, rem, 0);
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
    return bytes;
}

int main(int argc, char **argv)
{
    int sockfd;
    struct sockaddr_in address;
    char msg[128] = {0};
    char stock_code[STOCK_CODE_LEN + 1] = {0};

    if( (2 == argc) && (isValidStockCode(argv[1])) )
    {
        snprintf(stock_code, STOCK_CODE_LEN+1,"%s", argv[1]);
    }
    else
    {
        usage(argv[0]);
        return -1;
    }

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = inet_addr(SERVER_IP);
    address.sin_port = htons(TCP_CONNECT_PORT);

    if(-1== connect(sockfd, (struct sockaddr *)&address, sizeof(address)))
    {
        cerr<<"connect error."<<endl;
        return -1;
    }

    netRead(sockfd, msg, strlen(GREET_WORD));
    cout<<"recv msg from the server: "<<msg<<endl;

    netWrite(sockfd, stock_code, STOCK_CODE_LEN);

    struct StockTradeItem item;
    if(0 == netRead(sockfd, &item, sizeof(item)))
    {
        cerr<< "read the stock record error"<<endl;
    }
    else
    {
        if(0 == item.trade_qulity)
        {
            cerr<<"there is no record in the trade record with stock code "<<item.stock_code<<endl;
        }
        else
        {
           cout<<"the trade record has found and save in the file "<<TRADE_RECORD_FILE<<endl;
           saveTradeRecord(item);
        }
    }
    close(sockfd);
    return 0;
}
