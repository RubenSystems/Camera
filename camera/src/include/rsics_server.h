#pragma once

#include <transmit.h>
#include <observe.h>

#include <thread>
#include <string>
#include <unordered_map>
#include <time.h>

namespace rsics {

	class Client {

		public:

			Client(const std::string & uid, const Computer & comp, uint32_t ttl = 32) :
				uid_(uid), addr_(comp), timestamp_(time(NULL)), ttl_(ttl) {}

		public:
			bool valid() {
				return timestamp_ + ttl_ > time(NULL);
			}

			void renew() {
				timestamp_ = time(NULL);
			}

			Computer * address() {
				return &addr_;
			}

			std::string uid() {
				return uid_;
			}

		private:
			std::string uid_; 
			Computer addr_; 
			unsigned long timestamp_; 
			uint32_t ttl_; 
	};

	class BroadcastServer {

		public: 

			BroadcastServer() {
				create_listener( "5253" , &listener_);
			}

			~BroadcastServer() {
				if (active_ != 0) {
					stop();
				}
			}
		
		public:

			void start() {
				active_ = 1;
				server_thread_ = std::thread([this](){
					observe_with_context(&listener_, &active_, (const void *)this, client_connection);
				});
			}

			void broadcast(void * data, size_t data_size) {
				for (auto & client: clients) {
					Client c = client.second;
					if (c.valid()) {
						transmit(data, data_size, c.address());
					} else {
						evict(c.uid());
					}
					
				}
			}

			bool has_clients() {
				return clients.size() > 0;
			}

		private:
			static void client_connection(const void * s, struct Computer * client, void * message, int message_length) {
				Computer c = *client;
				BroadcastServer * server = (BroadcastServer *)s;
				server->recieved_client(
					std::string((const char *)message, message_length),
					c
				);
				
			}

			void add_client(const std::string & uid, const Computer & comp) {
				Client new_client = Client(uid, comp);
				std::cout << "NEW CLIENT: " << uid << "\n";
				clients.emplace(uid, new_client);
			}

			void renew_client(const std::string & uid, const Computer & comp) {
				clients.at(uid).renew();
			}

			void recieved_client(const std::string & uid, const Computer & comp) {
				if (clients.find(uid) == clients.end()) {
					add_client(uid, comp);
				} else {
					renew_client(uid, comp);
				}
			}

			void evict(const std::string & uid ) {
				std::cout << "EVICT: " << uid << "\n";
				clients.erase(clients.find(uid));
			}

			void stop(){
				active_ = 0;
				server_thread_.join();
			}

		private: 
			Computer listener_;
			char active_; // boolean
			std::unordered_map<std::string, Client> clients;
			std::thread server_thread_;
	};
}