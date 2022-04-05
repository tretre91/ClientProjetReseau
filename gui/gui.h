#ifndef CLIENT_H
#define CLIENT_H

#include <QBluetoothServiceDiscoveryAgent>
#include <QBluetoothServiceInfo>
#include <QBluetoothUuid>
#include <QMainWindow>
#include <QTableWidget>
#include <QThread>

#include "../src/client.hpp"

QT_BEGIN_NAMESPACE
namespace Ui { class ClientGui; }
QT_END_NAMESPACE

/**
 * @brief Classe principale de l'interface graphique
 */
class ClientGui : public QMainWindow
{
    Q_OBJECT

public:
    ClientGui(QWidget *parent = nullptr);
    ~ClientGui();

    /**
     * @brief Démarre la recherche de serveurs
     */
    void startServiceDiscovery();

private:
    class Receiver;
    static const QBluetoothUuid server_uuid;
    static const QString messageFormat;

    Ui::ClientGui *ui;
    QBluetoothServiceDiscoveryAgent* m_discoveryAgent;
    Client m_client;

    /**
     * @brief Modifie le nom d'utilisateur
     */
    void setUsername(const QString& username);

    static QString createMessage(QString sender, QString message) {
        return messageFormat.arg(sender.toHtmlEscaped(), message.toHtmlEscaped());
    }

signals:
    /**
     * @brief Signal émis lorsque des données doivent être envoyées
     */
    void sendData(QString);

private slots:
    /**
     * @brief Fonction appelée lorsqu'un service à été trouvé sur un appareil à proximité
     */
    void serviceDiscovered(const QBluetoothServiceInfo& service);

    /**
     * @brief Fonction appelée lorsque la recherche de services est terminée
     */
    void discoveryFinished();

    /**
     * @brief Envoie un message vers le serveur
     */
    void send(QString);

    /**
     * @brief Reçoit un message du serveur
     */
    void receive(QString, QString);

    void connected();
    void disconnected();

    void sendPressed();
    void refreshPressed();
    void connectPressed();
    void disconnectPressed();

    void serverSelectionChanged(QTableWidgetItem* current, QTableWidgetItem* previous);
};

/**
 * @brief Classe utilisée pour attendre des données depuis le serveur
 */
class ClientGui::Receiver : public QThread {
    Q_OBJECT
public:
    Receiver(QObject* parent) : QThread(parent) {}

    void setClient(Client* client) { m_client = client; }

    void run() override {
        QString sender, message;
        std::string buffer;
        while (m_client->receive(buffer)) {
            auto sep = buffer.find_first_of(' ');
            if (sep != std::string::npos) {
                sender = QString::fromStdString(buffer.substr(0, sep));
                message = QString::fromStdString(buffer.substr(sep + 1));
            } else {
                sender = "Anonyme";
                message = QString::fromStdString(buffer);
            }
            emit receivedData(std::move(sender), std::move(message));
        }
    }

signals:
    /**
     * @brief Signal indiquant que des données ont été reçues
     */
    void receivedData(QString, QString);
private:
    Client* m_client = nullptr;
};

#endif // CLIENT_H
