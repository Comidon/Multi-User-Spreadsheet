/* Declarations of server class
 *
 * Team : Bluefly
 *
 * Last Modified : April 10th, 2018
 */

#include "server.h"

// helper function for socket connection purposes
void *get_in_addr(struct sockaddr *sa);

// private helper function for ease of use of strings in C
std::string int_to_string(int a);
server * iserver = new server();
/*
* Main function creates and runs this spreadsheet server
*/
int main()
{
	// Default port 2112
	int port = 2112;
	std::cout << "Server on work." << std::endl;
	iserver->listen_for_connections(int_to_string(port));
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

	std::cout << "New thread created for socket " + int_to_string((*socket)) << std::endl;

	// Begin listening to messages from the new client on the new thread.
	iserver->listen_to_client(*socket);
}

void *server_end(void * arg)
{
	std::string input;
	while (1)
	{
		std::cin >> input;
		if (input == "kill")
		{
			exit(0);
		}
	}
}

/*
* Helper method to convert an integer to a string.
*/
std::string int_to_string(int a)
{
	std::string result;
	std::stringstream out;
	out << a;
	return out.str();
}

/*
* Constructor initializes all pointed to structures
* on the heap and opens the .bin file which holds state
* of registered users and spreadsheet information.
*/
server::server()
{
	sockets = new std::vector<int>();
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

	if (listen(sockfd, PENDINGCONNECTIONS) == -1) {
		perror("listen");
		exit(1);
	}

	printf("Waiting for connection from clients...\n");

	pthread_t end_thread;
	int end;
	int endint = 1;
	end = pthread_create(&end_thread, NULL, &server_end, &endint);
	if (end != 0)
	{
		printf("Error creating new pthread\n");
		exit(EXIT_FAILURE);
	}


	// Main listening thread! (for new connections)
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
	bool registered = false;  // Bool used for authentication flag for this sockeerver_end’ was not t.
	std::string temp = "";
	int received;             // How much data has come through on the socket.
	while (1)
	{
		char msg[1];
		received = recv(socket, msg, 1, 0); // Recieve and process one byte at a time
		if (received < 1) // Something went wrong (most likely client disconnect)
			break;

		// new line character found, process the message
		if (msg[0] == (char)3)
		{
			process_request(socket, temp, &registered);
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
void server::process_request(int socket, std::string input, bool * registered)
{
	std::vector<std::string> v = parse_command(input);
	if (v[0] == "register")
		process_register(socket, registered);
	else // Junk recieved... send error 2
		send_message(socket, "error 2 " + input);
}

/*
* As long as the specified user (index 1 of the vector) is a registered user,
* allow them to connect, open the specified spreadsheet (index 2) and associate
* the user with the spreadsheet graph so they can do things to it.
*/
void server::process_register(int socket, bool * registered)
{

	// This flag is used in other functions to make sure that 
	// a socket trying to make changes has been approved to 
	// make changes.
	*registered = true;

	send_message(socket, "connected");
}

/*
* Send a specified string over the specified socket with
* a newline character appended to the end.
*/
void server::send_message(int socket, std::string temp)
{
	// Allocate enough space in the char array for the string + \n
	char * cstr = new char[temp.length() + 1];
	std::strcpy(cstr, temp.c_str());
	cstr[temp.length()] = '\n';

	send(socket, cstr, (temp.length() + 1), 0);
}

/*
* Splits a string depending on its spaces. If there are no spaces,
* returns the same single string in the vector. If there is one space,
* returns two strings separated by that space in the vector. If there
* are two spaces or more, returns the string split up into three strings
* divided by the first and second spaces. All remaining spaces are ignored.
*/
std::vector<std::string> server::parse_command(std::string input)
{
	int first_space = 0;
	int second_space = 0;
	std::vector<std::string> result;

	// find positions of the first 2 empty spaces to parse string
	for (int i = 0; i < input.length(); i++)
	{
		if (input[i] == ' ')
			if (first_space == 0)
				first_space = i;
			else
			{
				second_space = i;
				break;
			}
	}

	// If there were no spaces, simply put the string back
	// in the vector as one.
	if (!first_space && !second_space)
		result.push_back(input);

	// If there was only one space, put the two space-separated
	// strings into the vector
	else if (!second_space)
	{
		std::string first_one = input.substr(0, first_space);
		std::string second_one = input.substr(first_space + 1, input.length() - first_space - 1);
		result.push_back(first_one);
		result.push_back(second_one);
	}
	// If there were two spaces, put the three first-and-second-space
	// separated strings into the vector.
	else if (first_space && second_space)
	{
		result.push_back(input.substr(0, first_space));
		// cell name
		result.push_back(input.substr(first_space + 1, second_space - first_space - 1));
		// contents
		result.push_back(input.substr(second_space + 1, input.length() - second_space - 1));
	}
	return result;
}