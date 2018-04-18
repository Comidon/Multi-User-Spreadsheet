#ifndef SERVERSIDE_SHEET_H
#define SERVERSIDE_SHEET_H
// Last Modified : April 17th, 2018

#include <map>
#include <string>
#include <iostream>
#include <vector>
#include <boost/regex.hpp>
#include <cctype> // toupper
#include <stack>

class serverside_sheet
{
private:
	//std::string filename;
	std::map<std::string, std::string> cells;
	std::map<std::string, std::vector<std::string> > revert_map;
	std::stack<std::pair<std::string, std::string> > undo_stack;

public:
	serverside_sheet() {};
	serverside_sheet(std::string filename) {};
	~serverside_sheet() {};
	std::string edit(std::string cell, std::string content);
	std::string get_sheet();
	std::string undo();
	std::string revert(std::string cellname);
	int size();
	void reset_undo();

};
#endif
