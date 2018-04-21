#ifndef SERVERSIDE_SHEET_H
#define SERVERSIDE_SHEET_H
// Last Modified : April 18th, 2018

#include <pthread.h>
#include <map>
#include <string>
#include <string.h>
#include <fstream>
#include <iostream>
#include <vector>
#include <cctype>
#include <stack>
#include <set>

class serverside_sheet
{
private:
	//std::string filename;
	pthread_mutex_t mtx;
	std::map<std::string, std::string> cells;
	std::map<std::string, std::vector<std::string> > revert_map;
	std::stack<std::pair<std::string, std::string> > undo_stack;

public:
	serverside_sheet() {};
	serverside_sheet(std::string filename);
	~serverside_sheet() {};
	std::string edit(std::string cell, std::string content);
	std::set<std::string> get_sheet();
	std::string undo();
	std::string revert(std::string cellname);
};
#endif
