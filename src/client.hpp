#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string_view>
#include <string>
#include <atomic>
#include <vector>

struct Server {
    std::string name;
    std::string address;
    uint8_t channel;
};

class Client {
public:
    Client();
    
    Client(std::string_view server_address);

    ~Client();

    bool set_log(std::string_view filename);

    bool set_log(FILE* f);

    void log(std::string_view s);

    std::vector<Server> search_servers(unsigned int limit = -1);

    bool connect(std::string_view server_address, uint8_t channel = 1);

    bool connect(const Server& server) { return connect(server.address, server.channel); }

    bool auto_connect();

    bool is_connected() const {
        return m_socket != -1;
    }

    bool receive(std::string& buffer);

    bool send(std::string_view message);

    void disconnect();

    void set_username(std::string_view name);

    std::string_view get_username() const {
        return m_username;
    }

private:
    static constexpr size_t buffer_size = 1024;
    static constexpr uint32_t server_uuid[4] = {0x98592148, 0xF9114837, 0x9132EF39, 0xF920A5B9};

    std::atomic<int> m_socket = -1;
    std::string m_username;
    bool m_should_close_log = true;
    FILE* m_log;

    void close_log();
    int create_socket(std::string_view address, uint8_t channel);
    std::string get_hostname() const;
};

#endif // !CLIENT_HPP
