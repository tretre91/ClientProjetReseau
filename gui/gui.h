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

class ClientGui : public QMainWindow
{
    Q_OBJECT

public:
    ClientGui(QWidget *parent = nullptr);
    ~ClientGui();

    void startServiceDiscovery();

private:
    class Receiver;
    static const QBluetoothUuid server_uuid;
    static const QString messageFormat;

    Ui::ClientGui *ui;
    QBluetoothServiceDiscoveryAgent* m_discoveryAgent;
    Client m_client;

    void setUsername(const QString& username);

    static QString createMessage(QString sender, QString message) {
        return messageFormat.arg(sender.toHtmlEscaped(), message.toHtmlEscaped());
    }

signals:
    void sendData(QString);

private slots:
    void serviceDiscovered(const QBluetoothServiceInfo& service);
    void discoveryFinished();

    void send(QString);
    void receive(QString, QString);

    void connected();
    void disconnected();

    void sendPressed();
    void refreshPressed();
    void connectPressed();
    void disconnectPressed();

    void serverSelectionChanged(QTableWidgetItem* current, QTableWidgetItem* previous);
};

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
    void receivedData(QString, QString);
private:
    Client* m_client = nullptr;
};

#endif // CLIENT_H
