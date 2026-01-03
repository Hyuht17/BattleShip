#ifndef GAMECLIENT_H
#define GAMECLIENT_H

#include <QObject>
#include <QTimer>
#include <QString>

// Include C header
extern "C" {
#include "../client-lib/battleship_client.h"
}

class GameClient : public QObject {
    Q_OBJECT
    
public:
    explicit GameClient(QObject *parent = nullptr);
    ~GameClient();
    
    // Kết nối và ngắt kết nối
    void connectToServer(const QString& host, int port);
    void disconnectFromServer();
    
    // Gửi message (JSON string)
    void sendMessage(const QString& message);
    
    // Kiểm tra trạng thái kết nối
    bool isConnected() const;
    
signals:
    void connected();
    void disconnected();
    void messageReceived(QString message);
    void errorOccurred(QString error);
    
private slots:
    void checkForMessages();
    
private:
    BattleshipClient* client;  // C struct pointer
    QTimer* pollTimer;
};

#endif // GAMECLIENT_H
