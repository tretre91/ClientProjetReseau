#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string_view>
#include <string>
#include <atomic>

class Client {
public:
    Client(std::string_view server_address);

    ~Client();

    bool is_connected() const {
        return m_socket != -1;
    }

    bool receive(std::string& buffer);

    bool send(std::string_view message);

    void disconnect();

    void set_username(std::string_view name) {
        m_username = name;
    }

    std::string_view get_username() const {
        return m_username;
    }

private:
    static constexpr size_t buffer_size = 1024;
    std::atomic<int> m_socket = -1;
    std::string m_username;
    std::string send_buffer;
};

#endif // !CLIENT_HPP
