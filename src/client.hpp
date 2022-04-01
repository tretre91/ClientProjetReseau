#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string_view>
#include <string>
#include <atomic>

class Client {
public:
    Client();
    
    Client(std::string_view server_address);

    ~Client();

    bool set_log(std::string_view filename);

    bool set_log(FILE* f);

    void log(std::string_view s);

    bool connect(std::string_view server_address);

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
    std::atomic<int> m_socket = -1;
    std::string m_username;
    std::string send_buffer;
    bool should_close_log = true;
    FILE* m_log;

    void close_log();
    int create_socket(std::string_view address);
    std::string get_hostname() const;
};

#endif // !CLIENT_HPP
