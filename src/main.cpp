#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>
#include <cstdio>
#include <sys/socket.h>
#include <unistd.h>

#include <array>
#include <thread>
#include <fmt/core.h>
#include <iostream>

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

void send_loop(int socket) {
    std::string message;
    ssize_t bytes_written = 0;
    while (message != "stop" && bytes_written != -1) {
        std::getline(std::cin, message);
        bytes_written = send(socket, message.data(), message.size(), 0);
    }
}

int main(int argc, char** argv) {
    if (argc < 2) {
        fmt::print("Usage: {} <adresse>\n", argv[0]);
        return 0;
    }

    int sock = create_socket(argv[1]);

    if (sock == -1) {
        return -1;
    }

    std::thread thr(&send_loop, sock);

    ssize_t bytes_read = 0;
    std::array<char, 1024> buffer;
    std::string_view message;
    std::string_view sender;

    while (message != "stop") {
        bytes_read = recv(sock, buffer.data(), buffer.size(), 0);
        if (bytes_read == -1) {
            break;
        }
        
        message = std::string_view(buffer.data(), bytes_read);
        auto sep = message.find_first_of(' ');
        if (sep != std::string_view::npos) {
            fmt::print("{}: {}\n", message.substr(0, sep), message.substr(sep + 1));
        } else {
            fmt::print("Anonyme: {}\n", message);
        }
    }

    thr.join();
    close(sock);
    return 0;
}
