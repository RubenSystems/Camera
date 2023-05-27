#include <iostream>
#include <rsics_server.h>
#include <string.h>
#include <vector>

using namespace rsics;

BroadcastServer::BroadcastServer() { rsics_make_listener("5253", &listener_); }

BroadcastServer::~BroadcastServer() {
  if (active_ != 0) {
    stop();
  }
}

void BroadcastServer::start() {
  active_ = 1;
  server_thread_ = std::thread([this]() {
    struct message_callback callback =
        create_message_callback(this, &client_connection);
    rsics_listen(&listener_, &active_, callback);
  });
}

void BroadcastServer::broadcast(void *data, size_t data_size) {

  static std::vector<std::string> eviction_list;

  for (auto &client : clients) {
    Client *c = client.second;
    if (c->valid()) {
      rsics_transmit(data, data_size, c->address());
    } else {
      eviction_list.push_back(c->uid());
    }
  }

  for (auto &uid : eviction_list)
    evict(uid);
  if (eviction_list.size() > 0)
    eviction_list.clear();
}

bool BroadcastServer::has_clients() { return clients.size() > 0; }

void BroadcastServer::client_connection(void *s, struct connection *client,
                                        message_type type, void *message,
                                        uint64_t message_length) {
  connection c = *client;
  char uid[37];
  strncpy(uid, (char *)message, 36);
  uid[36] = 0;

  BroadcastServer *server = (BroadcastServer *)s;
  server->recieved_client(std::string(uid), c);
}

void BroadcastServer::add_client(const std::string &uid,
                                 const connection &comp) {
  Client *new_client = new Client(uid, comp);
  clients[uid] = new_client;
}

void BroadcastServer::renew_client(const std::string &uid,
                                   const connection &comp) {
  clients[uid]->renew();
}

void BroadcastServer::recieved_client(const std::string &uid,
                                      const connection &comp) {
  if (clients.find(uid) == clients.end()) {
    add_client(uid, comp);
  } else {
    renew_client(uid, comp);
  }
  on_client_change_(clients.size());
}

void BroadcastServer::evict(const std::string &uid) {
  delete clients[uid];
  clients.erase(uid);
  on_client_change_(clients.size());
  std::cout << "EVICT: " << uid << "\n";
}

void BroadcastServer::stop() {
  active_ = 0;
  server_thread_.join();
}

void BroadcastServer::set_on_client_change(std::function<void(int)> handler) {
  on_client_change_ = handler;
}