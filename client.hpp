#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <WinSock2.h>
#include <WS2tcpip.h>

#include <chrono>
#include <thread>

#include <stdio.h> // logging

struct Packet {
	int size;
	unsigned char* packet;
};


class Client {
private:
	SOCKET proxy_server;

public:
	Client();
	~Client();

	bool initialize();
	bool establish_proxy(const char* ip, const int& port, const float& timeout_ms);

	Packet make_greeting(const unsigned char& auth_method);
	bool validate_greeting_response(const Packet* packet, const unsigned char& auth_method);

	Packet make_auth(const char* id, const char* pw); // only for username/password auth
	bool validate_auth_response(const Packet* packet);

	Packet make_connection(const char* ip, const int& port);
	bool validate_connection_response(const Packet* packet);

	Packet get_packet(const int& len, const float& timeout_ms);
	bool send_packet(const Packet* packet);


};

#endif