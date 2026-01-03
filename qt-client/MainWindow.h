#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStackedWidget>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDialog>
#include <QLabel>
#include <QTimer>
#include <QElapsedTimer>
#include "GameClient.h"

// Forward declarations
class ConnectionWidget;
class LoginWidget;
class LobbyWidget;
class GameWidget;

class MainWindow : public QMainWindow {
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    
private slots:
    // Connection screen
    void onConnectToServer(QString host, int port);
    
    // Client signals
    void onConnected();
    void onDisconnected();
    void onMessageReceived(QString message);
    void onErrorOccurred(QString error);
    
    // Login screen
    void onLoginClicked(QString username, QString password);
    void onRegisterClicked(QString username, QString password);
    
    // Lobby screen
    void onFindMatchClicked();
    void onCancelMatchClicked();
    void onViewLeaderboardClicked();
    void onViewHistoryClicked();
    void onViewPlayersClicked();
    void onChallengePlayerClicked(const QString& targetUsername);
    void onLogoutClicked();
    
    // Game screen
    void onMoveClicked(int row, int col);
    void onSurrenderClicked();
    void onOfferDrawClicked();
    void onShipsPlaced(const QVector<QJsonObject>& ships);
    void onChatMessageSent(const QString& message);
    
    // Ping
    void sendPing();
    
private:
    void setupUI();
    void handleMessage(const QJsonObject& msg);
    void showScreen(int index);
    void showGameEndDialog(const QString& result, const QString& reason, int newElo);
    void showChallengeDialog(const QString& challenger);
    
    GameClient* gameClient;
    QStackedWidget* stackedWidget;
    
    ConnectionWidget* connectionWidget;
    LoginWidget* loginWidget;
    LobbyWidget* lobbyWidget;
    GameWidget* gameWidget;
    
    // User data
    int userId;
    QString username;
    int elo;
    
    // Match dialog
    QDialog* matchDialog;
    QLabel* matchStatusLabel;
    QString currentOpponent;
    int currentOpponentElo;
    bool opponentReady;
    
    // Last opponent for rematch
    QString lastOpponent;
    
    // Ping measurement
    QTimer* pingTimer;
    QElapsedTimer pingElapsed;
    int currentPing;
};

#endif // MAINWINDOW_H
