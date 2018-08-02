#include <cstring>
#include <cstdlib>
#include <string>
#include <mutex>
#include <thread>
#include <functional>
#include "cache.h"
#include "handle.h"
#include "socket.h"

#define CACHE_SIZE 100
#define BUFF_SIZE 30000
unsigned long uid = 100;

/*
Since we program based on C++11, we try to avoid using new memory.
Therefore, most of our function are strong guarantee.
The only function which will have new operation is in cache class, 
but it is also strong guarantee, if it throw an exception, nothing bad will happened. 
We have handled most exception properly.
*/

std::fstream fs("/var/log/erss/proxy.log", std::fstream::app);
Cache cache(CACHE_SIZE);
Socket client_socket;

int main(int argc, char** argv) {
	if(argc != 2) {
		std::cerr << "Error: invalid arguments" << std::endl;
		exit(EXIT_FAILURE);
	}
	int i=0;
	client_socket.setup(NULL, argv[1], i);
	while(true) {
		std::cout << "Thread No : " << i << std::endl;
		client_socket.acc(i);
		Socket *proxy_socket = new Socket();
		std::thread t(handle, uid, proxy_socket, client_socket.connect_fd, i);
		t.detach();
		i++; uid++;
	}
	fs.close();
	return 0;
}
