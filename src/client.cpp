#include "client.hpp"

#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>
#include <cstdio>
#include <limits.h>
#include <sys/socket.h>
#include <unistd.h>

#include <fmt/core.h>

Client::Client() : m_username(get_hostname()) {
    m_log = fopen("/dev/null", "w");
}

Client::Client(std::string_view server_address) : Client() {
    connect(server_address);
}

Client::~Client() {
    close_log();
    if (m_socket != -1) {
        close(m_socket);
        m_socket = -1;
    }
}

bool Client::set_log(std::string_view filename) {
    close_log();
    m_log = fopen(filename.data(), "w");
    m_should_close_log = true;
    if (m_log == nullptr) {
        m_log = fopen("/dev/null", "w");
        return false;
    } else {
        return true;
    }
}

bool Client::set_log(FILE* f) {
    close_log();
    m_log = f;
    m_should_close_log = false;
    return true;
}

void Client::log(std::string_view s) {
    fmt::print(m_log, "{}\n", s);
}

void Client::close_log() {
    if (m_should_close_log) {
        fclose(m_log);
    }
}

bool Client::connect(std::string_view server_address) {
    disconnect();
    m_socket = create_socket(server_address);
    return is_connected();
}

bool Client::receive(std::string& buffer) {
    buffer.resize(buffer_size);
    ssize_t size = recv(m_socket, buffer.data(), buffer.size(), 0);
    if (size > 0) {
        buffer.resize(size);
        return true;
    } else if (size == 0) {
        fmt::print(m_log, "Fermeture du socket en lecture\n");
        return false;
    } else {
        fmt::print(m_log, "Echec lors de la reception d'un message\n");
        return false;
    }
}

bool Client::send(std::string_view message) {
    static std::string send_buffer;
    send_buffer = fmt::format("{} {}", m_username, message);
    ssize_t size = ::send(m_socket, send_buffer.data(), send_buffer.size(), 0);
    if (size < 0) {
        fmt::print(m_log, "Echec lors de l'envoi d'un message\n");
        return false;
    } else {
        return true;
    }
}

void Client::set_username(std::string_view name) {
    fmt::print(m_log, "Changement de nom d'utilisateur : '{}' -> '{}'\n", m_username, name);
    m_username = name;
}

void Client::disconnect() {
    if (is_connected()) {
        shutdown(m_socket, SHUT_RD);
        close(m_socket.exchange(-1));
    }
}

int Client::create_socket(std::string_view server_address) {
    int sock = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);
    if (sock == -1) {
        fmt::print(m_log, "Echec de la creation du socket\n");
        return -1;
    }

    sockaddr_rc addr = {0};
    addr.rc_family = AF_BLUETOOTH;
    addr.rc_channel = 1;
    str2ba(server_address.data(), &addr.rc_bdaddr);

    fmt::print(m_log, "Tentative de connexion au serveur (adresse : {}) ... ", server_address);
    fflush(m_log);

    if (::connect(sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        fmt::print(m_log, "Echec de la connexion\n");
        close(sock);
        return -1;
    }

    fmt::print(m_log, "Connecte\n");

    return sock;
}

std::string Client::get_hostname() const {
    char hostname[HOST_NAME_MAX + 1];
    if (gethostname(hostname, HOST_NAME_MAX + 1) != -1) {
        return hostname;
    } else {
        fmt::print(m_log, "Echec de l'obtention du nom d'hôte", errno);
        return "";
    }
}
