#include "client.hpp"

Client::Client() {
	this->proxy_server = INVALID_SOCKET;
}

Client::~Client() {
    if (this->proxy_server != INVALID_SOCKET) {
        closesocket(this->proxy_server);
    }

    WSACleanup();
}

bool Client::initialize() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        printf("Failed to initiliaze WinSock2\n");
        return false;
    }

    this->proxy_server = socket(AF_INET, SOCK_STREAM, 0);
    if (this->proxy_server == INVALID_SOCKET) {
        printf("Could not create the proxy server's socket.\n");
        return false;
    }

    int reuse = 1;
    if (setsockopt(this->proxy_server, SOL_SOCKET, SO_REUSEADDR, (const char*)&reuse, sizeof(reuse)) == SOCKET_ERROR) {
        return false;
    }

    unsigned long mode = 1;
    if (ioctlsocket(this->proxy_server, FIONBIO, &mode) == SOCKET_ERROR) {
        return false;
    }

    return true;

}

bool Client::establish_proxy(const char* ip, const int& port, const float& timeout_ms) {
    sockaddr_in proxy_address;
    proxy_address.sin_family = AF_INET;
    proxy_address.sin_port = htons(port);
    if (inet_pton(AF_INET, ip, &(proxy_address.sin_addr)) != 1) {
        return false;
    }

    std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();

    while (std::chrono::duration<float>(std::chrono::steady_clock::now() - start).count() < timeout_ms / 1000.f) {
        int status = connect(this->proxy_server, reinterpret_cast<sockaddr*>(&proxy_address), sizeof(proxy_address));
        int error = WSAGetLastError();

        if (status != INVALID_SOCKET || error == WSAEISCONN) {
            return true;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }


    return false;
}

Packet Client::make_greeting(const unsigned char& auth_method) {
    Packet packet;
    unsigned char* p = new unsigned char[3];

    p[0] = 0x05; // socks5
    p[1] = 0x01; // 1 method
    p[2] = auth_method;

    packet.size = 3;
    packet.packet = p;

    return packet;
}

bool Client::validate_greeting_response(const Packet* packet, const unsigned char& auth_method) {
    return packet->packet[0] == 5 && packet->packet[1] == auth_method;
}

Packet Client::make_auth(const char* id, const char* pw) {
    Packet packet;

    int id_len = strlen(id);
    int pw_len = strlen(pw);

    int packet_size = 3 + id_len + pw_len;
    unsigned char* p = new unsigned char[packet_size];

    p[0] = 0x01;
    p[1] = (unsigned char)id_len;
    memcpy(p + 2, id, id_len);
    p[id_len + 2] = (unsigned char)pw_len;
    memcpy(p + id_len + 3, pw, pw_len);

    packet.packet = p;
    packet.size = packet_size;

    return packet;
}

bool Client::validate_auth_response(const Packet* packet) {
    return packet->packet[0] == 0x01 && packet->packet[1] == 0x00;
}


Packet Client::make_connection(const char* ip, const int& port) {
    Packet packet = {0, nullptr};
    int packet_size = 10;
    unsigned char* p = new unsigned char[packet_size];

    p[0] = 0x05;
    p[1] = 0x01;
    p[2] = 0x00;
    p[3] = 0x01;

    unsigned long network_host;
    if (inet_pton(AF_INET, ip, &network_host) != 1) {
        return packet;
    }
    memcpy(p + 4, &network_host, 4);

    int network_port = htons(port);
    memcpy(p + 8, &network_port, 2);

    packet.size = packet_size;
    packet.packet = p;

    return packet;
}

bool Client::validate_connection_response(const Packet* packet) {
    return packet->packet[0] == 5 && packet->packet[1] == 0;
}

Packet Client::get_packet(const int& len, const float& timeout_ms) {
    Packet packet;
    packet.size = len;
    packet.packet = new unsigned char[len];

    std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();

    while (std::chrono::duration<float>(std::chrono::steady_clock::now() - start).count() < timeout_ms / 1000.f) {
        int bytes_received = recv(this->proxy_server, (char*)packet.packet, len, 0);

        if (bytes_received == len) {
            return packet;

        } else if (bytes_received == 0) {
            printf("The connection with proxy server has been closed.\n");
            delete[] packet.packet;
            return packet;

        } else if (WSAGetLastError() != WSAEWOULDBLOCK) {
            delete[] packet.packet;
            return packet;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }

    delete[] packet.packet;
    return packet;
}

bool Client::send_packet(const Packet* packet) {
    int bytes_sent = send(this->proxy_server, (const char*)packet->packet, packet->size, 0);
    return bytes_sent == packet->size;
}