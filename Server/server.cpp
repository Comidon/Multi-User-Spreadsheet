/* Definitions of server class
 *
 * server class is used to store connections
 * and provide methods to process incoming messages
 *
 * Team : Bluefly
 *
 * Last Modified : April 20th, 2018
 */

#include "server.h"

/*
 * Constructor:
 *  mapping command names to an id, that's it :)
 */
server::server()
{
	cmd_look_up["register"] = 0;
	cmd_look_up["disconnect"] = 1;
	cmd_look_up["load"] = 2;
	cmd_look_up["ping"] = 3;
	cmd_look_up["ping_response"] = 4;
	cmd_look_up["edit"] = 5;
	cmd_look_up["focus"] = 6;
	cmd_look_up["unfocus"] = 7;
	cmd_look_up["undo"] = 8;
	cmd_look_up["revert"] = 9;
}

/*
 * Distructor:
 *  that's it :)
 */
server::~server()
{
}


/*
 * 
 */
void server::process_request(int socket, std::string input, clock_t& ping_response_clock)
{
	std::string::size_type pos;
	pos = input.find(' ', 0);

	std::string key = input.substr(0, pos);
	std::string content = input.substr(pos + 1);

	switch (cmd_look_up[key])
	{
	case 0:
		process_register(socket);
		break;

	case 1:
		process_disconnect(socket);
		break;

	case 2:
		process_load(socket, content);
		break;

	case 3:
		process_ping(socket);
		break;

	case 4:
		process_ping_response(socket, ping_response_clock);
		break;

	case 5:
		process_edit(socket, content);
		break;

	case 6:
		process_focus(socket, content);
		break;

	case 7:
		process_unfocus(socket);
		break;

	case 8:
		process_undo(socket);
		break;

	case 9:
		process_revert(socket, content);
		break;

	default:
		break;
	}
}

/*
* As long as the specified user (index 1 of the vector) is a registered user,
* allow them to connect, open the specified spreadsheet (index 2) and associate
* the user with the spreadsheet graph so they can do things to it.
*/
void server::process_register(int socket)
{
	send_string(socket, "connect_accepted ");
	for (int i = 0; i < ssnamelist.size(); i++)
     {
		if (i == ssnamelist.size() - 1)
		{
			send_string(socket, ssnamelist[i]);
		}
		else
			send_string(socket, ssnamelist[i] + "\n");
	}
	send_string(socket, "\3");
}


void server::process_disconnect(int socket)
{
	//(*ssn_socketset_map)[(*socket_ssn_map)[socket]].
	close(socket);
	std::cout << "client disconnected" << std::endl;
	pthread_exit(NULL); // Kill this thread
}

void server::process_load(int socket, std::string ss)
{
	// if the spreadsheetname vector contains the ss, load
	if(std::find(ssnamelist.begin(), ssnamelist.end(), ss) != ssnamelist.end())
	{
		// check if ssn_sso_map contains the ss (in case of server delete the obj)
		if(ssn_sso_map.find(ss) != ssn_sso_map.end())
		{
			socket_ssn_map.insert(std::pair<int,std::string>(socket,ss));
			ssn_socketset_map[ss].insert(socket);
			// contains the ss, load the ss object
			std::set<std::string> sheet = ssn_sso_map[ss]->get_sheet();
			std::set<std::string>::iterator it;
			send_string(socket, "full_state ");
			for (it = sheet.begin(); it != sheet.end(); ++it)
			{
				std::string s = *it;
    		send_string(socket, s);
			}
			send_string(socket, "\3");

		}
		else{
			//construct the sso. add ssn,sso to ssn_sso_map
			serverside_sheet* newsheet = new serverside_sheet(ss);

			// add to the ssn_sso_map map
			ssn_sso_map.insert(std::pair<std::string, serverside_sheet*>(ss,newsheet));
			socket_ssn_map.insert(std::pair<int,std::string>(socket,ss));
			std::set<int> sockets;
			sockets.insert(socket);
			ssn_socketset_map.insert(std::pair<std::string,std::set<int> >(ss,sockets));

			std::set<std::string> sheet = newsheet->get_sheet();
			std::set<std::string>::iterator it;
			// sent "full_state and all cellname:cell_content and \3 to client"
			send_string(socket, "full_state ");
			for (it = sheet.begin(); it != sheet.end(); ++it)
			{
				std::string s = *it;
    		send_string(socket, s);
			}
			send_string(socket, "\3");

		}

	}
	// otherwise, create
	else {
		// add name to the ss list
		ssnamelist.push_back(ss);
		serverside_sheet* newsheet = new serverside_sheet(ss);
		// add to the ssn_sso_map map
		ssn_sso_map.insert(std::pair<std::string,serverside_sheet*>(ss,newsheet));
		socket_ssn_map.insert(std::pair<int,std::string>(socket,ss));
		std::set<int> sockets;
		sockets.insert(socket);
		ssn_socketset_map.insert(std::pair<std::string,std::set<int> >(ss,sockets));
		std::ofstream outfile((ss + ".txt").c_str());
		outfile.close();
		// sent "full_state and all cellname:cell_content and \3 to client"
		std::set<std::string> sheet = newsheet->get_sheet();
		std::set<std::string>::iterator it;
		send_string(socket, "full_state ");
		for (it = sheet.begin(); it != sheet.end(); ++it)
		{
			std::string s = *it;
			send_string(socket, s);
		}
		send_string(socket, "\3");
	}
}

void server::process_ping(int socket)
{
	std::cout << "sent" << std::endl;
	send_string(socket, "ping_response \3");
}

void server::process_ping_response(int socket, clock_t& ping_response_clock)
{
	ping_response_clock = clock();
}

void server::process_edit(int socket, std::string content )
{
	std::string::size_type pos;
	pos = content.find(':', 0);

	std::string cell_name = content.substr(0, pos);
	std::string cell_content = content.substr(pos + 1);

	std::string update = ssn_sso_map[socket_ssn_map[socket]]->edit(cell_name, cell_content);

	for (std::set<int>::iterator it = ssn_socketset_map[socket_ssn_map[socket]].begin(); it != ssn_socketset_map[socket_ssn_map[socket]].end(); ++it)
	{
		send_string(*it, update);
	}
}

void server::process_focus(int socket, std::string cell )
{
	std::string sckt = num2string(socket);
	for (std::set<int>::iterator it = ssn_socketset_map[socket_ssn_map[socket]].begin(); it != ssn_socketset_map[socket_ssn_map[socket]].end(); ++it)
	{
		send_string(*it, "focus " + cell + ":" + sckt + "\3");
	}
}

void server::process_unfocus(int socket)
{
	std::string sckt = num2string(socket);
	for (std::set<int>::iterator it = ssn_socketset_map[socket_ssn_map[socket]].begin(); it != ssn_socketset_map[socket_ssn_map[socket]].end(); ++it)
	{
		send_string(*it, "unfocus " + sckt + "\3");
	}
}

void server::process_undo(int socket)
{
	std::string update = ssn_sso_map[socket_ssn_map[socket]]->undo();
	for (std::set<int>::iterator it = ssn_socketset_map[socket_ssn_map[socket]].begin(); it != ssn_socketset_map[socket_ssn_map[socket]].end(); ++it)
	{
		send_string(*it, update);
	}
}

void server::process_revert(int socket, std::string cell )
{
	std::string update = ssn_sso_map[socket_ssn_map[socket]]->revert(cell);
	for (std::set<int>::iterator it = ssn_socketset_map[socket_ssn_map[socket]].begin(); it != ssn_socketset_map[socket_ssn_map[socket]].end(); ++it)
	{
		send_string(*it, update);
	}
}

/*
 * Citation : http://www.cplusplus.com/articles/D9j2Nwbp/
 */
std::string server::num2string(int Number)
{
	std::ostringstream ss;
	ss << Number;
	return ss.str();
}

/*
 * Send a specified string over the specified socket with
 * a newline character appended to the end.
 */
void server::send_string(int socket, std::string temp)
{
	send(socket, temp.c_str(), temp.length(), 0);
}


