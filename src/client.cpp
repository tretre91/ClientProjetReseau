#include "client.hpp"

#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>
#include <bluetooth/rfcomm.h>
#include <bluetooth/sdp.h>
#include <bluetooth/sdp_lib.h>

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

bool Client::connect(std::string_view server_address, uint8_t channel) {
    disconnect();
    m_socket = create_socket(server_address, channel);
    return is_connected();
}

bool Client::auto_connect() {
    std::vector<Server> servers = search_servers(1);
    if (!servers.empty()) {
        return connect(servers[0]);
    }
    return false;
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

int Client::create_socket(std::string_view server_address, uint8_t channel) {
    int sock = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);
    if (sock == -1) {
        fmt::print(m_log, "Echec de la creation du socket\n");
        return -1;
    }

    sockaddr_rc addr = {0};
    addr.rc_family = AF_BLUETOOTH;
    addr.rc_channel = channel;
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

std::vector<Server> Client::search_servers(unsigned int limit) {
    static const bdaddr_t bdaddr_any = {{0, 0, 0, 0, 0, 0}};

    int dev_id = hci_get_route(nullptr);
    int sock = hci_open_dev(dev_id);
    if (dev_id < 0 || sock < 0) {
        fmt::print(m_log, "Echec de l'ouverture du socket (sdp)\n");
        return {};
    }

    int max_rsp = 255;
    inquiry_info* inquiries = new inquiry_info[max_rsp];

    // device id, timeout (* 1.28), nb max de resulats
    int num_rsp = hci_inquiry(dev_id, 8, max_rsp, nullptr, &inquiries, IREQ_CACHE_FLUSH);
    if (num_rsp < 0) {
        fmt::print(m_log, "Echec de la requete sdp ({})\n", strerror(errno));
        delete inquiries;
        close(sock);
        return {};
    }

    uuid_t service_uuid;
    sdp_uuid128_create(&service_uuid, &server_uuid);

    bdaddr_t* target;
    uint32_t range = 0x0000ffff;
    sdp_list_t* search_list = sdp_list_append(0, &service_uuid);
    sdp_list_t* attrid_list = sdp_list_append(0, &range);

    sdp_session_t* session = nullptr;
    sdp_list_t* response_list = nullptr;
    uint8_t port = 0;
    char addr[19];
    char name[256];

    std::vector<Server> devices;

    for (int i = 0; i < num_rsp && devices.size() < limit; i++) {
        target = &(inquiries[i].bdaddr);

        // recuperation du nom et de l'adresse de l'appareil
        ba2str(target, addr);
        memset(name, 0, sizeof(name));
        if (hci_read_remote_name(sock, target, sizeof(name), name, 0) < 0) {
            fmt::print(m_log, "* {} (adresse : {})\n", "[inconnu]", addr);
        } else {
            fmt::print(m_log, "* {} (adresse : {})\n", name, addr);
        }

        // connexion au serveur sdp de l'appareil
        session = sdp_connect(&bdaddr_any, target, 0);
        if (session == nullptr) {
            fmt::print(m_log, "Echec de la connexion au serveur sdp distant ({})\n", strerror(errno));
            continue;
        }

        port = 0;
        response_list = 0;
        if (sdp_service_search_attr_req(session, search_list, SDP_ATTR_REQ_RANGE, attrid_list, &response_list) == 0) {
            sdp_list_t* proto_list;
            sdp_list_t* r = response_list;

            for (; r; r = r->next) {
                sdp_record_t* rec = (sdp_record_t*)r->data;
                if (sdp_get_access_protos(rec, &proto_list) == 0) {
                    port = sdp_get_proto_port(proto_list, RFCOMM_UUID);
                    fmt::print(m_log, "- canal {}\n", port);
                    sdp_list_free(proto_list, 0);
                }
                sdp_record_free(rec);
            }
        }

        sdp_list_free(response_list, 0);
        response_list = nullptr;
        sdp_close(session);

        if (port != 0) {
            Server serv;
            serv.name = name;
            serv.address = addr;
            serv.channel = port;
            devices.emplace_back(std::move(serv));
        }
    }

    sdp_list_free(search_list, 0);
    sdp_list_free(attrid_list, 0);

    delete inquiries;
    close(sock);

    return devices;
}
