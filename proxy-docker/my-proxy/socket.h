#ifndef __SOCKET_H__
#define __SOCKET_H__
#include <netdb.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <iostream>
#include <cstring>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>

class Socket {
public:
	struct addrinfo hints;
	struct addrinfo* result;
	int socket_fd;
	int connect_fd;
	int socket_server_fd;
	char* client_ip;
	//constructor
	Socket():result(NULL), socket_fd(-1), connect_fd(-1), socket_server_fd(-1), client_ip(NULL) {
		memset(&hints, 0, sizeof(hints));
	}
	//destructor
	~Socket() {
		if(result) freeaddrinfo(result);
		if(client_ip) free(client_ip);
		if(socket_fd != -1) {close(socket_fd);}
		if(connect_fd != -1) {close(connect_fd);}
		if(socket_server_fd != -1) {close(socket_server_fd);}
	}
	//copy constructor
	
	//accept method
	void setup(const char* hostname, const char* port, int i) {
		hints.ai_family = AF_UNSPEC;
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_flags = AI_PASSIVE;
		int status;
		status = getaddrinfo(hostname, port, &hints, &result);
		if(status != 0) {
			std::cerr << "Error: getaddrinfo" << std::endl;
			// exit(EXIT_FAILURE);
			throw std::runtime_error("error");
		}
		socket_server_fd = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
		if(socket_server_fd == -1) {
			std::cerr << "Error: socket create failed" << std::endl;
			// exit(EXIT_FAILURE);
			throw std::runtime_error("error");
		}
		//reuse setting omitted
		int yes = 1;
		status = setsockopt(socket_server_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
		status = bind(socket_server_fd, result->ai_addr, result->ai_addrlen);
		if(status == -1) {
			std::cerr << "Error: bind failed" << std::endl;
			// exit(EXIT_FAILURE);
			throw std::runtime_error("error");
		}
		status = listen(socket_server_fd, 10);
		if(status == -1) {
			std::cerr << "Error: listen failed" << std::endl;
			// exit(EXIT_FAILURE);
			throw std::runtime_error("error");
		}
	}
	void acc(int i) {
		struct sockaddr_in clientinfo;
		socklen_t addrlen = sizeof(struct sockaddr_in);
		connect_fd = accept(socket_server_fd, (struct sockaddr*)&clientinfo, &addrlen);
		client_ip = inet_ntoa(clientinfo.sin_addr);//need free?
		if(connect_fd == -1) {
			std::cerr << "Error: accept failed" << std::endl;
			// exit(EXIT_FAILURE);
			throw std::runtime_error("error");
		}
		std::cout << i << ": Accept Success" << std::endl;
		// return 0;
	}
	//connect method
	void link(const char* hostname, const char* port, int i) {
		hints.ai_family = AF_UNSPEC;
		hints.ai_socktype = SOCK_STREAM;
		int status;
		status = getaddrinfo(hostname, port, &hints, &result);
		if(status != 0) {
			std::cerr << "Error: getaddrinfo failed" << std::endl;
			// return;
			throw std::runtime_error("error");
		}
		socket_fd = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
		if(socket_fd == -1) {}
		status = connect(socket_fd, result->ai_addr, result->ai_addrlen);
		if(status == -1) {
			std::cerr << "Error: connect failed" << std::endl;
			// return;
			throw std::runtime_error("error");
		}
		std::cout << i << ": Connect Success" << std::endl;
	}
	//send method
	// void send_to_client(const char* buff, size_t len) {
		// int sentlen;
		// size_t index = 0;
		// while(index < len) {
			// if((sentlen = send(connect_fd, buff + index, len - index, 0)) < 0) {
				// std::cerr << "Error: send failed" << std::endl;
				// return;
			// }
			// index += sentlen;
		// }
	// }
	// void send_to_server(const char* buff, size_t len) {
		// int sentlen;
		// size_t index = 0;
		// while(index < len) {
			// if((sentlen = send(socket_fd, buff + index, len - index, 0)) < 0) {
				// std::cerr << "Error: send failed" << std::endl;
				// return;
			// }
			// index += sentlen;
		// }
	// }
	//send method
	void send_to_client(const char* buff, size_t len) {
		int status = send(connect_fd, buff, len, 0);
		if(status == -1) {
			std::cerr << "Error: send failed" << std::endl;
		}
	}
	void send_to_server(const char* buff, size_t len) {
		int status = send(socket_fd, buff, len, 0);
		if(status == -1) {
			std::cerr << "Error: send failed" << std::endl;
		}
	}
};

#endif
		
