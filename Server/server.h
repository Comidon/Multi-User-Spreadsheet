/* Declarations of server class
 *
 * Team : Bluefly
 *
 // Last Modified : April 18th, 2018

 */

#ifndef SERVER_H
#define SERVER_H
#include "serverside_sheet.h"
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


class server
{
public:
	server();
	void listen_for_connections(std::string port);
	void listen_to_client(int socket);
	void send_string(int socket, std::string s);
	~server();

private:
	clock_t ping_clock;
	std::vector<int> *sockets;
	std::vector<std::string> *ssnamelist;
	std::map<int, std::string> *socket_user_map;
    std::map<std::string, int> cmd_look_up;
	// <socket,spreadsheet_name>
	std::map<int, std::string> *socket_ssn_map;
	// <spreadsheet_name,spreadsheet_object>
	std::map<std::string, serverside_sheet*> *ssn_sso_map;
	std::map<std::string, std::set<int> > *ssn_socketset_map;

	void process_request(int socket, std::string input, clock_t& ping_response_clock);
	// show all spreadsheet on the server. use std::vector to store all
	// string names of ss
	void process_register(int socket);
	void process_disconnect(int socket);
	std::string num2string(int Number);
	// user chose one ss and load. 1.if contains 2.true: load; false: create
	void process_load(int socket, std::string s);
	void process_ping(int socket);
	void process_ping_response(int socket, clock_t& ping_response_clock);
	void process_edit(int socket, std:: string s);
	void process_focus(int socket, std::string s);
	void process_unfocus(int socket);
	void process_undo(int socket);
	void process_revert(int socket, std::string s);
};

#endif
