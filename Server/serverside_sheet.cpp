#include "serverside_sheet.h"
// Last Modified : April 18th, 2018

serverside_sheet::serverside_sheet(std::string s)
{
	// append .txt and convert ss string to char*
	//s.append(".txt");
	//const char *ssfile = ss.c_str();
	// requires that the ss file to be under the current directory
	//const char* const ss = s.c_str();
	std::ifstream inFile((s + ".txt").c_str());

	while (true)
	{
		std::string line;
		inFile >> line;
		if (inFile.fail())
			break;

		std::string::size_type pos;
		pos = line.find(':', 0);

		std::string cell_name = line.substr(0, pos);
		std::string cell_content = line.substr(pos + 1);

		this->cells.insert(std::pair<std::string,std::string>(cell_name,cell_content));

	}
	inFile.close();

}

std::string serverside_sheet::edit(std::string cell, std::string content)
{
	for (int i = 0; i < cell.length(); i++)
		cell[i] = toupper(cell[i]);

	if (cells.find(cell) == cells.end())
	{
		std::pair<std::string, std::string> temp(cell, "");
		undo_stack.push(temp);
		std::vector<std::string> res_stack;
		//
		revert_map[cell] = std::vector<std::string>();
		revert_map[cell].push_back("");
	}

	else
	{
		std::pair<std::string, std::string> temp(cell, cells[cell]);
		undo_stack.push(temp);
		revert_map[cell].push_back(cells[cell]);
	}

	// If there were no circuluar dependencies, it is safe to add to the graph.
	cells[cell] = content;
	std::string result = "change " + cell + ":" + content + "\3";
	return result;
}

std::set<std::string> serverside_sheet::get_sheet()
{
	std::set<std::string> res;
	for (std::map<std::string, std::string>::iterator it = cells.begin(); it != cells.end(); ++it)
	{
		std::string cell = it->first + ":" + it->second + "\n";
		res.insert(cell);
	}
	return res;
}

std::string serverside_sheet::undo()
{
	if (undo_stack.size() == 0)
		return "";

	// The information about the last changed cell is on top of the
	// stack. The stack consists of state pairs<key, value>.
	std::string cell = undo_stack.top().first;
	std::string content = undo_stack.top().second;
	undo_stack.pop();

	// Update the graph.
	cells[cell] = content;

	std::string result = "change " + cell + ":" + content + "\3";
	return result;
}

std::string serverside_sheet::revert(std::string cellname)
{
	if (revert_map[cellname].size() == 0)
		return "";
	std::string content = revert_map[cellname].back();
	revert_map[cellname].pop_back();

	std::pair<std::string, std::string> temp(cellname, cells[cellname]);
	undo_stack.push(temp);

	cells[cellname] = content;

	std::string result = "change " + cellname + ":" + content + "\3";
	return result;
}
