#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <iostream>
#include <fstream>
using namespace std;

#define STOCK_CODE_LEN          6
#define UDP_CONNECT_PORT       7777
#define SERVER_IP              "127.0.0.1"
#define TRADE_RECORD_FILE      "trade_record_udp.txt"

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

int main(int argc, char* argv[])
{
    struct sockaddr_in servaddr;
    int sockfd, res;
    char stock_code[STOCK_CODE_LEN + 1] = {0};

    if( (2 == argc) && (isValidStockCode(argv[1])) )
    {
        snprintf(stock_code, STOCK_CODE_LEN+1,"%s", argv[1]);
    }
    else
    {
        usage(argv[0]);
        return 0;
    }

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    inet_pton(AF_INET, SERVER_IP, &servaddr.sin_addr);
    servaddr.sin_port = htons(UDP_CONNECT_PORT);


    res = sendto(sockfd, stock_code, STOCK_CODE_LEN, 0, (struct sockaddr*)&servaddr, sizeof(servaddr));
    if(-1 == res)
    {
        cerr<< "sendto error"<<endl;
    }

    struct StockTradeItem item;
    res = recvfrom(sockfd, &item, sizeof(item), 0, NULL, 0);
    if(-1 == res)
    {
        cerr<< "recvfrom error"<<endl;
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

