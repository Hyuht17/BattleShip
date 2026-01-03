#include "GameClient.h"
#include <QDebug>

GameClient::GameClient(QObject *parent)
    : QObject(parent)
{
    // Tạo C client
    client = client_create();
    if (!client) {
        qCritical() << "Failed to create C client";
    }
    
    // Timer để poll messages từ socket
    pollTimer = new QTimer(this);
    connect(pollTimer, &QTimer::timeout, this, &GameClient::checkForMessages);
}

GameClient::~GameClient() {
    if (pollTimer) {
        pollTimer->stop();
    }
    
    if (client) {
        client_destroy(client);
        client = nullptr;
    }
}

void GameClient::connectToServer(const QString& host, int port) {
    if (!client) {
        emit errorOccurred("Client not initialized");
        return;
    }
    
    qDebug() << "Connecting to" << host << ":" << port;
    
    int result = client_connect(client, host.toUtf8().constData(), port);
    
    if (result == 0) {
        // Set non-blocking mode
        client_set_nonblocking(client);
        
        qDebug() << "Connected successfully";
        emit connected();
        
        // Bắt đầu poll messages mỗi 100ms
        pollTimer->start(100);
    } else {
        QString error = QString("Failed to connect to %1:%2").arg(host).arg(port);
        qWarning() << error;
        emit errorOccurred(error);
    }
}

void GameClient::disconnectFromServer() {
    if (pollTimer) {
        pollTimer->stop();
    }
    
    if (client) {
        client_disconnect(client);
        qDebug() << "Disconnected from server";
        emit disconnected();
    }
}

void GameClient::sendMessage(const QString& message) {
    if (!client || !client->connected) {
        qWarning() << "Not connected, cannot send message";
        return;
    }
    
    qDebug() << "Sending:" << message;
    
    int result = client_send(client, message.toUtf8().constData());
    
    if (result < 0) {
        qWarning() << "Failed to send message";
        emit errorOccurred("Failed to send message");
    }
}

bool GameClient::isConnected() const {
    return client && client->connected;
}

void GameClient::checkForMessages() {
    if (!client || !client->connected) {
        return;
    }
    
    char buffer[8192];
    int received;
    
    // Đọc tất cả messages có sẵn
    while ((received = client_receive(client, buffer, sizeof(buffer))) > 0) {
        QString message = QString::fromUtf8(buffer, received);
        qDebug() << "Received:" << message;
        emit messageReceived(message);
    }
    
    if (received < 0) {
        // Connection lost
        qWarning() << "Connection lost";
        pollTimer->stop();
        emit disconnected();
    }
}
