#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <atomic>
#include <string>
#include <string_view>
#include <vector>

struct Server {
    std::string name;
    std::string address;
    uint8_t channel;
};

/**
 * @brief Classe permettant de gèrer la connexion avec un serveur
 */
class Client
{
public:
    Client();

    Client(std::string_view server_address);

    ~Client();

    bool set_log(std::string_view filename);

    bool set_log(FILE* f);

    void log(std::string_view s);

    /**
     * @brief Cherche des serveurs disponibles
     * @param limit Le nombre maximal de serveurs à chercher
     * @return La liste des serveurs trouvés
     */
    std::vector<Server> search_servers(unsigned int limit = -1);

    /**
     * @brief Essaie de se connecter à un serveur
     * @param server_address L'adresse bluetooth du serveur
     * @param channel Le canal RFCOMM à utiliser (par défaut 1)
     * @return true si la connexion a réussi, false sinon
     */
    bool connect(std::string_view server_address, uint8_t channel = 1);

    /**
     * @brief Connection à un serveur après recherche automatique
     * @param server Le résultat de la recherche
     * @return true si la connexion a réussi, false sinon
     */
    bool connect(const Server& server) {
        return connect(server.address, server.channel);
    }

    /**
     * @brief Cherche le premier serveur disponible et se connecte
     * @return true si la connexion a réussi, false sinon
     */
    bool auto_connect();

    bool is_connected() const {
        return m_socket != -1;
    }

    /**
     * @brief Recoit des données depuis le serveur
     * @param buffer Le buffer où les données seront écrites
     * @return false en cas d'erreur, true sinon
     */
    bool receive(std::string& buffer);

    /**
     * @brief Envoie un message au serveur
     * @return false en cas d'erreur, true sinon
     */
    bool send(std::string_view message);

    void disconnect();

    /**
     * @brief Modifie le nom d'utilisateur
     */
    void set_username(std::string_view name);

    std::string_view get_username() const {
        return m_username;
    }

private:
    static constexpr size_t buffer_size = 1024;
    static constexpr uint8_t server_uuid[16] = {0x98, 0x59, 0x21, 0x48, 0xF9, 0x11, 0x48, 0x37, 0x91, 0x32, 0xEF, 0x39, 0xF9, 0x20, 0xA5, 0xB9};

    std::atomic<int> m_socket = -1;
    std::string m_username;
    bool m_should_close_log = true;
    FILE* m_log;

    void close_log();

    /**
     * @brief Crée un socket bluetooth
     * @param address L'adresse à laquelle se connecter
     * @param channel Le canal à utiliser
     * @return int le descripteur du socket, ou -1 en cas d'erreur
     */
    int create_socket(std::string_view address, uint8_t channel);

    /**
     * @brief renvoie le nom d'hôte de cet ordinateur
     */
    std::string get_hostname() const;
};

#endif // !CLIENT_HPP
