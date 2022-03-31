#include <fmt/core.h>
#include <iostream>
#include <string>
#include <thread>

#include "client.hpp"

void send_loop(Client& client) {
    std::string message;
    ssize_t bytes_written = 0;

    while (bytes_written != -1) {
        std::getline(std::cin, message);
        auto sep = message.find_first_of(' ');
        if (message == "stop") {
            client.send(message);
            break;
        } else if (message.substr(0, sep) == "username") {
            client.set_username(message.substr(sep + 1));
        } else {
            bytes_written = client.send(message);
        }
    }
}

int main(int argc, char** argv) {
    if (argc < 2) {
        fmt::print("Usage : {} <adresse>\n", argv[0]);
        return 0;
    }

    Client client(argv[1]);

    if (!client.is_connected()) {
        return -1;
    }

    std::thread thr(&send_loop, std::ref(client));

    std::string buffer;
    std::string_view message;
    std::string_view sender;

    while (message != "stop" && client.receive(buffer)) {
        message = std::string_view(buffer);

        auto sep = message.find_first_of(' ');
        if (sep != std::string_view::npos) {
            fmt::print("{}: {}\n", message.substr(0, sep), message.substr(sep + 1));
        } else {
            fmt::print("Anonyme: {}\n", message);
        }
    }

    thr.join();
    client.disconnect();

    return 0;
}
