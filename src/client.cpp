#include "client.hpp"

#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>
#include <cstdio>
#include <limits.h>
#include <sys/socket.h>
#include <unistd.h>

#include <fmt/core.h>

int create_socket(std::string_view server_address) {
    int sock = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);
    if (sock == -1) {
        fmt::print("Echec de la creation du socket\n");
        return -1;
    }

    sockaddr_rc addr = {0};
    addr.rc_family = AF_BLUETOOTH;
    addr.rc_channel = 1;
    str2ba(server_address.data(), &addr.rc_bdaddr);

    fmt::print("Tentative de connexion au serveur (adresse : {}) ... ", server_address);
    fflush(stdout);

    if (connect(sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        fmt::print("Echec de la connexion\n");
        close(sock);
        return -1;
    }

    fmt::print("Connecte\n");

    return sock;
}

std::string get_hostname() {
    char hostname[HOST_NAME_MAX + 1];
    if (gethostname(hostname, HOST_NAME_MAX + 1) != -1) {
        return hostname;
    } else {
        fmt::print(stderr, "Failed to get the computer's name (error code {})\n", errno);
        return "";
    }
}

Client::Client(std::string_view server_address) : m_socket(create_socket(server_address)), m_username(get_hostname()) {}

Client::~Client() {
    if (m_socket != -1) {
        close(m_socket);
        m_socket = -1;
    }
}

bool Client::receive(std::string& buffer) {
    buffer.resize(buffer_size);
    ssize_t size = recv(m_socket, buffer.data(), buffer.size(), 0);
    if (size < 0) {
        fmt::print(stderr, "Echec lors de la reception d'un message\n");
        return false;
    } else {
        buffer.resize(size);
        return true;
    }
}

bool Client::send(std::string_view message) {
    send_buffer = fmt::format("{} {}", m_username, message);
    ssize_t size = ::send(m_socket, send_buffer.data(), send_buffer.size(), 0);
    if (size < 0) {
        fmt::print(stderr, "Echec lors de l'envoi d'un message\n");
        return false;
    } else {
        return true;
    }
}

void Client::disconnect() {
    if (is_connected()) {
        close(m_socket.exchange(-1));
    }
}
