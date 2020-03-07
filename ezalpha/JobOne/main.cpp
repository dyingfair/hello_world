#include <iostream>
#include <sstream>
#include <fstream>
#include <list>
#include <map>
#include <algorithm>
#include <math.h>
using namespace std;

typedef struct {
	string stock_code;
	unsigned int quality;
	float price;
	bool buy;
}TradeItem;

typedef struct {
	list < TradeItem> list_trade_items;
	float realized_profit;
}TraderProfit;

void PrintTraderAndProfit(map<string, TraderProfit >::value_type map_item)
{
//	cout << "the remain record should be stored!" << endl;
//	for (list< TradeItem>::iterator iter_item = map_item.second.list_trade_items.begin(); iter_item != map_item.second.list_trade_items.end(); ++iter_item)
//	{
//		cout << map_item.first << " " << iter_item->stock_code << " " << iter_item->quality << " " << iter_item->price << " " << iter_item->buy << endl;
//	}
	cout<<map_item.first<<"	"<<map_item.second.realized_profit <<endl;
}
void CalculateProfit(map<string, TraderProfit > & trader_all_datas, const string & trade_record)
{
	unsigned int order;
	string trader;
	string stock_code;
	unsigned int quanlity;
	float price;
	string trade_type;
	float trade_fee;
	string trade_date, trade_time;
	

	istringstream strInfo(trade_record);
	strInfo >> order >> trader >> stock_code >> quanlity >> price >> trade_type >> trade_fee >> trade_date >> trade_time;
	
	map<string, TraderProfit >::iterator iter_trader = trader_all_datas.find(trader);

	TradeItem trade_item;
	if (iter_trader != trader_all_datas.end())
	{
		list< TradeItem>::iterator iter_item = iter_trader->second.list_trade_items.begin();
		while (iter_item != iter_trader->second.list_trade_items.end())
		{
			if (stock_code == iter_item->stock_code)
			{
				if (!trade_type.compare("Buy")) //buy trade
				{
					//find the existing sell trade
					if (!iter_item->buy)		
					{
						if (quanlity > iter_item->quality)
						{
							quanlity -= iter_item->quality;
							iter_trader->second.realized_profit -= trade_fee;
							iter_trader->second.realized_profit += iter_item->quality * (iter_item->price - price);
							iter_item = iter_trader->second.list_trade_items.erase(iter_item);
							trade_fee = 0;
						}
						else 
						{
							iter_trader->second.realized_profit -= trade_fee;
							iter_trader->second.realized_profit += quanlity * (iter_item->price - price);
							if (quanlity == iter_item->quality)
							{
								iter_trader->second.list_trade_items.erase(iter_item);
							}
							else
							{
								iter_item->quality -= quanlity;
							}
							quanlity = 0;
							break;
						}
					}
					else
					{
						iter_item++;
					}
				} 
				else // sell trade 
				{
					//find the existing buy trade
					if (iter_item->buy)
					{
						if (quanlity > iter_item->quality)  // sell quality > buy quality
						{
							quanlity -= iter_item->quality;
							iter_trader->second.realized_profit -= trade_fee;
							iter_trader->second.realized_profit += iter_item->quality * (price - iter_item->price);
							iter_item = iter_trader->second.list_trade_items.erase(iter_item);
							trade_fee = 0;
						}
						else // sell quality <= buy quality record
						{
							iter_trader->second.realized_profit -= trade_fee;
							iter_trader->second.realized_profit += quanlity * (price - iter_item->price);
							if (quanlity == iter_item->quality)
							{
								iter_trader->second.list_trade_items.erase(iter_item);
							}
							else
							{
								iter_item->quality -= quanlity;
							}
							quanlity = 0;
							break;
						}
					}
					else
					{
						iter_item++;
					}
				}
			}
			else
			{
				iter_item++;
			}
		}
		// Both the remain stock quanlity and the new stock quanlity should be stored
		if (0 != quanlity)
		{
			bool merge_flag = false;
			list< TradeItem>::reverse_iterator iter_item = iter_trader->second.list_trade_items.rbegin();
			while (iter_item != iter_trader->second.list_trade_items.rend())
			{
				// merge the trade with the same stock code, the same price, the same trade type from the end
				const bool buyType = trade_type.compare("Buy") ? false : true;
				if (   (iter_item->stock_code == stock_code)
					&& (buyType == iter_item->buy) )
				{
					if (fabs(iter_item->price - price) < 0.001)
					{
						iter_item->quality += quanlity;
						trader_all_datas[trader].realized_profit += -trade_fee;
						merge_flag = true;
					}
					break;
				}
				++iter_item;
			}
			if(!merge_flag)
			{
				trade_item.buy		  = trade_type.compare("Buy") ? false : true;
				trade_item.price	  = price;
				trade_item.quality    = quanlity;
				trade_item.stock_code = stock_code;
				trader_all_datas[trader].list_trade_items.push_back(trade_item);
				trader_all_datas[trader].realized_profit += -trade_fee;
			}
		}
	}
	else // it is a new trader
	{
		trade_item.buy = trade_type.compare("Buy") ? false : true;
		trade_item.price = price;
		trade_item.quality = quanlity;
		trade_item.stock_code = stock_code;
		trader_all_datas[trader].list_trade_items.push_back(trade_item);
		trader_all_datas[trader].realized_profit = -trade_fee;
	}
}


int test(const string &file_infor)
{
	string trade_record;
	ifstream in_file(file_infor.c_str());
	map<string, TraderProfit > trader_all_datas;  // trader : profit
	
	if (!in_file)
	{
		cout << "input file is invalid" << endl;
		return -1;
	}
	while (!in_file.eof())
	{ 
		getline(in_file, trade_record);
		CalculateProfit(trader_all_datas, trade_record);
	}
	in_file.close();

	cout << "Ouptut the trader information and profit in the file " << file_infor << endl;
	for_each(trader_all_datas.begin(), trader_all_datas.end(), PrintTraderAndProfit);
	cout << endl;

	return 0;
}

int main(void)
{
	test("stock_record1.txt");
	test("stock_record2.txt");
	test("stock_record3.txt");
	test("stock_record4.txt");
	test("stock_record5.txt");
	test("stock_record6.txt");
	system("pause");
	return 0;
}
