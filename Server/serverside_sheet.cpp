#include "serverside_sheet.h"

std::string serverside_sheet::edit(std::string cell, std::string content)
{
	for (int i = 0; i < cell.length(); i++)
		cell[i] = toupper(cell[i]);

	if (cells.find(cell) == cells.end())
	{
		std::pair<std::string, std::string> temp(cell, "");
		undo_stack.push(temp);
		std::vector<std::string> res_stack;
		reverse_map[cell] = std::vector<string>();
		reverse_map[cell].push_back("");
	}

	else
	{
		std::pair<std::string, std::string> temp(cell, cells[cell]);
		undo_stack.push(temp);
		reverse_map[cell].push_back(cells[cell]);
	}

	// If there were no circuluar dependencies, it is safe to add to the graph.
	cells[cell] = content;
	return true;
}

std::string serverside_sheet::get_sheet()
{
	std::string sheet = "";
	for (std::map<char, int>::iterator it = cells.begin(); it != cells.end(); ++it)
	{
		sheet += it->first + ":" + it->second + "\n";
	}
	sheet += (char)3;
	return sheet;
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

	std::string result = "change " + cell + ":" + content + (char)3;
	return result;
}

std::string serverside_sheet::reverse(std::string cellname)
{

}

int serverside_sheet::size()
{

}

void serverside_sheet::reset_undo()
{

}