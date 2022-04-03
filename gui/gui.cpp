#include "gui.h"
#include "./ui_gui.h"

#include <QBluetoothServiceDiscoveryAgent>

const QBluetoothUuid ClientGui::server_uuid{QString("{98592148-f911-4837-9132-ef39f920a5b9}")};

const QString ClientGui::messageFormat("<div class=\"message\"><b>%1 :</b><br>%2</div><br>");

ClientGui::ClientGui(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::ClientGui)
{
    ui->setupUi(this);
    std::string_view name = m_client.get_username();
    ui->usernameLabel->setText(QString::fromUtf8(name.data(), name.size()));

    m_discoveryAgent = new QBluetoothServiceDiscoveryAgent(this);
    m_discoveryAgent->setUuidFilter(server_uuid);

    connect(ui->serversView, SIGNAL(currentItemChanged(QTableWidgetItem*,QTableWidgetItem*)),
            this, SLOT(serverSelectionChanged(QTableWidgetItem*,QTableWidgetItem*)));

    connect(ui->sendButton, SIGNAL(released()), this, SLOT(sendPressed()));
    connect(ui->connectButton, SIGNAL(released()), this, SLOT(connectPressed()));
    connect(ui->disconnectButton, SIGNAL(released()), this, SLOT(disconnectPressed()));
    connect(ui->refreshButton, SIGNAL(released()), this, SLOT(refreshPressed()));

    connect(m_discoveryAgent, SIGNAL(serviceDiscovered(QBluetoothServiceInfo)),
            this, SLOT(serviceDiscovered(QBluetoothServiceInfo)));

    connect(m_discoveryAgent, SIGNAL(finished()), this, SLOT(discoveryFinished()));

    connect(this, SIGNAL(sendData(QString)), this, SLOT(send(QString)));
}

ClientGui::~ClientGui()
{
    delete ui;
    delete m_discoveryAgent;
}

void ClientGui::startServiceDiscovery() {
    qDebug() << "Discovery started";
    m_discoveryAgent->start(QBluetoothServiceDiscoveryAgent::DiscoveryMode::FullDiscovery);
}

void ClientGui::setUsername(const QString& username)
{
    m_client.set_username(username.toStdString());
    ui->usernameLabel->setText(username);
}

void ClientGui::serviceDiscovered(const QBluetoothServiceInfo &service)
{
    qDebug() << "UUID = " << service.serviceUuid().toString() << " Name = " << service.device().name() << "; " << service.serviceName();

    auto name = service.device().name();
    auto address = service.device().address();
    int channel = service.serverChannel();

    auto* nameItem = new QTableWidgetItem(name);
    auto* addressItem = new QTableWidgetItem(address.toString());
    auto* channelItem = new QTableWidgetItem(QString::number(channel));

    const int row = ui->serversView->rowCount();
    ui->serversView->setRowCount(row + 1);
    ui->serversView->setItem(row, 0, nameItem);
    ui->serversView->setItem(row, 1, addressItem);
    ui->serversView->setItem(row, 2, channelItem);
}

void ClientGui::discoveryFinished()
{
    qDebug() << "Discovery finished\n";
}

void ClientGui::send(QString s)
{
    m_client.send(s.toStdString());
}

void ClientGui::receive(QString sender, QString message)
{
    ui->messages->insertHtml(createMessage(sender, message));
}

void ClientGui::connected()
{
    qDebug() << "Client is connected\n";
}

void ClientGui::disconnected()
{
    qDebug() << "Client is disconnected\n";
}

void ClientGui::sendPressed()
{
    QString message = ui->messageBox->toPlainText();
    if (message.startsWith("username")) {
        auto sep = message.indexOf(' ');
        if (sep != -1) {
            message.remove(0, sep);
            setUsername(message);
        }
    } else {
        emit sendData(message);
        receive(ui->usernameLabel->text(), message);
        ui->messageBox->clear();
    }
}

void ClientGui::refreshPressed() {
    ui->serversView->clearContents();
    startServiceDiscovery();
}

void ClientGui::connectPressed()
{
    QString server_address = ui->serverInput->text();
    int channel = ui->portInput->value();

    m_client.disconnect();
    if (m_client.connect(server_address.toStdString(), channel)) {
        auto* receiver = new Receiver(this);
        receiver->setClient(&m_client);
        connect(receiver, SIGNAL(receivedData(QString,QString)), this, SLOT(receive(QString,QString)));
        connect(receiver, SIGNAL(finished()), receiver, SLOT(deleteLater()));
        receiver->start();
    } else {
        qDebug() << "Echec de la connexion\n";
    }
}

void ClientGui::disconnectPressed()
{
    m_client.disconnect();
}

void ClientGui::serverSelectionChanged(QTableWidgetItem *current, QTableWidgetItem *previous)
{
    QString address = ui->serversView->item(current->row(), 1)->text();
    QString port = ui->serversView->item(current->row(), 2)->text();
    ui->serverInput->setText(address);
    ui->portInput->setValue(port.toInt());
}
