#include "LobbyWidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QJsonArray>
#include <QDateTime>

LobbyWidget::LobbyWidget(QWidget *parent)
    : QWidget(parent), isMatching(false), currentMode(MODE_LEADERBOARD)
{
    matchingTimer = new QTimer(this);
    connect(matchingTimer, &QTimer::timeout, this, &LobbyWidget::updateMatchingTimer);
    setupUI();
}

void LobbyWidget::setupUI() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    
    // Welcome section
    QHBoxLayout* topLayout = new QHBoxLayout();
    
    welcomeLabel = new QLabel("Welcome!", this);
    QFont welcomeFont = welcomeLabel->font();
    welcomeFont.setPointSize(16);
    welcomeFont.setBold(true);
    welcomeLabel->setFont(welcomeFont);
    topLayout->addWidget(welcomeLabel);
    
    topLayout->addStretch();
    
    eloLabel = new QLabel("ELO: 1000", this);
    QFont eloFont = eloLabel->font();
    eloFont.setPointSize(14);
    eloLabel->setFont(eloFont);
    topLayout->addWidget(eloLabel);
    
    topLayout->addSpacing(20);
    
    pingLabel = new QLabel("Ping: --", this);
    QFont pingFont = pingLabel->font();
    pingFont.setPointSize(12);
    pingLabel->setFont(pingFont);
    pingLabel->setStyleSheet("color: #666;");
    topLayout->addWidget(pingLabel);
    
    mainLayout->addLayout(topLayout);
    
    // Status label
    statusLabel = new QLabel(this);
    statusLabel->setAlignment(Qt::AlignCenter);
    statusLabel->setStyleSheet("color: blue; font-size: 14px;");
    mainLayout->addWidget(statusLabel);
    
    // Buttons
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    
    findMatchButton = new QPushButton("Find Match", this);
    findMatchButton->setMinimumHeight(40);
    connect(findMatchButton, &QPushButton::clicked, this, &LobbyWidget::findMatchClicked);
    buttonLayout->addWidget(findMatchButton);
    
    cancelMatchButton = new QPushButton("Cancel", this);
    cancelMatchButton->setMinimumHeight(40);
    cancelMatchButton->setVisible(false);
    cancelMatchButton->setStyleSheet("background-color: #ff6666;");
    connect(cancelMatchButton, &QPushButton::clicked, this, &LobbyWidget::cancelMatchClicked);
    buttonLayout->addWidget(cancelMatchButton);
    
    leaderboardButton = new QPushButton("Leaderboard", this);
    leaderboardButton->setMinimumHeight(40);
    connect(leaderboardButton, &QPushButton::clicked, this, &LobbyWidget::viewLeaderboardClicked);
    buttonLayout->addWidget(leaderboardButton);
    
    historyButton = new QPushButton("Match History", this);
    historyButton->setMinimumHeight(40);
    connect(historyButton, &QPushButton::clicked, this, &LobbyWidget::viewHistoryClicked);
    buttonLayout->addWidget(historyButton);
    
    playersButton = new QPushButton("Online Players", this);
    playersButton->setMinimumHeight(40);
    connect(playersButton, &QPushButton::clicked, this, &LobbyWidget::viewPlayersClicked);
    buttonLayout->addWidget(playersButton);
    
    logoutButton = new QPushButton("Logout", this);
    logoutButton->setMinimumHeight(40);
    connect(logoutButton, &QPushButton::clicked, this, &LobbyWidget::logoutClicked);
    buttonLayout->addWidget(logoutButton);
    
    mainLayout->addLayout(buttonLayout);
    
    // Refresh players button (hidden by default)
    refreshPlayersButton = new QPushButton("Refresh Player List", this);
    refreshPlayersButton->setMinimumHeight(35);
    refreshPlayersButton->setStyleSheet("background-color: #4CAF50; color: white; font-weight: bold;");
    refreshPlayersButton->setVisible(false);
    connect(refreshPlayersButton, &QPushButton::clicked, this, &LobbyWidget::viewPlayersClicked);
    mainLayout->addLayout(buttonLayout);
    
    // Refresh players button (hidden by default)
    refreshPlayersButton = new QPushButton("Refresh Player List", this);
    refreshPlayersButton->setMinimumHeight(35);
    refreshPlayersButton->setStyleSheet("background-color: #4CAF50; color: white; font-weight: bold;");
    refreshPlayersButton->setVisible(false);
    connect(refreshPlayersButton, &QPushButton::clicked, this, &LobbyWidget::viewPlayersClicked);
    mainLayout->addWidget(refreshPlayersButton);
    
    // Data table
    dataTable = new QTableWidget(this);
    dataTable->setColumnCount(3);
    dataTable->setHorizontalHeaderLabels({"Rank", "Username", "ELO"});
    dataTable->horizontalHeader()->setStretchLastSection(true);
    dataTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    dataTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    mainLayout->addWidget(dataTable);
}

void LobbyWidget::setUserInfo(const QString& username, int elo) {
    currentUsername = username;
    welcomeLabel->setText("Welcome, " + username + "!");
    eloLabel->setText("ELO: " + QString::number(elo));
    statusLabel->clear();
}

void LobbyWidget::updateLeaderboard(const QJsonObject& data) {
    currentMode = MODE_LEADERBOARD;
    if (refreshPlayersButton) {
        refreshPlayersButton->setVisible(false);
    }
    statusLabel->clear();
    
    QJsonArray players = data["players"].toArray();
    
    dataTable->setRowCount(players.size());
    dataTable->setColumnCount(6);
    dataTable->setHorizontalHeaderLabels({"Rank", "Username", "ELO", "Games", "Wins", "Win Rate"});
    
    for (int i = 0; i < players.size(); ++i) {
        QJsonObject player = players[i].toObject();
        
        dataTable->setItem(i, 0, new QTableWidgetItem(QString::number(player["rank"].toInt())));
        dataTable->setItem(i, 1, new QTableWidgetItem(player["username"].toString()));
        dataTable->setItem(i, 2, new QTableWidgetItem(QString::number(player["elo"].toInt())));
        dataTable->setItem(i, 3, new QTableWidgetItem(QString::number(player["games"].toInt())));
        dataTable->setItem(i, 4, new QTableWidgetItem(QString::number(player["wins"].toInt())));
        dataTable->setItem(i, 5, new QTableWidgetItem(QString::number(player["winrate"].toDouble(), 'f', 1) + "%"));
    }
}

void LobbyWidget::updateMatchHistory(const QJsonObject& data) {
    currentMode = MODE_HISTORY;
    if (refreshPlayersButton) {
        refreshPlayersButton->setVisible(false);
    }
    statusLabel->clear();
    
    QJsonArray matches = data["matches"].toArray();
    
    dataTable->setRowCount(matches.size());
    dataTable->setColumnCount(3);
    dataTable->setHorizontalHeaderLabels({"Date", "Opponent", "Result"});
    
    for (int i = 0; i < matches.size(); ++i) {
        QJsonObject match = matches[i].toObject();
        
        // Convert timestamp to date string
        qint64 timestamp = match["timestamp"].toVariant().toLongLong();
        QDateTime dateTime = QDateTime::fromSecsSinceEpoch(timestamp);
        QString dateStr = dateTime.toString("yyyy-MM-dd HH:mm");
        
        dataTable->setItem(i, 0, new QTableWidgetItem(dateStr));
        dataTable->setItem(i, 1, new QTableWidgetItem(match["opponent"].toString()));
        dataTable->setItem(i, 2, new QTableWidgetItem(match["result"].toString()));
    }
}

void LobbyWidget::showWaitingMessage(const QString& message) {
    statusLabel->setText(message);
}

void LobbyWidget::updatePlayerList(const QJsonObject& response) {
    currentMode = MODE_PLAYERS;
    
    // Show refresh button
    if (refreshPlayersButton) {
        refreshPlayersButton->setVisible(true);
    }
    
    // Configure table for player list
    dataTable->clear();
    dataTable->setColumnCount(4);
    dataTable->setHorizontalHeaderLabels({"Username", "ELO", "Status", "Action"});
    
    QJsonArray players = response["players"].toArray();
    
    // Filter out current user and count valid players
    QVector<QJsonObject> validPlayers;
    for (int i = 0; i < players.size(); i++) {
        QJsonObject player = players[i].toObject();
        QString username = player["username"].toString();
        if (username != currentUsername) {
            validPlayers.append(player);
        }
    }
    
    dataTable->setRowCount(validPlayers.size());
    
    for (int i = 0; i < validPlayers.size(); i++) {
        QJsonObject player = validPlayers[i];
        QString username = player["username"].toString();
        int elo = player["elo"].toInt();
        int status = player["status"].toInt();
        
        // Username column
        dataTable->setItem(i, 0, new QTableWidgetItem(username));
        
        // ELO column
        dataTable->setItem(i, 1, new QTableWidgetItem(QString::number(elo)));
        
        // Status column
        // Server status: 0=OFFLINE, 1=ONLINE, 2=IN_LOBBY (Matching), 3=IN_GAME
        QString statusText;
        if (status == 1) {
            statusText = "Online";
        } else if (status == 2) {
            statusText = "Matching";
        } else if (status == 3) {
            statusText = "In Game";
        } else {
            statusText = "Offline";
        }
        dataTable->setItem(i, 2, new QTableWidgetItem(statusText));
        
        // Action column - Challenge button
        QPushButton* challengeBtn = new QPushButton("Challenge");
        challengeBtn->setMinimumHeight(30);
        challengeBtn->setProperty("playerUsername", username);
        
        // Enable only if player is ONLINE (status == 1)
        if (status != 1) {
            challengeBtn->setEnabled(false);
        }
        
        connect(challengeBtn, &QPushButton::clicked, this, &LobbyWidget::onChallengeClicked);
        dataTable->setCellWidget(i, 3, challengeBtn);
    }
    
    // Adjust column widths
    dataTable->setColumnWidth(0, 150);
    dataTable->setColumnWidth(1, 80);
    dataTable->setColumnWidth(2, 100);
    dataTable->setColumnWidth(3, 120);
}

void LobbyWidget::onChallengeClicked() {
    QPushButton* button = qobject_cast<QPushButton*>(sender());
    if (!button) return;
    
    QString username = button->property("playerUsername").toString();
    if (!username.isEmpty()) {
        statusLabel->setText("Challenge sent to " + username + "...");
        emit challengePlayerClicked(username);
    }
}

void LobbyWidget::startMatchingTimer() {
    isMatching = true;
    matchingElapsed.start();
    matchingTimer->start(100); // Update every 100ms
    findMatchButton->setVisible(false);
    cancelMatchButton->setVisible(true);
    leaderboardButton->setEnabled(false);
    historyButton->setEnabled(false);
    playersButton->setEnabled(false);
}

void LobbyWidget::stopMatchingTimer() {
    isMatching = false;
    matchingTimer->stop();
    findMatchButton->setVisible(true);
    cancelMatchButton->setVisible(false);
    leaderboardButton->setEnabled(true);
    historyButton->setEnabled(true);
    playersButton->setEnabled(true);
    statusLabel->clear();
}

void LobbyWidget::updateMatchingTimer() {
    if (isMatching) {
        int seconds = matchingElapsed.elapsed() / 1000;
        statusLabel->setText(QString("Finding match... %1s").arg(seconds));
    }
}

void LobbyWidget::updatePing(int ping) {
    QString color;
    if (ping < 50) {
        color = "#00cc00"; // Green
    } else if (ping < 100) {
        color = "#ffaa00"; // Yellow/Orange
    } else {
        color = "#ff0000"; // Red
    }
    
    pingLabel->setText(QString("Ping: %1ms").arg(ping));
    pingLabel->setStyleSheet(QString("color: %1;").arg(color));
}
