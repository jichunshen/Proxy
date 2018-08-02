//
// Created by sjc on 2/23/18.
//
#include <cstdlib>
#include <cstdio>
#include <string>
#include <vector>
#include <iostream>
#include <cmath>
#include <utility>
#include <list>
#include <fstream>
#include <time.h>
#include <thread>
#include <mutex>
#include "response.h"
#include "log.h"

#ifndef MY_PROXY_CACHE_H
#define MY_PROXY_CACHE_H

std::mutex mtx;

class Cache {
public:
    class block{
    public:
        std::string url;
        Response response;
		block(){};
        block(std::string url, Response response):url(url), response(response){};
    };
    size_t cache_size;
	block* tar;
    std::list<block> Cache_list;
    Cache():cache_size(0), tar(NULL){};
    Cache(size_t cache_size):cache_size(cache_size), tar(NULL){};
    void insert(std::string url, Response respond){
		std::lock_guard<std::mutex> lock(mtx);
        //check whether list size is equal to cache_size, if equal{delete the last one}, add to front
        block temp(url, respond);
        if(Cache_list.size()==cache_size){
            Cache_list.pop_back();
        }
        Cache_list.push_front(temp);
    }
    block* search(unsigned long uid, std::fstream& fs, std::string& url){
		std::lock_guard<std::mutex> lock(mtx);
        //search url, if found, return it and add it to front, delete original one
        //if not found, call insert
        int status = 0;// 0:not in cache, 1:in cache but expired, 2:in cache not expired
        std::list<block>::iterator it;
        // std::string return_buff;
		// block *tar;
        for(it = Cache_list.begin(); it != Cache_list.end(); ++it){
            if((*it).url == url){
                if(isExpired((*it).response)){
                    // status = 1;
                    Cache_list.erase(it);
                }
                else { 
					if(it->response.etag.empty() && !it->response.expire.empty()) status = 1; //expire time
					if(it->response.etag.empty() && it->response.expire.empty()) status = 2; // valid
					if(!it->response.etag.empty() && it->response.expire.empty()) status = 3; // revalidation
                    block temp = *it;
                    // return_buff = (*it).response.respond_buff;
					tar = new block(url, it->response);
                    Cache_list.push_front(temp);
                    Cache_list.erase(it);
                }
                break;
            }
        }
        logCache(uid, fs, status, ((*it).response.expire));
        // return return_buff;
		return tar;
    }
	void remove() {
		std::lock_guard<std::mutex> lock(mtx);
		Cache_list.erase(Cache_list.begin());
	}
    static int isExpired(Response response){
    //compare with current_date, deciding whether expire
        struct tm expire;
        time_t now;
        time(&now);
        std::string expire_time = response.expire;
        strptime(expire_time.c_str(), "%a, %d %b %Y %H:%M:%S GMT", &expire);
        time_t expire_t = mktime(&expire);
        double seconds = difftime(now, expire_t);//para1-para2
        if(seconds >= 0){
            return 1;
        }
        else{
            return 0;
        }
    }
};


#endif //MY_PROXY_CACHE_H
