#ifndef __LOG_H__
#define __LOG_H__
#include <unistd.h>
#include <cstring> 
#include <string>
#include <iostream>
#include <fstream>
#include <thread>
#include <mutex>
#include <time.h>

std::mutex m;

char* getcurtime() {
	time_t rawtime;
	struct tm* timeinfo;
	time(&rawtime);
	timeinfo = localtime(&rawtime);
	return asctime(timeinfo);
}

void logRequest(unsigned long uid, std::fstream& fs, std::string& reqline, const char* client_ip) {
	std::lock_guard<std::mutex> lock(m);
	char* time = getcurtime();
	fs << uid << ": " << reqline << " from " << client_ip << " @ " << time << std::endl;
	fs.flush();
}
void logClient(unsigned long uid, std::fstream& fs, std::string respline) {
	std::lock_guard<std::mutex> lock(m);
	fs << uid << ": Responding " << respline << std::endl;
	fs.flush();
}
void logCache(unsigned long uid, std::fstream& fs, int cache, std::string expiredtime) {
	std::lock_guard<std::mutex> lock(m);
	switch(cache) {
		case 0:
			fs << uid << ": not in cache" << std::endl;
			break;
		case 1:
			fs << uid << ": in cache, but expired at " << expiredtime << std::endl; //expiretime???
			break;
		case 2:
			fs << uid << ": in cache, valid" << std::endl;
			break;
		case 3: //Cache-Control: must-revalidate
			fs << uid << ": in cache, requires validation" << std::endl;
			break;
		default:
			fs << "(no-id): NOTE invalid cache status" << std::endl;
			break;
	}
	fs.flush();
}
//here, request and response are the request line and response line (first line) in the message), and server is the server name;
void logServer (unsigned long uid, std::fstream& fs, std::string& request, std::string& response, std::string& servername) {
	std::lock_guard<std::mutex> lock(m);
	if(!request.empty()) {
		fs << uid << ": Requesting " << request << " from " << servername << std::endl;
	}
	if(!response.empty()) {
		fs << uid << ": Received " << response << " from " << servername << std::endl;
	}
	fs.flush();
}
//If your proxy receives a 200-OK in response to a GET request, it should print one of the following:
void logOK(unsigned long uid, std::fstream& fs, int response, std::string expiretime) {
	std::lock_guard<std::mutex> lock(m);
	switch(response) {
		case 0: //not cacheable
			fs << uid << ": not cacheable because no-store" << std::endl; // no-store => do not cache
			break;
		case 1: //cache expire
			fs << uid << ": cached, expires at " << expiretime << std::endl;
			break;
		case 2: //cached, need revalidation
			fs << uid << ": cached, but requires re-validation" << std::endl;
			break;
		default:
			fs << "(no-id): NOTE invalid response status" << std::endl;
			break;
	}
	fs.flush();
}
//When your proxy is handling a tunnel as a result of 200-OK, it should log when the tunnel closes
//how to know the tunnel is closed
void logTunnel(unsigned long uid, std::fstream& fs) {
	std::lock_guard<std::mutex> lock(m);
	fs << uid << ": Tunnel closed" << std::endl;
	fs.flush();
}
//log Cache-Control
void logCC(unsigned long uid, std::fstream& fs, std::string& cc) {
	std::lock_guard<std::mutex> lock(m);
	if(!cc.empty()) {
		fs << uid << ": NOTE Cache-Control: " << cc << std::endl;
	}
	else {
		fs << uid << ": NOTE empty \'cache-control\', consider to be noncacheable" << std::endl;
	}
	fs.flush();
}
//log Etag
void logEtag(unsigned long uid, std::fstream& fs, std::string& etag) {
	std::lock_guard<std::mutex> lock(m);
	fs << uid << ": NOTE Etag: " << etag << std::endl;
	fs.flush();
}

#endif