/* Declarations of server class
 *
 * server class is used to store connections
 * and provide methods to process incoming messages
 *
 * Team : Bluefly
 *
 * Last Modified : April 20th, 2018
 */

#ifndef SERVER_H
#define SERVER_H

#include <string>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <cstring>
#include <vector>
#include <algorithm>
#include <map>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include <sstream>
#include <fstream>
#include <time.h>
#include "serverside_sheet.h"

class server
{
public:
	server();
	~server();
	// process general request
	void process_request(int socket, std::string input, clock_t& ping_response_clock);
	// send a string through a socket
	void send_string(int socket, std::string s);
	// process disconnect request
	void process_disconnect(int socket);
	// save all opened spreadsheets
	void save_all_open_sheets();

private:
	/* * -Data- * */
	// lock
	pthread_mutex_t mtx;
	// socket container
	std::vector<int> sockets;
	// serverside sheet name container
	std::vector<std::string> ssnamelist;
	// <socket_id, spreadsheet_name>
	std::map<int, std::string> socket_user_map;
	// <socket,spreadsheet_name>
	std::map<int, std::string> socket_ssn_map;
	// <spreadsheet_name, spreadsheet_object>
	std::map<std::string, serverside_sheet*> ssn_sso_map;
	// <spreadsheet_name, set of socket ids who is editing the sheet>
	std::map<std::string, std::set<int> > ssn_socketset_map;
	// <command_name, command_id>
	std::map<std::string, int> cmd_look_up;

	/* * -Methods- * */
	// process register request
	void process_register(int socket);
	// process load request
	void process_load(int socket, std::string s);
	// process ping request
	void process_ping(int socket);
	// process ping response request
	void process_ping_response(int socket, clock_t& ping_response_clock);
	// process edit request
	void process_edit(int socket, std:: string s);
	// process focus request
	void process_focus(int socket, std::string s);
	// process unfocus request
	void process_unfocus(int socket);
	// process undo request
	void process_undo(int socket);
	// process revert request
	void process_revert(int socket, std::string s);
	// parse an int to a string
	std::string num2string(int Number);
};

#endif
