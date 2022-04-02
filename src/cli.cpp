#include "client.hpp"
#include <fmt/color.h>
#include <fmt/core.h>
#include <iostream>
#include <string>
#include <thread>

#include <poll.h>
#include <sys/eventfd.h>
#include <unistd.h>

// permet de notifier le thread qui attend l'input sur stdin
int event_fd = eventfd(0, 0);

// bloque jusqu'a ce qu'il y ait des données dispo sur stdin
// renvoie true si des données sont dispo, false si event_fd a ete notifie
bool wait_stdin() {
    static pollfd pfds[2] = {
        {fileno(stdin), POLLIN, 0},
        {event_fd, POLLIN, 0}
    };

    poll(pfds, 2, -1);

    return pfds[0].revents & POLLIN;
}

void send_loop(Client& client) {
    std::string message;
    ssize_t bytes_written = 0;

    while (bytes_written != -1 && wait_stdin()) {
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
    Client client;
    client.set_log(stderr);

    switch (argc) {
    case 1: // ./client
        client.auto_connect();
        break;
    case 2: // ./client "address"
        client.connect(argv[1]);
        break;
    default: // ./client "address" port
        client.connect(argv[1], std::atoi(argv[2]));
        break;
    }

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
            sender = message.substr(0, sep);
            message = message.substr(sep + 1);
        } else {
            sender = "Anonyme";
        }

        fmt::print(fmt::emphasis::bold, "{} : ", sender);
        fmt::print("{}\n", message);
    }

    write(event_fd, "00000000", 8);
    thr.join();
    client.disconnect();
    close(event_fd);

    return 0;
}
