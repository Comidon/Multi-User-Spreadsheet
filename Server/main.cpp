/* main.cpp
 *
 * contains tcp netcode and main method to run the server
 *
 * Team : Bluefly
 *
 * Last Modified : April 22th, 2018
 */

#include "server.h"

static void *wait_quit(void * arg);
static void *kick_to_listen(void * arg);
static void listen_for_connections(std::string port);

static server * working_server;

int main()
{
	// construct the server
	working_server = new server();
	// start the server on port 2112
	std::cout << "Server on." << std::endl;
	listen_for_connections("2112");
	return 0;
}

/*
 * Netcode:
 *  listen for new incoming connections
 *  when new connection arrives, start a new thread to listen from the client
 */
static void listen_for_connections(std::string port)
{
	int sockfd, new_fd;
	struct addrinfo hints, *servinfo, *p;
	struct sockaddr_storage client_addr;
	socklen_t sin_size;
	int yes = 1;
	int rv;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE; // use my IP

	if ((rv = getaddrinfo(NULL, port.c_str(), &hints, &servinfo)) != 0)
	{
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return;
	}

	// loop through all the results and bind to the first we can
	for (p = servinfo; p != NULL; p = p->ai_next)
	{
		if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)
		{
			continue;
		}

		if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1)
		{
			perror("setsockopt");
			exit(EXIT_FAILURE);
		}

		// Bind the server socket to listen to this IP and port
		if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1)
		{
			close(sockfd);
			continue;
		}
		break;
	}

	// Exit if something goes wrong when binding
	if (p == NULL)
	{
		std::cout << "Failed to bind." << std::endl;
		exit(EXIT_FAILURE);
	}

	freeaddrinfo(servinfo);

	if (listen(sockfd, SOMAXCONN) == -1)
	{
		perror("listen");
		exit(EXIT_FAILURE);
	}

	std::cout << "Start listening for connections." << std::endl;


	// Create a thread to listen for shut down command
	pthread_t exiter;
	if (pthread_create(&exiter, NULL, &wait_quit, NULL) != 0)
	{
		std::cout << "Failed creating new pthread." << std::endl;
		exit(EXIT_FAILURE);
	}

	while (true)
	{
		sin_size = sizeof client_addr;

		// Accept call is BLOCKING until a client connects
		new_fd = accept(sockfd, (struct sockaddr *)&client_addr, &sin_size);
		if (new_fd == -1)
		{
			continue;
		}
		
		// Create a thread to listen to a client
		pthread_t new_thread;
		if (pthread_create(&new_thread, NULL, &kick_to_listen, &new_fd))
		{
			std::cout << "Failed creating new pthread." << std::endl;
			exit(EXIT_FAILURE);
		}
		std::cout << "client accepted" << std::endl;
	}
	return;
}

/*
 * Continually read message from a socket.
 * Process request on receiving a terminator (\3).
 * Ping the client every 10 seconds.
 * If the client doesn't reponse in 60 seconds, disconnect it.
 */
void listen_to_client(int socket)
{
	// timers used to ping
	time_t ping_clock, ping_response_clock;
	time(&ping_clock);
	time(&ping_response_clock);

	// char pointer used to store single byte of message
	char msg[1];

	std::string request = "";

	while (recv(socket, msg, 1, 0) >= 1)
	{
		if (time(NULL) - ping_clock >= 10)
		{
			// Ping the client
			working_server->send_string(socket, "ping \3");
			ping_clock = time(NULL);
		}

		if (time(NULL) - ping_response_clock >= 60)
		{
			// Disconnect the client
			working_server->send_string(socket, "disconnect \3");
			working_server->process_disconnect(socket);
		}

		// If a terminator is received, process the message
		if (msg[0] == (char)3)
		{
			working_server->process_request(socket, request, ping_response_clock);
			request = "";
		}
		// else, append message string
		else
			request += msg[0];
	}

	// client was somehow disconnected unexpectedly
	working_server->send_string(socket, "disconnect \3");
	close(socket);
	std::cout << "client disconnected" << std::endl;
	pthread_exit(NULL);
}


/*
 * Specify a thread to listen to cin.
 * When "quit" is detected from cin,
 * save all sheets, then shut down the server
 */
static void *wait_quit(void * arg)
{
	std::string input;
	while (1)
	{
		std::cin >> input;
		if (input == "quit")
		{
			working_server->save_all_open_sheets();
			exit(0);
		}
	}
}

/*
 * Specify a thread to listen to a socket
 */
static void *kick_to_listen(void* socket_ptr)
{
	listen_to_client(*((int*)socket_ptr));
}