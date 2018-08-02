#ifndef __KALVIE_H__
#define __KALVIE_H__
#include <iostream>
#include <vector>
#include <string>
#include <cstring>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include "response.h"
#include "request.h"
// #include "myrecv.h"
#include "readsocketdata.h"

#define BUFF_SIZE 30000
extern unsigned long uid;
extern std::fstream fs;
extern Cache cache;
extern Socket client_socket;

void conn_alive(int connect_fd, Socket* proxy_socket) {
	fd_set readfds;
	std::string empty_str;
	int num = 0;
	std::vector<char> marker = {'c', 'l', 'o', 's', 'e'};
	while(true) {
		FD_ZERO(&readfds);
		FD_SET(connect_fd, &readfds);
		FD_SET(proxy_socket->socket_fd, &readfds);
		int status = select(FD_SETSIZE, &readfds, NULL, NULL, NULL);
		if(status == -1) {
			std::cerr << "Keep-Alive Error: select failed" << std::endl;
			delete proxy_socket;
			return;
		}
		Request conn_req;
		std::cout << "Select Keep-Alive No. " << num << std::endl;
		try {
		if(FD_ISSET(connect_fd, &readfds)) { // receive from browser
			std::cout << "Keep-Alive Receive from Browser" << std::endl;
			std::vector<char> conn_buffer(BUFF_SIZE);
			// uid++;
			int recv_len = recv(connect_fd, conn_buffer.data(), BUFF_SIZE, 0);
			// need parse => keep-alive
			std::string conn_rmsg(conn_buffer.data());
			// Request conn_req;
			conn_req.parse(conn_rmsg);
			logRequest(uid, fs, conn_req.header, client_socket.client_ip);
			// lineminus;
			// std::cout << conn_buffer.data() << std::endl;
			// lineminus;
			if(recv_len == 0) break;
			if(recv_len < 0) {
				std::cerr << "Keep-Alive Error: recv from client failed" << std::endl;
				// exit(EXIT_FAILURE);
				return;
			}
			// proxy_socket->send_to_server(conn_buffer.data(), recv_len);
			std::cout << "Keep-Alive Send to Server" << std::endl;
			// conn_buffer.resize(recv_len);
			if(send(proxy_socket->socket_fd, conn_buffer.data(), recv_len, 0) < 0) break;
			// logServer(uid, fs, conn_req.header, empty_str, conn_req.host);
		}
		else if(FD_ISSET(proxy_socket->socket_fd, &readfds)) { // receive from server
			std::cout << "keep alive Receive from Server" << std::endl;
			std::vector<char> conn_buffer; // (BUFF_SIZE);
			Response conn_resp;
			int msg_len;
			// if(myrecv(conn_resp, conn_buffer, proxy_socket->socket_fd, msg_len)) break;
			conn_buffer = getreadsocketdata(proxy_socket->socket_fd, conn_resp, msg_len);
			if(conn_buffer == marker) break;
			logServer(uid, fs, empty_str, conn_resp.status, conn_req.host);
			std::cout << "keep alive Finish Receiving from Server" << std::endl;
			// linestar;
			// std::cout << conn_buffer.data() << std::endl;
			// linestar;
			// conn_buffer.resize(msg_len);
			std::cout << "keep alive Send to Client" << std::endl;
			if(send(connect_fd, conn_buffer.data(), msg_len, 0) < 0) break;
			// conn_buffer.resize(BUFF_SIZE, '\0');
		}
		}
		catch (const std::out_of_range& oor){
			std::cerr << "Out of range error" << std::endl;
			break;
		}
		num++;
	}
}
#endif