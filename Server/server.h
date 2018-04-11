/* Declarations of server class
 *
 * Team : Bluefly
 *
 * Last Modified : April 10th, 2018
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
#include <map>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include <sstream>
#include <fstream>

class server
{
public:
	server();
	void listen_for_connections(std::string port);
	void listen_to_client(int socket);
	void send_message(int socket, std::string s);
	~server();

private:
	void open();
	void process_request(int socket, std::string input, bool * registered);
	void process_register(int socket, bool * registered);
	std::vector<std::string> parse_command(std::string input);

	std::vector<int> *sockets;
};

#endif