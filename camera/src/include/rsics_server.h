#pragma once

#include <transmit.h>
#include <recieve.h>

#include <thread>
#include <string>
#include <unordered_map>
#include <time.h>
#include <functional>

namespace rsics {

	class Client {

		public:

			Client(const std::string & uid, const connection & comp, uint32_t ttl = 32) :
				uid_(uid), addr_(comp), timestamp_(time(NULL)), ttl_(ttl) {}

		public:
			bool valid() {
				return timestamp_ + ttl_ > time(NULL);
			}

			void renew() {
				timestamp_ = time(NULL);
			}

			connection * address() {
				return &addr_;
			}

			std::string uid() {
				return uid_;
			}

		private:
			std::string uid_; 
			connection addr_; 
			unsigned long timestamp_; 
			uint32_t ttl_; 
	};

	class BroadcastServer {

		public: 

			BroadcastServer();

			~BroadcastServer();
		
		public:

			void start();

			void broadcast(void * data, size_t data_size);

			bool has_clients();

			void set_on_client_change(std::function<void (int)> handler); 


		private:
			static void client_connection(void * s, struct connection * client, message_type type, void * message, uint64_t message_length) ;

			void add_client(const std::string & uid, const connection & comp);

			void renew_client(const std::string & uid, const connection & comp);

			void recieved_client(const std::string & uid, const connection & comp);

			void evict(const std::string & uid );

			void stop();


		private: 
			connection listener_;
			bool active_; // boolean
			std::unordered_map<std::string, Client *> clients;
			std::thread server_thread_;

			// Event handlers 
			std::function<void(int)> on_client_change_;
	};
}