#ifndef __SHUTTLE_H__
#define __SHUTTLE_H__
#include <string>
#include <cstring>
#include <vector>
#include <iostream>
// #include "myrecv.h"
#include "readsocketdata.h"

#define BUFF_SIZE 30000
extern unsigned long uid;
extern std::fstream fs;
extern Cache cache;
extern Socket client_socket;

void shuttle(Socket* proxy_socket, std::string& rmsg, int connect_fd, int i, Request& req, Response& resp) {
	std::string empty_str;
	logServer(uid, fs, req.header, empty_str, req.host);
	proxy_socket->link(req.host.c_str(), std::to_string(req.port).c_str(), i);
	//send to server
	proxy_socket->send_to_server(rmsg.c_str(), rmsg.length() + 1); //
	std::vector<char> r_buff; // (BUFF_SIZE);
	// Response resp;
	int msg_len = 0;
	//receive from server
	// myrecv(resp, r_buff, proxy_socket->socket_fd, msg_len);
	r_buff = getreadsocketdata(proxy_socket->socket_fd, resp, msg_len);
	logServer(uid, fs, empty_str, resp.status, req.host);
	// send to client
	int status = send(connect_fd, r_buff.data(), msg_len, 0);
	if(status == -1) {
		std::cerr << "Error: send failed" << std::endl;
		// exit(EXIT_FAILURE);
		throw std::runtime_error("error");
	}
	logClient(uid, fs, resp.status);
}
#endif