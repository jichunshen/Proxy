// Created by sjc on 2/23/18.
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

#ifndef MY_PROXY_REQUEST_H
#define MY_PROXY_REQUEST_H

#define MSG1 "GET http://www.cplusplus.com/reference/unordered_map/unordered_map/ HTTP/1.1\r\nHost: www.cplusplus.com\r\nProxy-Connection: Keep-Alive\r\n\r\n"
#define MSG2 "GET http://www.sina.com.cn/ HTTP/1.1\r\nHost: www.sina.com.cn\r\nProxy-Connection: keep-alive\r\n\r\n"
#define MSG3 "GET http://stackoverflow.com/questions/37907986/error-in-recover-free-invalid-next-size-normal HTTP/1.1\r\nHost: stackoverflow.com\r\nProxy-Connection: Keep-Alive\r\n\r\n"
#define MSG4 "GET http://beej.us/guide/bgnet/output/html/multipage/getaddrinfoman.html HTTP/1.1\r\nHost: beej.us\r\nProxy-Connection: Keep-Alive\r\n\r\n"
#define MSG5 "GET http://pubs.opengroup.org/onlinepubs/9699919799/functions/strncat.html HTTP/1.1\r\nHost: pubs.opengroup.org\r\nProxy-Connection: Keep-Alive\r\n\r\n"
#define MSG6 "GET http://www.gnu.org/software/gsl HTTP/1.1\r\nHost: www.gnu.org\r\nProxy-Connection: Keep-Alive\r\n\r\n"
#define MSG7 "GET http://man7.org/linux/man-pages/man3/pthread_create.3.html HTTP/1.1\r\nHost: man7.org\r\nProxy-Connection: Keep-Alive\r\n\r\n"

class Request {
public:
    std::string method;
    std::string protocol;
    std::string host;
    std::string url;
    std::string sub_url;
    std::string header;
    int port;
    std::string return_buff;

    Request(){
        port = 0;
    }
    std::string parse(std::string buff){
        std::size_t pos = buff.find("\r\n");//find first occurrence of "\r\n"
        header = buff.substr(0,pos);
        pos = buff.find(' ');
        method = buff.substr(0, pos);
        std::size_t pos2 = buff.find(' ', pos+2);//+1?
        url = buff.substr(pos+1, pos2-(pos+1));
        pos2 = buff.find("://");
        if(pos2 != std::string::npos) {//include http
            protocol = buff.substr(pos + 1, pos2 - (pos + 1));
            pos = pos2 += 3;
        }
        else{
            pos = buff.find(' ')+1;
        }
        pos2 = buff.find(' ', pos);
        std::size_t pos3 = buff.find(':', pos);
        if(pos3 != std::string::npos && pos3 < pos2){//has port
            host = buff.substr(pos, pos3-pos);
            pos = pos3+1;
            pos3 = buff.find('/', pos);
            if(pos3 != std::string::npos && pos3 < pos2){//has sub_url
                sub_url = buff.substr(pos3, pos2-pos3);
                if(protocol.empty()){//protocol is empty, save port as protocol
                    protocol = buff.substr(pos, pos3-pos);
                }
                port = (int)strtol(buff.substr(pos, pos3-pos).c_str(), NULL, 10);
                // sub_url = buff.substr(pos3, pos2-pos3);
            }
            else{//no sub_url
                // pos3 = buff.find(':', pos);
                if(protocol.empty()){//protocol is empty, save port as protocol
                    protocol = buff.substr(pos, pos2-pos);
                }
                port = (int)strtol(buff.substr(pos, pos2-pos).c_str(), NULL, 10);
                //didn't set no sub_url as '/'
            }
        }
        else {//no port
            pos3 = buff.find('/', pos);
            if (pos3 != std::string::npos && pos3 < pos2) {//no port has sub_url
                host = buff.substr(pos, pos3 - pos);
                sub_url = buff.substr(pos3, pos2 - pos3);
            } else {//no port no sub_url
                host = buff.substr(pos, pos2 - pos);
                //didn't set no sub_url as '/'
            }
            if (protocol.empty()) {//no port no protocol
                protocol = "80";
            }
            port = 80;
        }
        std::string str_host = "\r\nHost: ";
        std::string str_connection = "\r\nConnection: ";
        std::string http = "HTTP/1.1";
        std::string close = "close";
        pos = buff.find("\r\n", pos2);
        int need_host = 0;
        int need_connection = 0;
        if(buff.find(str_host, pos)==std::string::npos){//no "\r\nHost: "
            need_host = 1;
        }
        if(buff.find(str_connection, pos)==std::string::npos){// no "\r\nConnection: "
            need_connection = 1;
        }
        if(method == "CONNECT"){
            return buff;
        }
        //std::string return_buff;
        return_buff = method + " " + sub_url + " " + http;
        if(need_host){
            return_buff += str_host + host;
        }
        if(need_connection){
            return_buff += str_connection + close;
        }
        pos3 = buff.find("\r\nProxy-Connection: ", pos);
        if(pos3!=std::string::npos) {
            return_buff += buff.substr(pos, pos3 - pos);
        }
        pos3 += 2;
        return_buff += buff.substr(buff.find("\r\n", pos3));
        return return_buff;
    }
    void display(){
        std::cout<<"method:"<<method<<std::endl;
        std::cout<<"protocol:"<<protocol<<std::endl;
        std::cout<<"host:"<<host<<std::endl;
        std::cout<<"url:"<<url<<std::endl;
        std::cout<<"sub_url:"<<sub_url<<std::endl;
        std::cout<<"header:"<<header<<std::endl;
        std::cout<<"port:"<<port<<std::endl;
        std::cout<<"return buff:"<<"\n"<<return_buff<<std::endl;
    }
};

#endif //MY_PROXY_REQUEST_H
