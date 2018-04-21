/* Declarations of serverside_sheet class
 *
 * serverside_sheet class is used to store spreadsheet cells
 * and proceed spreadsheet-related operations on the server side
 * Each serverside_sheet object represents one spreadsheet.
 *
 * Team : Bluefly
 *
 * Last Modified : April 20th, 2018
 */

#ifndef SERVERSIDE_SHEET_H
#define SERVERSIDE_SHEET_H

#include <pthread.h>
#include <string>
#include <string.h>
#include <fstream>
#include <iostream>
#include <vector>
#include <map>
#include <cctype>
#include <stack>
#include <set>

class serverside_sheet
{
private:
	//std::string filename;
	// lock
	pthread_mutex_t mtx;
	// cell container
	std::map<std::string, std::string> cells;
	// map of revert stacks
	std::map<std::string, std::vector<std::string> > revert_map;
	// undo stack
	std::stack<std::pair<std::string, std::string> > undo_stack;

public:
	// default constructor (do nothing)
	serverside_sheet() {};
	// constructor that construct an object according to a file
	serverside_sheet(std::string filename);
	// destructor
	~serverside_sheet() {};
	// update a cell content
	std::string edit(std::string cell, std::string content);
	// one step undo
	std::string undo();
	// revert a cell's content
	std::string revert(std::string cellname);
	// return the cells in this sheet
	std::set<std::string> get_sheet();
};

#endif
