#include "wrapper.hpp"

Wrapper::Wrapper() : Client() {
	this->client_initialized = false;
}
Wrapper::~Wrapper() {}

bool Wrapper::connect_to_proxy(const char* ip, const int& port, const Credentials* credentials) {
	if (!this->client_initialized) {
		if (!this->initialize()) {
			return false;
		}

		this->client_initialized = true;

	}

	if (!this->establish_proxy(ip, port, 20.f * 1000.f)) {
		printf("Failed to establish proxy connection.\n");
		return false;
	}

	printf("Connected to the proxy.\n");

	unsigned char auth_method = 0x00;
	if (credentials != nullptr) {
		auth_method = 0x02;
	}

	if (!greet(auth_method)) {
		printf("Failed to greet using %d authentication method.\n", auth_method);
		return false;
	}

	if (credentials != nullptr && !set_auth(credentials)) {
		printf("Failed to authenticate using provided credentials.\n");
		return false;
	}

	return true;
}

bool Wrapper::greet(const unsigned char& auth_method) {
	Packet greeting_packet = this->make_greeting(auth_method);
	if (!send_packet(&greeting_packet)) {
		delete[] greeting_packet.packet;
		return false;
	}

	delete[] greeting_packet.packet;

	Packet greeting_response = this->get_packet(2, 10.f * 1000.f);
	if (greeting_response.packet == nullptr) {
		return false;
	}

	if (!this->validate_greeting_response(&greeting_response, auth_method)) {
		delete[] greeting_response.packet;
		return false;
	}

	printf("The greeting was acknowledged by the server.\n");
	delete[] greeting_packet.packet;

	return true;
}

bool Wrapper::set_auth(const Credentials* credentials) {
	Packet auth_packet = this->make_auth(credentials->username, credentials->password);
	if (!send_packet(&auth_packet)) {
		delete[] auth_packet.packet;
		return false;
	}

	delete[] auth_packet.packet;

	Packet auth_response = this->get_packet(2, 20.f * 1000.f);
	if (auth_response.packet == nullptr) {
		return false;
	}

	if (!validate_auth_response(&auth_response)) {
		delete[] auth_response.packet;
		return false;
	}

	delete[] auth_response.packet;
	return true;
}

bool Wrapper::set_destination(const char* dst_ip, const int& dst_port) {
	Packet connection_packet = this->make_connection(dst_ip, dst_port);
	if (!this->send_packet(&connection_packet)) {
		delete[] connection_packet.packet;
		return false;
	}

	delete[] connection_packet.packet;

	Packet connection_response = this->get_packet(6, 10.f * 1000.f);
	if (connection_response.packet == nullptr) {
		return false;
	}

	if (!this->validate_connection_response(&connection_response)) {
		delete[] connection_response.packet;
		return false;
	}


	printf("The connection was acknowledged by the server.\n");
	delete[] connection_response.packet;

	return true;
}