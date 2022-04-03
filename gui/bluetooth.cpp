#include "bluetooth.h"
#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>

int createSocket(const QString& server_address, int channel) {
    int sock = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);
    if (sock == -1) {
        return -1;
    }

    sockaddr_rc addr = {0};
    addr.rc_family = AF_BLUETOOTH;
    addr.rc_channel = channel;
    str2ba(server_address.toStdString().c_str(), &addr.rc_bdaddr);

    if (::connect(sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        close(sock);
        return -1;
    }

    return sock;
}
