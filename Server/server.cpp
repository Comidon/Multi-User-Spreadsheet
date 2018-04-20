/* Declarations of server class
 *
 * Team : Bluefly
 *
 * Last Modified : April 18th, 2018
 */
#include "server.h"


// helper function for socket connection purposes
void *get_in_addr(struct sockaddr *sa);

server * iserver = new server();
/*
* Main function creates and runs this spreadsheet server
*/
int main()
{
	std::cout << "Server on." << std::endl;
	// Default port 2112
	iserver->listen_for_connections("2112");
	return 0;
}

/*
* Static method for thread initialization. Required by the pthread_create function.
*/
void *thread_init(void * arg)
{
	// The parameter will be an integer specifying the connected client's socket.
	int * socket;
	socket = (int *)arg;

	// Begin listening to messages from the new client on the new thread.
	iserver->listen_to_client(*socket);
}

void *server_end(void * arg)
{
	std::string input;
	while (1)
	{
		std::cin >> input;
		if (input == "quit")
		{
			exit(0);
		}
	}
}

/*
* Constructor initializes all pointed to structures
* on the heap and opens the .bin file which holds state
* of registered users and spreadsheet information.
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

	ping_clock = clock();
	sockets = new std::vector<int>();
	ssnamelist = new std::vector<std::string>();
	socket_user_map = new std::map<int, std::string>();
	socket_ssn_map = new std::map<int, std::string>();
	ssn_sso_map = new std::map<std::string, serverside_sheet*>();
	ssn_socketset_map = new std::map<std::string, std::set<int> >();
}

/*
* Destructor cleans up heap memory.
*/
server::~server()
{
}

/*
* Kicks off the server listening and accepting connections.
* When a connection is made, the a new thread is made to listen
* for messages from the client over the socket.
*/
void server::listen_for_connections(std::string port)
{
	int sockfd, new_fd;                   // listen on sock_fd, new connection on new_fd
	struct addrinfo hints, *servinfo, *p; // hints used for connection flexibility
	struct sockaddr_storage client_addr;  // connector's address information
	socklen_t sin_size;
	int yes = 1;
	char s[INET6_ADDRSTRLEN];
	int rv;

	memset(&hints, 0, sizeof hints); // allocate memory for hints
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE; // use my IP

	if ((rv = getaddrinfo(NULL, port.c_str(), &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return;
	}

	// loop through all the results and bind to the first we can
	for (p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
			p->ai_protocol)) == -1) {
			perror("server: socket");
			continue;
		}

		if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
			sizeof(int)) == -1) {
			perror("setsockopt");
			exit(1);
		}

		// Bind the server socket to listen to this IP and port
		if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockfd);
			perror("server: bind");
			continue;
		}
		break;
	}

	// if something went wrong when trying to use the port, print error and return
	if (p == NULL) {
		fprintf(stderr, "server: failed to bind\n");
		return;
	}

	freeaddrinfo(servinfo); // all done with this structure

	if (listen(sockfd, SOMAXCONN) == -1) {
		perror("listen");
		exit(1);
	}

	printf("Waiting for connection from clients...\n");

	pthread_t exiter;
	//int end;
	//int endint = 1;
	//end = pthread_create(&end_thread, NULL, &server_end, &endint);
	if (pthread_create(&exiter, NULL, &server_end, NULL) != 0)
	{
		printf("Error creating new pthread\n");
		exit(EXIT_FAILURE);
	}

	while (true)
	{
		sin_size = sizeof client_addr;

		// Accept call is BLOCKING until a client connects
		new_fd = accept(sockfd, (struct sockaddr *)&client_addr, &sin_size);
		if (new_fd == -1) {
			perror("accept");
			continue;
		}

		inet_ntop(client_addr.ss_family, get_in_addr((struct sockaddr *)&client_addr), s, sizeof s);
		printf("server: got connection from %s\n", s);

		// create new thread to handle this connection on this socket.
		pthread_t new_thread;
		int ret;
		int * socket_ID = &new_fd;
		ret = pthread_create(&new_thread, NULL, &thread_init, socket_ID);
		if (ret != 0)
		{
			printf("Error creating new pthread\n");
			exit(EXIT_FAILURE);
		}
	}
	return;
}

/*
* Helper method to determine what protocol (ipv4 or ipv6) the client is using for communication
* (used with inet_ntop method)
*/
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

/*
* Continually listens and reads in messages from clients on the given socket.
* Once a new line character is detected, the request is processed and sent
* off to be interpreted in process_request. This method also detects when
* a client has disconnected, and saves the spreadsheet accordingly.
*/
void server::listen_to_client(int socket)
{
	clock_t ping_response_clock = clock();
	std::string temp = "";
	int received;             // How much data has come through on the socket.
	while (1)
	{
		if ((clock() - ping_clock) * 1.0 / CLOCKS_PER_SEC >= 10)
		{
			send_string(socket, "ping \3");
			ping_clock = clock();
		}

		if ((clock() - ping_clock) * 1.0 / CLOCKS_PER_SEC >= 60)
		{
			process_disconnect(socket);
		}

		char msg[1];
		received = recv(socket, msg, 1, 0); // Recieve and process one byte at a time
		if (received < 1) // Something went wrong (most likely client disconnect)
			break;
		// new line character found, process the message
		if (msg[0] == (char)3)
		{
			process_request(socket, temp, ping_response_clock);
			temp = "";
		}
		else
			temp += msg[0];
	}

	// Client has disconnected at this point!
	close(socket);
	std::cout << "client disconnected" << std::endl;
	pthread_exit(NULL); // Kill this thread
}



/*
* Takes in the message sent from the client,  figures out what
* the message is, and call the correct function associated with
* that message.
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
		std::cout << "on ping" << std::endl;
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
	for (int i = 0; i < (*ssnamelist).size(); i++)
     {
		if (i == (*ssnamelist).size() - 1)
		{
			send_string(socket, (*ssnamelist)[i]);
		}
		else
			send_string(socket, (*ssnamelist)[i] + "\n");
	}
	send_string(socket, "\3");
}


void server::process_disconnect(int socket )
{
	//(*ssn_socketset_map)[(*socket_ssn_map)[socket]].
	close(socket);
	std::cout << "client disconnected" << std::endl;
	pthread_exit(NULL); // Kill this thread
}

void server::process_load(int socket, std::string ss )
{
	// if the spreadsheetname vector contains the ss, load
	if(std::find((*ssnamelist).begin(), (*ssnamelist).end(), ss) != (*ssnamelist).end())
	{
		// check if ssn_sso_map contains the ss (in case of server delete the obj)
		if(ssn_sso_map->find(ss) != (*ssn_sso_map).end())
		{
			(*socket_ssn_map).insert(std::pair<int,std::string>(socket,ss));
			(*ssn_socketset_map)[ss].insert(socket);
			// contains the ss, load the ss object
			std::set<std::string> sheet = (*ssn_sso_map)[ss]->get_sheet();
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
			(*ssn_sso_map).insert(std::pair<std::string,serverside_sheet*>(ss,newsheet));
			(*socket_ssn_map).insert(std::pair<int,std::string>(socket,ss));
			std::set<int> sockets;
			sockets.insert(socket);
			(*ssn_socketset_map).insert(std::pair<std::string,std::set<int> >(ss,sockets));

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
		(*ssnamelist).push_back(ss);
		serverside_sheet* newsheet = new serverside_sheet(ss);
		// add to the ssn_sso_map map
		(*ssn_sso_map).insert(std::pair<std::string,serverside_sheet*>(ss,newsheet));
		(*socket_ssn_map).insert(std::pair<int,std::string>(socket,ss));
		std::set<int> sockets;
		sockets.insert(socket);
		(*ssn_socketset_map).insert(std::pair<std::string,std::set<int> >(ss,sockets));
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

	std::string update = (*ssn_sso_map)[(*socket_ssn_map)[socket]]->edit(cell_name, cell_content);

	for (std::set<int>::iterator it = (*ssn_socketset_map)[(*socket_ssn_map)[socket]].begin(); it != (*ssn_socketset_map)[(*socket_ssn_map)[socket]].end(); ++it)
	{
		send_string(*it, update);
	}
}

void server::process_focus(int socket, std::string cell )
{
	std::string sckt = num2string(socket);
	for (std::set<int>::iterator it = (*ssn_socketset_map)[(*socket_ssn_map)[socket]].begin(); it != (*ssn_socketset_map)[(*socket_ssn_map)[socket]].end(); ++it)
	{
		send_string(*it, "focus " + cell + ":" + sckt + "\3");
	}
}

void server::process_unfocus(int socket)
{
	std::string sckt = num2string(socket);
	for (std::set<int>::iterator it = (*ssn_socketset_map)[(*socket_ssn_map)[socket]].begin(); it != (*ssn_socketset_map)[(*socket_ssn_map)[socket]].end(); ++it)
	{
		send_string(*it, "unfocus " + sckt + "\3");
	}
}

void server::process_undo(int socket)
{
	std::string update = (*ssn_sso_map)[(*socket_ssn_map)[socket]]->undo();
	for (std::set<int>::iterator it = (*ssn_socketset_map)[(*socket_ssn_map)[socket]].begin(); it != (*ssn_socketset_map)[(*socket_ssn_map)[socket]].end(); ++it)
	{
		send_string(*it, update);
	}
}

void server::process_revert(int socket, std::string cell )
{
	std::string update = (*ssn_sso_map)[(*socket_ssn_map)[socket]]->revert(cell);
	for (std::set<int>::iterator it = (*ssn_socketset_map)[(*socket_ssn_map)[socket]].begin(); it != (*ssn_socketset_map)[(*socket_ssn_map)[socket]].end(); ++it)
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
