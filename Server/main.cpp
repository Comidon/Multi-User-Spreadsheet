#include "server.h"
static void *wait_quit(void * arg);
static void *kick_to_listen(void * arg);
static void listen_for_connections(std::string port);

server * iserver = new server();

int main()
{
	// start the server on port 2112
	std::cout << "Server on." << std::endl;
	listen_for_connections("2112");
	return 0;
}

/*
* Kicks off the server listening and accepting connections.
* When a connection is made, the a new thread is made to listen
* for messages from the client over the socket.
*/
static void listen_for_connections(std::string port)
{
	int sockfd, new_fd;                   // listen on sock_fd, new connection on new_fd
	struct addrinfo hints, *servinfo, *p; // hints used for connection flexibility
	struct sockaddr_storage client_addr;  // connector's address information
	socklen_t sin_size;
	int yes = 1;
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

	if (pthread_create(&exiter, NULL, &wait_quit, NULL) != 0)
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

		// create new thread to handle this connection on this socket.
		pthread_t new_thread;
		int ret;
		int * socket_ID = &new_fd;
		ret = pthread_create(&new_thread, NULL, &kick_to_listen, socket_ID);
		if (ret != 0)
		{
			printf("Error creating new pthread\n");
			exit(EXIT_FAILURE);
		}
	}
	return;
}

/*
* Continually listens and reads in messages from clients on the given socket.
* Once a new line character is detected, the request is processed and sent
* off to be interpreted in process_request. This method also detects when
* a client has disconnected, and saves the spreadsheet accordingly.
*/
void listen_to_client(int socket)
{
	clock_t ping_clock = clock();
	clock_t ping_response_clock = clock();
	std::string temp = "";
	int received;             // How much data has come through on the socket.
	while (1)
	{
		if ((clock() - ping_clock) * 1.0 / CLOCKS_PER_SEC >= 10)
		{
			iserver->send_string(socket, "ping \3");
			ping_clock = clock();
		}

		if ((clock() - ping_response_clock) * 1.0 / CLOCKS_PER_SEC >= 60)
		{
			iserver->process_disconnect(socket);
		}

		char msg[1];
		received = recv(socket, msg, 1, 0); // Recieve and process one byte at a time
		if (received < 1) // Something went wrong (most likely client disconnect)
			break;
		// new line character found, process the message
		if (msg[0] == (char)3)
		{
			iserver->process_request(socket, temp, ping_response_clock);
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
* Send a specified string over the specified socket with
* a newline character appended to the end.
*/
static void *wait_quit(void * arg)
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
* Static method for thread initialization. Required by the pthread_create function.
*/
static void *kick_to_listen(void * arg)
{
	// The parameter will be an integer specifying the connected client's socket.
	int * socket;
	socket = (int *)arg;

	// Begin listening to messages from the new client on the new thread.
	listen_to_client(*socket);
}