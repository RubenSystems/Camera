#include <rsics_client.h> 
#include <transmit.h>


using namespace rsics; 


Server::Server(const char * port, const char * ip) {
	server_connection_ = rsics_init_connection();
	rsics_connect(ip, port, server_connection_);
}

void Server::send(void * data, size_t size){ 
	rsics_transmit(data, size, server_connection_);
}