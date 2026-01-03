#ifndef LOBBYWIDGET_H
#define LOBBYWIDGET_H

#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QTableWidget>
#include <QJsonObject>
#include <QTimer>
#include <QElapsedTimer>

class LobbyWidget : public QWidget {
    Q_OBJECT
    
public:
    explicit LobbyWidget(QWidget *parent = nullptr);
    
    void setUserInfo(const QString& username, int elo);
    void updateLeaderboard(const QJsonObject& data);
    void updateMatchHistory(const QJsonObject& data);
    void updatePlayerList(const QJsonObject& data);
    void showWaitingMessage(const QString& message);
    void startMatchingTimer();
    void stopMatchingTimer();
    void updatePing(int ping);
    
signals:
    void findMatchClicked();
    void cancelMatchClicked();
    void viewLeaderboardClicked();
    void viewHistoryClicked();
    void viewPlayersClicked();
    void challengePlayerClicked(const QString& username);
    void logoutClicked();
    
private slots:
    void updateMatchingTimer();
    void onChallengeClicked();
    
private:
    void setupUI();
    
    QLabel* welcomeLabel;
    QLabel* eloLabel;
    QLabel* pingLabel;
    QLabel* statusLabel;
    QPushButton* findMatchButton;
    QPushButton* cancelMatchButton;
    QPushButton* leaderboardButton;
    QPushButton* historyButton;
    QPushButton* playersButton;
    QPushButton* refreshPlayersButton;
    QPushButton* logoutButton;
    QTableWidget* dataTable;
    
    QTimer* matchingTimer;
    QElapsedTimer matchingElapsed;
    bool isMatching;
    QString currentUsername;
    
    enum TableMode {
        MODE_LEADERBOARD,
        MODE_HISTORY,
        MODE_PLAYERS
    };
    TableMode currentMode;
};

#endif // LOBBYWIDGET_H
