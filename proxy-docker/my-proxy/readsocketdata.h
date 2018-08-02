#ifndef __READSOCKETDATA_H__
#define __READSOCKETDATA_H__
#include <iostream>
#include <cstring>
#include <string>
#include <unistd.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <fstream>
#include <thread>
#include <mutex>
#include <pthread.h>
#include <vector>
#include "socket.h"
#include "cache.h"
#include "request.h"
#include "response.h"
#define BUFF_SIZE 30000
std::vector<char> getreadsocketdata (int socket_fd, Response &respheader, int & already_recv){
	std::vector<char> r_buff(BUFF_SIZE);
	int index = 0;
	std::cout<<"begin getreadsocketdata recv header from server\n";
	already_recv = recv(socket_fd, r_buff.data(), BUFF_SIZE, 0);
	if(already_recv == 0){
		std::vector<char> marker = {'c', 'l', 'o', 's', 'e'};
		std::cout<<"recv getreadsocketdata from server close\n";
		return marker;
	}
	std::cout<<"finish getreadsocketdata recv header from server\n";
	index = already_recv;
	std::string recv_header(r_buff.begin(), r_buff.end());
	respheader.parse(recv_header);
	if(!(respheader.length.empty())){
		int alldata_len = (int) strtol(respheader.length.c_str(), NULL, 10);
		int end_header = recv_header.find("\r\n\r\n");
		end_header += 4;
		int already_recvdata = already_recv - end_header;
		int expect_recvdata = alldata_len - already_recvdata + already_recv;
		int recvlen = 0;
		while(already_recv < expect_recvdata){
			r_buff.resize(BUFF_SIZE+already_recv);
			std::cout<<"begin recv extra data from server\n";
			recvlen = recv(socket_fd, &r_buff.data()[index], BUFF_SIZE, 0);
			std::cout<<"finish recv extra datafrom server\n";
			if(recvlen == 0){
				break;
			}
			if(recvlen < 0) {
				//error handle
				std::cerr << "Error : recv from origin server failed" << std::endl;
				// exit(EXIT_FAILURE);
				throw std::runtime_error("error\n");
			}
			already_recv += recvlen;
			index = already_recv;
		}
	}
	else{
		while(index > 0){
			r_buff.resize(BUFF_SIZE+already_recv);
			index = recv(socket_fd, &r_buff.data()[already_recv], BUFF_SIZE,0);
			already_recv += index;	
		}
	}
	r_buff.resize(already_recv);
	return r_buff;
}
std::vector<char> readsocketdata (int socket_fd, Response &respheader, int & already_recv){
	std::vector<char> r_buff(BUFF_SIZE);
	int index = 0;
	already_recv = recv(socket_fd, r_buff.data(), BUFF_SIZE, 0);
	if(already_recv == 0){
		std::vector<char> marker = {'c', 'l', 'o', 's', 'e'};
		std::cout<<"recv from server close\n";
		return marker;
	}
	index = already_recv;
	std::string recv_header(r_buff.begin(), r_buff.end());
	respheader.parse(recv_header);
	if(!(respheader.length.empty())){
		int alldata_len = (int) strtol(respheader.length.c_str(), NULL, 10);
		int end_header = recv_header.find("\r\n\r\n");
		end_header += 4;
		int already_recvdata = already_recv - end_header;
		int expect_recvdata = alldata_len - already_recvdata + already_recv;
		int recvlen = 0;
		while(already_recv < expect_recvdata){
			r_buff.resize(BUFF_SIZE+already_recv);
			recvlen = recv(socket_fd, &r_buff.data()[index], BUFF_SIZE, 0);
			if(recvlen == 0){
				break;
			}
			if(recvlen < 0) {
				//error handle
				std::cerr << "Error : recv from origin server failed" << std::endl;
				// exit(EXIT_FAILURE);
				throw std::runtime_error("error\n"); 
			}
			already_recv += recvlen;
			index = already_recv;
		}
	}
	r_buff.resize(already_recv);
	return r_buff;
}
#endif