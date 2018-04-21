/* Definations of serverside_sheet class
 *
 * serverside_sheet class is used to store spreadsheet cells
 * and proceed spreadsheet-related operations on the server side
 * Each serverside_sheet object represents one spreadsheet.
 *
 * Team : Bluefly
 *
 * Last Modified : April 20th, 2018
 */

#include "serverside_sheet.h"

/*
 * Give a filename as input,
 *  construct an object that takes 
 *  all cells stored in this file.
 */
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

/*
 * Given a cellname and a content string,
 *  update the cell content.
 */
std::string serverside_sheet::edit(std::string cell, std::string content)
{
	// lock
	pthread_mutex_lock(&mtx);

	/* * * * * * * * * * * * * * * * * * * * * * */
	for (int i = 0; i < cell.length(); i++)
		cell[i] = toupper(cell[i]);

	// If the cell doesn't exist in the sheet,
	//   add it to the cells container.
	if (cells.find(cell) == cells.end())
	{
		// Create a revert stack slot for this cell and push an empty content to it
		std::vector<std::string> res_stack;
		revert_map[cell] = std::vector<std::string>();
		revert_map[cell].push_back("");

		// Push a pair of cellname and empty content to the undo stack
		undo_stack.push(std::make_pair(cell, ""));
	}

	// If the cell exists in the sheet,
	//   update its content.
	else
	{
		// Push the old cell content to its revert stack
		revert_map[cell].push_back(cells[cell]);
		// Push the cellname and its old cell content to the undo stack
		undo_stack.push(std::make_pair(cell, cells[cell]));
	}

	// Update the cell content.
	cells[cell] = content;
	/* * * * * * * * * * * * * * * * * * * * * * */

	// unlock
	pthread_mutex_unlock(&mtx);

	// return the message for informing clients
	return "change " + cell + ":" + content + "\3";
}


/*
 * Returns all cells stored in this object as a set of strings.
 */
std::string serverside_sheet::undo()
{
	// lock
	pthread_mutex_lock(&mtx);

	/* * * * * * * * * * * * * * * * * * * * * * */
	// If undo stack is empty, return empty string (clients do nothing)
	if (undo_stack.size() == 0)
		return "";

	// Get the last edit
	std::string cell = undo_stack.top().first;
	std::string content = undo_stack.top().second;
	undo_stack.pop();

	// Push the current value of the cell to its revert stack
	revert_map[cell].push_back(cells[cell]);

	// Update cell content
	cells[cell] = content;
	/* * * * * * * * * * * * * * * * * * * * * * */

	// unlock
	pthread_mutex_unlock(&mtx);

	// return the message for informing clients
	return "change " + cell + ":" + content + "\3";
}

/*
 * Returns all cells stored in this object as a set of strings.
 */
std::string serverside_sheet::revert(std::string cellname)
{
	// lock
	pthread_mutex_lock(&mtx);

	/* * * * * * * * * * * * * * * * * * * * * * */
	// If the revert stack is empty, return empty string (clients do nothing)
	if (revert_map[cellname].size() == 0)
		return "";

	// Get the cell's old value
	std::string content = revert_map[cellname].back();
	revert_map[cellname].pop_back();

	// Since revert is considered as edit, push the current value to undo stack before revert
	std::pair<std::string, std::string> temp(cellname, cells[cellname]);
	undo_stack.push(temp);

	// Update cell's content
	cells[cellname] = content;
	/* * * * * * * * * * * * * * * * * * * * * * */

	// unlock
	pthread_mutex_unlock(&mtx);

	// return the message for informing clients
	return "change " + cellname + ":" + content + "\3";
}

/*
 * Returns all cells stored in this object as a set of strings.
 */
std::set<std::string> serverside_sheet::get_sheet()
{
	// lock
	pthread_mutex_lock(&mtx);

	/* * * * * * * * * * * * * * * * * * * * * * */
	std::set<std::string> res;
	// Go through the cells container and insert cells to a set
	for (std::map<std::string, std::string>::iterator it = cells.begin(); it != cells.end(); ++it)
	{
		// Cells are represented by <cellname>:<cellcontent>, seperated by newline
		std::string cell = it->first + ":" + it->second + "\n";
		res.insert(cell);
	}
	/* * * * * * * * * * * * * * * * * * * * * * */

	// unlock
	pthread_mutex_unlock(&mtx);
	return res;
}