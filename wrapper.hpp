#ifndef WRAPPER_HPP
#define WRAPPER_HPP

#include "client.hpp"

struct Credentials {
	const char* username;
	const char* password;
};

class Wrapper : public Client {
private:
	bool client_initialized;

	bool greet(const unsigned char& auth_method);
	bool set_auth(const Credentials* credentials);

public:
	Wrapper();
	~Wrapper();

	bool connect_to_proxy(const char* ip, const int& port, const Credentials* credentials);
	bool set_destination(const char* dst_ip, const int& dst_port);

};


#endif