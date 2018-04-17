#ifndef SERVERSIDE_SHEET_H
#define SERVERSIDE_SHEET_H

#include <map>
#include <string>
#include <iostream>
#include <vector>
#include <boost/regex.hpp>
#include <cctype> // toupper
#include <stack>

class serverside_sheet
{
public:
	spreadsheet_graph() {};
	//spreadsheet_graph(std::string filename, bool exist) {};
	~spreadsheet_graph() {};
	std::string edit(std::string cell, std::string content);
	std::string get_sheet();
	std::string undo();
	std::string reverse(std::string cellname);
	int size();
	void reset_undo();

private:
	//std::string filename;
	std::map<std::string, std::string> cells;
	std::map<std::string, std::vector<std::string> > reverse_map;
	std::stack<std::pair<std::string, std::string> > undo_stack;
};
#endif
