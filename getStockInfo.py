#fdsfadsfad
from  urllib.request import urlopen
from bs4 import BeautifulSoup
import requests
import datetime,time,os
import csv

stock_datas = []

def insertStockInfo(stock_info):
    find_flag = False
    for idx, _ in enumerate(stock_datas):
        if stock_datas[idx][0] == stock_info[0]:
            find_flag = True
            stock_datas[idx][2] = round(stock_info[2] + stock_datas[idx][2], 2)
            stock_datas[idx][3] = round(stock_info[3] + stock_datas[idx][3], 2)
            stock_datas[idx][4] = round(stock_info[4] + stock_datas[idx][4], 2)
            stock_datas[idx][5] += 1
    if(False == find_flag):
        tmp = stock_info
        tmp.extend([1])
        stock_datas.append(tmp)

url_prefix = "http://data.eastmoney.com/hsgt/top10/"
head = {
    "User-Agent":"Mozilla/5.0 (Macintosh; Intel Mac OS X 10_9_5)AppleWebKit 537.36 (KHTML, like Gecko) Chrome",
    "Accept":"text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8",
    "Accept-Language": "zh-CN,zh;q=0.8"
    }
begin_date = datetime.datetime(2017,1,1)
end_date = datetime.datetime.today()
while begin_date <= end_date:
    begin_date += datetime.timedelta(days=1)
    if begin_date.isoweekday() > 5:
        print("the day is offwork")
        continue
    date_str = begin_date.strftime("%Y-%m-%d")
    url_all = url_prefix + date_str + ".html"
    print(url_all)
    session = requests.Session()
    req = session.get(url_all, headers=head)
    bsObj = BeautifulSoup(req.text, "html.parser")
    # talbes_all = bsObj.find_all("table",{"cellpadding":"0", "cellspacing":"0", "class":"tab1"})
    # for tab_tmp in talbes_all:
    #     if -1 != tab_tmp.get_text().find("暂无数据"):
    #         print("today is offwork")
    #         continue
    talbes_all = bsObj.find_all('tr',{"onmouseover": "this.className='over'"})
    for tab_tmp in talbes_all:
        # print(tab_tmp.get_text())float(items[6].get_text()[0:-1])
        items = tab_tmp.find_all('td')
        stock_num = items[1].get_text()
        if 6 != len(stock_num):
            continue
        stock_name = items[2].get_text()

        # for python   8570.63 * 10000 = 85706299.99999999

        if -1 != items[6].get_text().find('亿'):
            stock_total = round(float(items[6].get_text()[0:-1]) * 10000, 2)
        elif -1 != items[6].get_text().find('万'):
            stock_total = float(items[6].get_text()[0:-1])
        else:
            stock_total = 0.0

        if -1 != items[7].get_text().find('亿'):
            stock_in = round(float(items[7].get_text()[0:-1]) * 10000, 2)
        elif -1 != items[7].get_text().find('万'):
            stock_in = float(items[7].get_text()[0:-1])
        else:
            stock_in = 0.0

        if  -1 != items[8].get_text().find('亿'):
            stock_out = round(float(items[8].get_text()[0:-1]) * 10000, 2)
        elif -1 != items[8].get_text().find('万'):
            stock_out = float(items[8].get_text()[0:-1])
        else:
            stock_out = 0.0

        print(stock_num, stock_name,stock_total,stock_in,stock_out)
        insertStockInfo([stock_num, stock_name,stock_total,stock_in,stock_out])

stock_datas.sort(key=lambda x:x[2])
with open("stock_data.csv","w+") as stock_file:
    stock_file = csv.writer(stock_file)
    stock_file.writerow(("number", "name", "net buying", "buy", "sell", "list day"))
    for row in stock_datas:
        stock_file.writerow(row)



