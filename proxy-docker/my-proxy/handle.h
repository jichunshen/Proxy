#ifndef __HANDLE_H__
#define __HANDLE_H__
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
#include <exception>
#include <vector>
#include "socket.h"
#include "cache.h"
#include "request.h"
#include "response.h"
#include "readsocketdata.h"
#include "kalive.h"
#include "shuttle.h"

#define BUFF_SIZE 30000
extern unsigned long uid;
extern std::fstream fs;
extern Cache cache;
extern Socket client_socket;

// std::mutex mtx;

double getAge(const char* date){ //age
	char* curr = getcurtime();
	struct tm c;
	strptime(curr, "%a, %b %d %H:%M:%S %Y", &c);
	struct tm d;
	strptime(date, "%a, %d %b %Y %H:%M:%S GMT", &d);
	time_t date_t = mktime(&d);
	time_t curr_t = mktime(&c);
	double diff = difftime(curr_t, date_t); //max-age
	return diff;  
}

//entry for multithreads
void handle(unsigned long uid, Socket* proxy_socket, int connect_fd, int i) {
	try {
		std::vector<char> buff(BUFF_SIZE);
		int len = recv(connect_fd, buff.data(), BUFF_SIZE, 0);
		buff.resize(len);
		std::string end_4(buff.end()-4, buff.end());
		std::string rmsg(buff.data());
		Request req;
		req.parse(rmsg);
		if(end_4 != "\r\n\r\n" && req.method!="POST"){
			std::cerr << "Invalid Request Format" << std::endl;
			std::cout << "Thread " << i << " Finished." << std::endl;
			delete proxy_socket ;
			return;
		}
		if(len <= 0) {
			delete proxy_socket;
			return;
		}
		// std::string rmsg(buff.data());
		// Request req;
		// req.parse(rmsg);
		if(req.method == "POST" && !req.length.empty()){
			int alldata_len = (int) strtol(req.length.c_str(), NULL, 10);
			std::string recv_header(buff.begin(), buff.end());
			int end_header = recv_header.find("\r\n\r\n");
			end_header += 4;
			int already_recvdata = len - end_header;
			int expect_recvdata = alldata_len - already_recvdata + len;
			int recvlen = 0;
			while(len < expect_recvdata){
				buff.resize(BUFF_SIZE+len);
				recvlen = recv(connect_fd, &buff.data()[len], BUFF_SIZE, 0);
				if(recvlen == 0){
					break;
				}
				if(recvlen < 0) {
					//error handle
					std::cerr << "Error : recv from client failed (POST)" << std::endl;
					// exit(EXIT_FAILURE);
					return;
				}
				len += recvlen;
			}
			buff.resize(len);
		}
		logRequest(uid, fs, req.header, client_socket.client_ip);
		if(req.method == "GET" || req.method == "POST") {
			// std::string res;
			Cache dummy;
			if(req.method == "GET"){
				// std::lock_guard<std::mutex> lock(mtx);
				dummy.tar = cache.search(uid, fs, req.url);
				std::cout<<"finsh cache search\n";
				if(dummy.tar != NULL){
					std::cout << "URL:          " << dummy.tar->url << std::endl;
				}
			}	
			if((req.method == "GET" && !dummy.tar) || req.method == "POST") { // GET not found|POST
				std::string empty_str;
				logServer(uid, fs, req.header, empty_str, req.host);
				//std::cout << "link" << std::endl;
				proxy_socket->link(req.host.c_str(), std::to_string(req.port).c_str(), i);
				proxy_socket->send_to_server(buff.data(), len);
				Response respheader;
				int already_recv = 0;
				std::vector<char> r_buff = getreadsocketdata(proxy_socket->socket_fd, respheader, already_recv);//socket, response, already 
				logServer(uid, fs, empty_str, respheader.status, req.host);
				/*
				   0 => not cacheable
				   1 => expire
				   2 => revalidation
				*/
				if(!req.method.compare("GET") && respheader.code == 200) {
					if(respheader.cache.find("no-store") != std::string::npos) {
						logOK(uid, fs, 0, std::string());
					}
					if(!respheader.expire.empty()) {
						logOK(uid, fs, 1, respheader.expire);
					}
					if(respheader.cache.find("must-revalidate") != std::string::npos || respheader.cache.find("no-cache") != std::string::npos) {
						logOK(uid, fs, 2, std::string());
					}
				}
				//cache handle
				if(!req.method.compare("GET") && respheader.code == 200) {
					
					if(respheader.cache.empty() || !respheader.cache.compare("no-store")) { // non-cacheable
						std::string empty_str;
						logCC(uid, fs, empty_str);
						// cache.insert(req.url, resp);
					}
					else { // cache
						logCC(uid, fs, respheader.cache);
						cache.insert(req.url, respheader);
					}
				}
				//Etag handle
				if(!respheader.etag.empty()) {
					logEtag(uid, fs, respheader.etag);
				}
				//send message received from origin server to client
				int status = send(connect_fd, r_buff.data(), already_recv, 0);
				if(status == -1) {
					std::cerr << "Error: send failed" << std::endl;
				}
				logClient(uid, fs, respheader.status);
			}
			
			else if(req.method == "GET" && dummy.tar) { //cache hit
				std::string empty_str;
				if(dummy.tar->response.cache.find("must-revalidate") != std::string::npos
					|| dummy.tar->response.cache.find("no-cache") != std::string::npos) { // must-revalidate => highest priority
					if(dummy.tar->response.etag.empty()) { // no Etag header
						// do nothing
					}
					else {
						std::cout << "Etag: " << dummy.tar->response.etag << std::endl;
						std::vector<char> etag_buff; // (BUFF_SIZE);
						//size_t pos = req.return_buff.find("\r\n");
						//std::string Etag = "If-None-Match: " + dummy.tar->response.etag + "\r\n";
						//req.return_buff.insert(pos + 2, Etag);
						// logServer(uid, fs, req.header, empty_str, req.host);//???
						// send(proxy_socket->socket_fd, tar.response.etag.c_str(), tar.response.etag.length(), 0);
						proxy_socket->send_to_server(dummy.tar->response.etag.c_str(), dummy.tar->response.etag.length()); // dummy.tar->response.etag.c_str()
						// proxy_socket->send_to_server(req.return_buff.c_str(), req.return_buff.length());
						Response etag_resp;
						int msg_len = 0;
						// myrecv(etag_resp, etag_buff, proxy_socket->socket_fd, msg_len);
						etag_buff = getreadsocketdata(proxy_socket->socket_fd, etag_resp, msg_len);//socket, response, already 
						logServer(uid, fs, empty_str, dummy.tar->response.status, req.host);
						std::string etag_str(etag_buff.begin(), etag_buff.end());
						if(etag_str.find("304") != std::string::npos) { // Valid
							send(connect_fd, dummy.tar->response.respond_buff.c_str(), dummy.tar->response.respond_buff.length(), 0);
							logClient(uid, fs, etag_resp.status); // "HTTP/1.1 304 Not Modified"
						}
						else { // 200 OK
							cache.remove(); // remove the head of the cache list
							int status = send(connect_fd, etag_buff.data(), msg_len, 0);
							if(status == -1) {
								std::cerr << "Error: send failed" << std::endl;
								// exit(EXIT_FAILURE);
								return;
							}
							//log info sent to client
							logClient(uid, fs, etag_resp.status); // dummy.tar->response.status
						}
					}
				}
				if(dummy.tar->response.cache.find("max-age") != std::string::npos) { // max-age
					const char* ptr = dummy.tar->response.cache.c_str();
					size_t pos = dummy.tar->response.cache.find("=") + 1;
					long span = strtol(ptr + pos, NULL, 10);
					double age = getAge(dummy.tar->response.date.c_str());
					if(span <= age) { // expired => send request to origin server
						cache.remove();
						Response hit_resp;
						shuttle(proxy_socket, rmsg, connect_fd, i, req, hit_resp);
						cache.insert(req.url, hit_resp);//
					}
					else { // send the content in cache
						send(connect_fd, dummy.tar->response.respond_buff.c_str(), dummy.tar->response.respond_buff.length(), 0);
						logClient(uid, fs, dummy.tar->response.status); //
					}
				}
				else if(!dummy.tar->response.expire.empty()) { // no max-age , expire
					if(Cache::isExpired(dummy.tar->response)) { // expired => send request to origin server
						cache.remove();
						Response hit_resp;
						shuttle(proxy_socket, rmsg, connect_fd, i, req, hit_resp);
						cache.insert(req.url, hit_resp);//
					}
					else {// send the content in cache
						send(connect_fd, dummy.tar->response.respond_buff.c_str(), dummy.tar->response.respond_buff.length(), 0);
						logClient(uid, fs, dummy.tar->response.status);
					}
				}
				else { 
					// do nothing
					send(connect_fd, dummy.tar->response.respond_buff.c_str(), dummy.tar->response.respond_buff.length(), 0);
					logClient(uid, fs, dummy.tar->response.status);
				}
			}
			
			std::cout << i << ": Finish processing GET | POST request" << std::endl;
		}
		
		else if(req.method == "CONNECT"){
			std::string empty_str;
			logServer(uid, fs, req.header, empty_str, req.host);
			proxy_socket->link(req.host.c_str(), std::to_string(req.port).c_str(), i);//server socket setup
			std::string http200ok = "200 OK\r\n\r\n";
			std::vector<char> marker = {'c', 'l', 'o', 's', 'e'};
			send(connect_fd, http200ok.c_str(), http200ok.size(), 0);
			while(true){
				fd_set readfds;
				int maxFD = 0;
				FD_ZERO(&readfds);
				FD_SET(connect_fd, &readfds);
				FD_SET(proxy_socket->socket_fd, &readfds);
				maxFD = connect_fd>(proxy_socket->socket_fd)?connect_fd:proxy_socket->socket_fd;
				int status = select(maxFD+1, &readfds, NULL, NULL, NULL);
				if(status == -1){
					std::cerr<<"Error: select failed\n";
					// exit(EXIT_FAILURE);
					return;
				}                                  
				if(FD_ISSET(connect_fd, &readfds)){
					std::vector<char> req_buff(BUFF_SIZE);
					int recv_len;
					recv_len = recv(connect_fd, req_buff.data(), BUFF_SIZE, 0);
					if(recv_len == -1){
						// exit(EXIT_FAILURE);
						return;
					}
					if(recv_len == 0)break;
					send(proxy_socket->socket_fd, req_buff.data(), recv_len, 0);
				}
				else if(FD_ISSET(proxy_socket->socket_fd, &readfds)){
					Response respheader;
					int already_recv;
					std::vector<char> resp_buff = readsocketdata(proxy_socket->socket_fd, respheader, already_recv);
					if(resp_buff == marker){
						break;
					}
					send(connect_fd, resp_buff.data(), already_recv, 0);
				}
			}
		}
		close(connect_fd);
		std::cout << "Thread " << i << " Finished." << std::endl;
		delete(proxy_socket);
	} catch (const std::exception& e) {
		std::cerr << "exception caught" << e.what() << std::endl;
		return;
	}
}
#endif