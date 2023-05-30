#pragma once

#include <recieve.h>



namespace rsics {

	class Server {
		public: 
			Server(const char * port, const char * ip);

			void send(void * data, size_t size);

		private: 
			struct connection * server_connection_;	
	};

}