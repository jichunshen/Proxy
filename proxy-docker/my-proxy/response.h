// Created by sjc on 2/23/18.
#ifndef MY_PROXY_RESPONSE_H
#define MY_PROXY_RESPONSE_H
#include <cstdlib>
#include <cstdio>
#include <string>
#include <vector>
#include <iostream>
#include <cmath>
#include <iomanip>
#include <utility>
#include <list>
#include <fstream>

class Response {
public:
    std::string status;
    std::string cache;
    std::string date;
    std::string expire;
    std::string length;
    std::string etag;
	std::string connection;
    std::string respond_buff;
    int code;
    Response(){
        code = 0;
    }
    void parse(const std::string &buff){
        std::size_t pos = buff.find("\r\n");
        status = buff.substr(0, pos);
        pos = buff.find(' ') + 1;
        std::size_t pos2 = buff.find(' ', pos);
        code = (int)strtol(buff.substr(pos, pos2-pos).c_str(), nullptr, 10);
        pos2 = buff.find("\r\nDate: ");
        if(pos2!=std::string::npos){
            pos = buff.find(' ', pos2) + 1;
            pos2 = buff.find("\r\n", pos);
            date = buff.substr(pos, pos2-pos);
        }
		pos2 = buff.find("\r\nConnection: ");
		if(pos2 != std::string::npos) {
			pos = buff.find(' ', pos2) + 1;
			pos2 = buff.find("\r\n", pos);
			connection = buff.substr(pos, pos2 - pos);
		}
        pos2 = buff.find("\r\nContent-Length: ");
        if(pos2!=std::string::npos){
            pos = buff.find(' ', pos2) + 1;
            pos2 = buff.find("\r\n", pos);
            length = buff.substr(pos, pos2-pos);
        }
        pos2 = buff.find("\r\nCache-Control: ");
        if(pos2!=std::string::npos){
            pos = buff.find(' ', pos2) + 1;
            pos2 = buff.find("\r\n", pos);
            cache = buff.substr(pos, pos2-pos);
        }
        pos2 = buff.find("\r\nExpires: ");
        if(pos2!=std::string::npos){
            pos = buff.find(' ', pos2) + 1;
            pos2 = buff.find("\r\n", pos);
            expire = buff.substr(pos, pos2-pos);
        }
        pos2 = buff.find("\r\nEtag: ");
        if(pos2!=std::string::npos){
            pos = buff.find(' ', pos2) + 1;
            pos2 = buff.find("\r\n", pos);
            etag = buff.substr(pos, pos2-pos);
        }
        respond_buff = buff;
    }
    void display(){
        std::cout<<"status:"<<status<<std::endl;
        std::cout<<"cache:"<<cache<<std::endl;
        std::cout<<"date:"<<date<<std::endl;
        std::cout<<"expire:"<<expire<<std::endl;
        std::cout<<"length:"<<length<<std::endl;
        std::cout<<"etag:"<<etag<<std::endl;
        std::cout<<"code:"<<code<<std::endl;
		std::cout<<"connection:"<<connection<<std::endl;
    }
};


#endif //MY_PROXY_RESPONSE_H
