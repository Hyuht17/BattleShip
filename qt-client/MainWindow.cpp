#include "MainWindow.h"
#include "ConnectionWidget.h"
#include "LoginWidget.h"
#include "LobbyWidget.h"
#include "GameWidget.h"
#include <QMessageBox>
#include <QDebug>
#include <QJsonArray>
#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , userId(-1)
    , elo(1000)
    , matchDialog(nullptr)
    , opponentReady(false)
    , currentPing(0)
{
    setupUI();
    
    // Tạo game client
    gameClient = new GameClient(this);
    
    // Connect client signals
    connect(gameClient, &GameClient::connected, this, &MainWindow::onConnected);
    connect(gameClient, &GameClient::disconnected, this, &MainWindow::onDisconnected);
    connect(gameClient, &GameClient::messageReceived, this, &MainWindow::onMessageReceived);
    connect(gameClient, &GameClient::errorOccurred, this, &MainWindow::onErrorOccurred);
    
    // Setup ping timer
    pingTimer = new QTimer(this);
    connect(pingTimer, &QTimer::timeout, this, &MainWindow::sendPing);
    pingTimer->setInterval(1000); // Send ping every 1 second for better accuracy
    
    // KHÔNG auto connect nữa - đợi user nhập server address
}

MainWindow::~MainWindow() {
}

void MainWindow::setupUI() {
    setWindowTitle("Battleship Game");
    resize(800, 600);
    
    // Stacked widget để chuyển giữa các màn hình
    stackedWidget = new QStackedWidget(this);
    setCentralWidget(stackedWidget);
    
    // Tạo các màn hình
    connectionWidget = new ConnectionWidget(this);
    loginWidget = new LoginWidget(this);
    lobbyWidget = new LobbyWidget(this);
    gameWidget = new GameWidget(this);
    
    stackedWidget->addWidget(connectionWidget); // index 0
    stackedWidget->addWidget(loginWidget);      // index 1
    stackedWidget->addWidget(lobbyWidget);      // index 2
    stackedWidget->addWidget(gameWidget);       // index 3
    
    // Connect connection widget signals
    connect(connectionWidget, &ConnectionWidget::connectClicked, this, &MainWindow::onConnectToServer);
    
    // Connect login widget signals
    connect(loginWidget, &LoginWidget::loginClicked, this, &MainWindow::onLoginClicked);
    connect(loginWidget, &LoginWidget::registerClicked, this, &MainWindow::onRegisterClicked);
    
    // Connect lobby widget signals
    connect(lobbyWidget, &LobbyWidget::findMatchClicked, this, &MainWindow::onFindMatchClicked);
    connect(lobbyWidget, &LobbyWidget::cancelMatchClicked, this, &MainWindow::onCancelMatchClicked);
    connect(lobbyWidget, &LobbyWidget::viewLeaderboardClicked, this, &MainWindow::onViewLeaderboardClicked);
    connect(lobbyWidget, &LobbyWidget::viewHistoryClicked, this, &MainWindow::onViewHistoryClicked);
    connect(lobbyWidget, &LobbyWidget::viewPlayersClicked, this, &MainWindow::onViewPlayersClicked);
    connect(lobbyWidget, &LobbyWidget::challengePlayerClicked, this, &MainWindow::onChallengePlayerClicked);
    connect(lobbyWidget, &LobbyWidget::logoutClicked, this, &MainWindow::onLogoutClicked);
    
    // Connect game widget signals
    connect(gameWidget, &GameWidget::moveClicked, this, &MainWindow::onMoveClicked);
    connect(gameWidget, &GameWidget::surrenderClicked, this, &MainWindow::onSurrenderClicked);
    connect(gameWidget, &GameWidget::offerDrawClicked, this, &MainWindow::onOfferDrawClicked);
    connect(gameWidget, &GameWidget::shipsPlaced, this, &MainWindow::onShipsPlaced);
    connect(gameWidget, &GameWidget::chatMessageSent, this, &MainWindow::onChatMessageSent);
    
    showScreen(0); // Start with connection screen
    loginWidget->setEnabled(false); // Disable until connected
}

void MainWindow::onConnectToServer(QString host, int port) {
    qDebug() << "Connecting to" << host << ":" << port;
    gameClient->connectToServer(host, port);
}

void MainWindow::onConnected() {
    qDebug() << "Connected to server";
    loginWidget->setEnabled(true);
    showScreen(1); // Go to login screen
    
    // Start ping timer when connected
    pingTimer->start();
}

void MainWindow::onDisconnected() {
    qDebug() << "Disconnected from server";
    
    // Stop ping timer when disconnected
    pingTimer->stop();
    
    QMessageBox::warning(this, "Connection Lost", "Disconnected from server");
    showScreen(0); // Back to connection screen
    loginWidget->setEnabled(false);
}

void MainWindow::onMessageReceived(QString message) {
    // Parse JSON
    QJsonDocument doc = QJsonDocument::fromJson(message.toUtf8());
    if (doc.isNull() || !doc.isObject()) {
        qWarning() << "Invalid JSON:" << message;
        return;
    }
    
    handleMessage(doc.object());
}

void MainWindow::handleMessage(const QJsonObject& msg) {
    // Server uses "cmd" field with "payload", client uses "type"
    QString cmd = msg["cmd"].toString();
    QString type = msg["type"].toString();
    QString messageType = cmd.isEmpty() ? type : cmd;
    
    if (messageType.isEmpty()) {
        qDebug() << "Empty message type, ignoring";
        return;
    }
    
    if (messageType == "WELCOME") {
        qDebug() << "Connected to server";
        return;
    }
    
    if (messageType == "LOGIN_SUCCESS") {
        QJsonObject payload = msg["payload"].toObject();
        username = payload["username"].toString();
        elo = payload["elo"].toInt();
        
        qDebug() << "Login successful:" << username << "ELO:" << elo;
        lobbyWidget->setUserInfo(username, elo);
        showScreen(2); // Go to lobby
        
    } else if (messageType == "LOGIN_FAILED") {
        QJsonObject payload = msg["payload"].toObject();
        QString reason = payload["message"].toString();
        if (reason.isEmpty()) reason = msg["message"].toString();
        QMessageBox::warning(this, "Login Failed", reason);
        
    } else if (messageType == "REGISTER_SUCCESS") {
        QMessageBox::information(this, "Success", "Registration successful! Please login.");
        
    } else if (messageType == "REGISTER_FAILED") {
        QJsonObject payload = msg["payload"].toObject();
        QString reason = payload["message"].toString();
        if (reason.isEmpty()) reason = msg["message"].toString();
        QMessageBox::warning(this, "Registration Failed", reason);
        
    } else if (messageType == "LEADERBOARD") {
        lobbyWidget->updateLeaderboard(msg["payload"].toObject());
        
    } else if (messageType == "MATCH_HISTORY") {
        lobbyWidget->updateMatchHistory(msg["payload"].toObject());
        
    } else if (messageType == "MATCHING_STARTED") {
        QJsonObject payload = msg["payload"].toObject();
        QString message = payload["message"].toString();
        // Timer already started in onFindMatchClicked
        
    } else if (messageType == "MATCHING_CANCELLED") {
        lobbyWidget->stopMatchingTimer();
        
    } else if (messageType == "MATCH_FOUND") {
        lobbyWidget->stopMatchingTimer();
        
        QJsonObject payload = msg["payload"].toObject();
        currentOpponent = payload["opponent"].toString();
        currentOpponentElo = payload["elo"].toInt();
        opponentReady = false;
        
        qDebug() << "Match found against:" << currentOpponent;
        
        // Create custom dialog
        matchDialog = new QDialog(this);
        matchDialog->setWindowTitle("Match Found!");
        matchDialog->setModal(true);
        matchDialog->setMinimumSize(350, 200);
        
        QVBoxLayout* layout = new QVBoxLayout(matchDialog);
        
        QLabel* titleLabel = new QLabel("Match Found!", matchDialog);
        QFont titleFont = titleLabel->font();
        titleFont.setPointSize(16);
        titleFont.setBold(true);
        titleLabel->setFont(titleFont);
        titleLabel->setAlignment(Qt::AlignCenter);
        layout->addWidget(titleLabel);
        
        QLabel* opponentLabel = new QLabel(
            QString("Opponent: %1\nELO: %2").arg(currentOpponent).arg(currentOpponentElo),
            matchDialog);
        opponentLabel->setAlignment(Qt::AlignCenter);
        QFont opponentFont = opponentLabel->font();
        opponentFont.setPointSize(12);
        opponentLabel->setFont(opponentFont);
        layout->addWidget(opponentLabel);
        
        matchStatusLabel = new QLabel("Waiting for opponent to accept...", matchDialog);
        matchStatusLabel->setAlignment(Qt::AlignCenter);
        matchStatusLabel->setStyleSheet("color: orange; font-weight: bold;");
        layout->addWidget(matchStatusLabel);
        
        layout->addStretch();
        
        QHBoxLayout* buttonLayout = new QHBoxLayout();
        QPushButton* acceptButton = new QPushButton("Accept", matchDialog);
        acceptButton->setMinimumHeight(35);
        acceptButton->setStyleSheet("background-color: #66ff66; font-weight: bold;");
        connect(acceptButton, &QPushButton::clicked, [this, acceptButton]() {
            QJsonObject response;
            response["cmd"] = "MATCH_READY";
            response["payload"] = QJsonObject();
            gameClient->sendMessage(QJsonDocument(response).toJson(QJsonDocument::Compact));
            
            matchStatusLabel->setText("You: Ready ✓\nOpponent: " + 
                (opponentReady ? QString("Ready ✓") : QString("Waiting...")));
            acceptButton->setEnabled(false);
        });
        
        QPushButton* declineButton = new QPushButton("Decline", matchDialog);
        declineButton->setMinimumHeight(35);
        declineButton->setStyleSheet("background-color: #ff6666; font-weight: bold;");
        connect(declineButton, &QPushButton::clicked, [this]() {
            QJsonObject response;
            response["cmd"] = "MATCH_DECLINE";
            response["payload"] = QJsonObject();
            gameClient->sendMessage(QJsonDocument(response).toJson(QJsonDocument::Compact));
            matchDialog->reject();
        });
        
        buttonLayout->addWidget(acceptButton);
        buttonLayout->addWidget(declineButton);
        layout->addLayout(buttonLayout);
        
        matchDialog->show();
        
    } else if (messageType == "OPPONENT_READY") {
        QJsonObject payload = msg["payload"].toObject();
        QString opponentName = payload["username"].toString();
        opponentReady = true;
        
        if (matchDialog && matchStatusLabel) {
            matchStatusLabel->setText(QString("Opponent: Ready ✓\nStarting game..."));
            matchStatusLabel->setStyleSheet("color: green; font-weight: bold;");
        }
        
    } else if (messageType == "GAME_START") {
        QJsonObject payload = msg["payload"].toObject();
        QString opponent = payload["opponent"].toString();
        bool yourTurn = payload["your_turn"].toBool();
        
        // Store opponent for rematch
        lastOpponent = opponent;
        
        // Close match dialog if still open
        if (matchDialog) {
            matchDialog->accept();
            matchDialog = nullptr;
        }
        
        qDebug() << "Game starting against:" << opponent << "Your turn:" << yourTurn;
        gameWidget->startMatch(username, opponent, 0);  // ELO will be updated later
        showScreen(3); // Go to game screen
        gameWidget->showShipPlacement();
        
    } else if (messageType == "PLACE_SHIP_ACK") {
        // Server acknowledges ship placement, waiting for opponent
        QJsonObject payload = msg["payload"].toObject();
        QString message = payload["message"].toString();
        gameWidget->showWaitingMessage(message);
        qDebug() << "Ships placed, waiting for opponent:" << message;
        
    } else if (messageType == "WAITING_OPPONENT") {
        // Waiting for opponent to ready up
        QJsonObject payload = msg["payload"].toObject();
        QString message = payload["message"].toString();
        gameWidget->showWaitingMessage(message);
        qDebug() << "Waiting for opponent:" << message;
        
    } else if (messageType == "GAME_READY") {
        // Both players have placed ships, game is starting
        QJsonObject payload = msg["payload"].toObject();
        bool yourTurn = payload["your_turn"].toBool();
        QString message = payload["message"].toString();
        
        qDebug() << "Game ready! Your turn:" << yourTurn;
        
        // Kích hoạt game (set gameActive = true)
        gameWidget->activateGame();
        
        gameWidget->showWaitingMessage(message);
        gameWidget->setYourTurn(yourTurn);
        
    } else if (messageType == "SHIPS_PLACED") {
        gameWidget->showWaitingMessage("Waiting for opponent to place ships...");
        
    } else if (messageType == "YOUR_TURN") {
        gameWidget->setYourTurn(true);
        
    } else if (messageType == "TURN_CHANGE") {
        // Server thông báo thay đổi lượt
        QJsonObject payload = msg["payload"].toObject();
        bool yourTurn = payload["your_turn"].toBool();
        gameWidget->setYourTurn(yourTurn);
        qDebug() << "Turn changed - Your turn:" << yourTurn;
        
    } else if (messageType == "MOVE_RESULT") {
        QJsonObject payload = msg["payload"].toObject();
        QString coord = payload["coord"].toString(); // e.g., "G1"
        QString result = payload["result"].toString(); // "HIT", "MISS", "ALREADY_HIT"
        QString shipSunk = payload["ship_sunk"].toString(); // ship name if sunk
        bool isYourShot = payload["is_your_shot"].toBool();
        
        // Parse coordinate (e.g., "G1" -> row=6, col=1)
        if (coord.length() >= 2) {
            int row = coord[0].toLatin1() - 'A'; // A=0, B=1, ..., J=9
            int col = coord.mid(1).toInt(); // Extract number after letter
            bool hit = (result == "HIT");
            bool sunk = !shipSunk.isEmpty();
            
            qDebug() << "MOVE_RESULT: coord=" << coord << "row=" << row << "col=" << col 
                     << "result=" << result << "sunk=" << sunk << "isYourShot=" << isYourShot;
            
            gameWidget->updateMove(row, col, hit, sunk, isYourShot);
        }
        
    } else if (messageType == "OPPONENT_MOVE") {
        QJsonObject payload = msg["payload"].toObject();
        int row = payload["row"].toInt();
        int col = payload["col"].toInt();
        bool hit = payload["hit"].toBool();
        
        gameWidget->updateMove(row, col, hit, false, false);
        
    } else if (messageType == "GAME_END") {
        // Game kết thúc
        QJsonObject payload = msg["payload"].toObject();
        QString result = payload["result"].toString(); // WIN, LOSE, DRAW
        QString reason = payload["reason"].toString();
        int newElo = payload["elo"].toInt();
        
        qDebug() << "Game ended - Result:" << result << "Reason:" << reason << "ELO:" << newElo;
        
        // Hiển thị dialog kết quả với 2 nút
        showGameEndDialog(result, reason, newElo);
        
    } else if (messageType == "GAME_OVER") {
        QJsonObject payload = msg["payload"].toObject();
        QString winner = payload["winner"].toString();
        QString reason = payload["reason"].toString();
        int eloChange = payload["elo_change"].toInt();
        
        gameWidget->showGameOver(winner, reason, eloChange);
        
    } else if (messageType == "DRAW_OFFER") {
        int result = QMessageBox::question(this, "Draw Offer", 
            "Opponent offered a draw. Accept?",
            QMessageBox::Yes | QMessageBox::No);
        
        QJsonObject response;
        response["cmd"] = "DRAW_REPLY";
        QJsonObject payload;
        payload["status"] = (result == QMessageBox::Yes) ? "accept" : "reject";
        response["payload"] = payload;
        gameClient->sendMessage(QJsonDocument(response).toJson(QJsonDocument::Compact));
        
    } else if (messageType == "DRAW_REJECTED") {
        QMessageBox::information(this, "Draw Rejected", "Opponent rejected the draw offer.");
        
    } else if (messageType == "CHAT") {
        QJsonObject payload = msg["payload"].toObject();
        QString from = payload["from"].toString();
        QString message = payload["message"].toString();
        
        qDebug() << "Chat message from" << from << ":" << message;
        gameWidget->addChatMessage(from, message);
        
    } else if (messageType == "PLAYER_LIST") {
        lobbyWidget->updatePlayerList(msg["payload"].toObject());
        
    } else if (messageType == "CHALLENGE") {
        QJsonObject payload = msg["payload"].toObject();
        QString challenger = payload["challenger"].toString();
        
        qDebug() << "Challenge received from:" << challenger;
        showChallengeDialog(challenger);
        
    } else if (messageType == "CHALLENGE_REPLY") {
        QJsonObject payload = msg["payload"].toObject();
        QString targetUsername = payload["target_username"].toString();
        QString status = payload["status"].toString();
        
        if (status == "ACCEPT") {
            lobbyWidget->showWaitingMessage("Challenge accepted! Starting game...");
            QMessageBox::information(this, "Challenge Accepted", 
                targetUsername + " accepted your challenge!");
        } else if (status == "REJECT") {
            lobbyWidget->showWaitingMessage("Challenge rejected.");
            QMessageBox::information(this, "Challenge Rejected", 
                targetUsername + " rejected your challenge.");
        }
        
    } else if (messageType == "ERROR" || messageType == "SYSTEM_MSG") {
        QJsonObject payload = msg["payload"].toObject();
        QString message = payload["message"].toString();
        int code = payload["code"].toInt();
        
        if (!message.isEmpty()) {
            if (code >= 400) {
                QMessageBox::warning(this, "Message", message);
            } else {
                QMessageBox::information(this, "Message", message);
            }
        }
        
    } else if (messageType == "MATCH_DECLINED") {
        QJsonObject payload = msg["payload"].toObject();
        QString message = payload["message"].toString();
        lobbyWidget->showWaitingMessage(message);
        QMessageBox::information(this, "Match Declined", message);
        
    } else if (messageType == "LOGOUT_SUCCESS") {
        lobbyWidget->showWaitingMessage("");
        showScreen(1); // Back to login
        
    } else if (messageType == "PONG") {
        // Calculate ping from elapsed time
        qint64 elapsed = pingElapsed.elapsed();
        currentPing = static_cast<int>(elapsed);
        
        qDebug() << "PONG received - Ping:" << currentPing << "ms";
        
        // Update UI
        lobbyWidget->updatePing(currentPing);
        gameWidget->updateMyPing(currentPing);
        
        // Optionally send UPDATE_PING to server
        QJsonObject msg;
        msg["cmd"] = "UPDATE_PING";
        QJsonObject payload;
        payload["ping"] = currentPing;
        msg["payload"] = payload;
        QString jsonStr = QString("{\"cmd\":\"UPDATE_PING\",\"payload\":{\"ping\":%1}}")
                         .arg(currentPing);
        gameClient->sendMessage(jsonStr);
        
    } else if (messageType == "PING_UPDATE") {
        // Opponent's ping update
        QJsonObject payload = msg["payload"].toObject();
        int opponentPing = payload["opponent_ping"].toInt();
        
        qDebug() << "PING_UPDATE received - Opponent ping:" << opponentPing << "ms";
        
        // Update game widget if in game
        gameWidget->updateOpponentPing(opponentPing);
        
    } else {
        qDebug() << "Unhandled message type:" << messageType;
    }
}

void MainWindow::onErrorOccurred(QString error) {
    qWarning() << "Error:" << error;
    QMessageBox::critical(this, "Error", error);
}

void MainWindow::onLoginClicked(QString username, QString password) {
    QJsonObject msg;
    msg["cmd"] = "LOGIN";  // Server expects "cmd" not "type"
    QJsonObject payload;
    payload["username"] = username;
    payload["password"] = password;
    msg["payload"] = payload;
    
    // Tạo JSON string thủ công để đảm bảo thứ tự đúng
    QString jsonStr = QString("{\"cmd\":\"LOGIN\",\"payload\":{\"username\":\"%1\",\"password\":\"%2\"}}")
                          .arg(username).arg(password);
    
    gameClient->sendMessage(jsonStr);
}

void MainWindow::onRegisterClicked(QString username, QString password) {
    QJsonObject msg;
    msg["cmd"] = "REGISTER";  // Server expects "cmd"
    QJsonObject payload;
    payload["password"] = password;
    payload["username"] = username;
    msg["payload"] = payload;
    
    gameClient->sendMessage(QJsonDocument(msg).toJson(QJsonDocument::Compact));
}

void MainWindow::onFindMatchClicked() {
    QJsonObject msg;
    msg["cmd"] = "START_MATCHING";  // Server expects "cmd"
    msg["payload"] = QJsonObject();
    
    gameClient->sendMessage(QJsonDocument(msg).toJson(QJsonDocument::Compact));
    
    lobbyWidget->startMatchingTimer();
}

void MainWindow::onCancelMatchClicked() {
    QJsonObject msg;
    msg["cmd"] = "CANCEL_MATCHING";
    msg["payload"] = QJsonObject();
    
    gameClient->sendMessage(QJsonDocument(msg).toJson(QJsonDocument::Compact));
    
    lobbyWidget->stopMatchingTimer();
}

void MainWindow::onViewLeaderboardClicked() {
    QJsonObject msg;
    msg["cmd"] = "LEADERBOARD";  // Server expects "cmd"
    msg["payload"] = QJsonObject();
    
    gameClient->sendMessage(QJsonDocument(msg).toJson(QJsonDocument::Compact));
}

void MainWindow::onViewHistoryClicked() {
    QJsonObject msg;
    msg["cmd"] = "MATCH_HISTORY";  // Server expects "cmd"
    msg["payload"] = QJsonObject();
    
    gameClient->sendMessage(QJsonDocument(msg).toJson(QJsonDocument::Compact));
}

void MainWindow::onViewPlayersClicked() {
    QJsonObject msg;
    msg["cmd"] = "PLAYER_LIST";
    msg["payload"] = QJsonObject();
    
    gameClient->sendMessage(QJsonDocument(msg).toJson(QJsonDocument::Compact));
}

void MainWindow::onChallengePlayerClicked(const QString& targetUsername) {
    QString jsonStr = QString("{\"cmd\":\"CHALLENGE\",\"payload\":{\"target_username\":\"%1\"}}")
                          .arg(targetUsername);
    gameClient->sendMessage(jsonStr);
}

void MainWindow::onLogoutClicked() {
    QJsonObject msg;
    msg["cmd"] = "LOGOUT";  // Server expects "cmd"
    msg["payload"] = QJsonObject();
    
    gameClient->sendMessage(QJsonDocument(msg).toJson(QJsonDocument::Compact));
    
    showScreen(1); // Back to login (not connection screen)
}

void MainWindow::onMoveClicked(int row, int col) {
    // Convert row, col to coord format (e.g., row=0, col=5 -> "A5")
    char coordStr[10];
    sprintf(coordStr, "%c%d", 'A' + row, col);
    
    QJsonObject msg;
    msg["cmd"] = "MOVE";  // Server expects "cmd"
    QJsonObject payload;
    payload["coord"] = QString(coordStr);
    msg["payload"] = payload;
    
    gameClient->sendMessage(QJsonDocument(msg).toJson(QJsonDocument::Compact));
}

void MainWindow::onSurrenderClicked() {
    int result = QMessageBox::question(this, "Surrender", 
        "Are you sure you want to surrender?",
        QMessageBox::Yes | QMessageBox::No);
    
    if (result == QMessageBox::Yes) {
        QJsonObject msg;
        msg["cmd"] = "SURRENDER";  // Server expects "cmd"
        msg["payload"] = QJsonObject();
        
        gameClient->sendMessage(QJsonDocument(msg).toJson(QJsonDocument::Compact));
    }
}

void MainWindow::onOfferDrawClicked() {
    QJsonObject msg;
    msg["cmd"] = "DRAW_OFFER";  // Server expects "cmd"
    msg["payload"] = QJsonObject();
    
    gameClient->sendMessage(QJsonDocument(msg).toJson(QJsonDocument::Compact));
}

void MainWindow::onShipsPlaced(const QVector<QJsonObject>& ships) {
    // Tạo JSON string thủ công để đảm bảo thứ tự đúng với sscanf của server
    // Server expect: {"name":"...","size":N,"row":N,"col":N,"horizontal":true/false}
    
    QString shipsJson = "[";
    for (int i = 0; i < ships.size(); ++i) {
        if (i > 0) shipsJson += ",";
        
        const QJsonObject& ship = ships[i];
        QString name = ship["name"].toString();
        int size = ship["size"].toInt();
        int row = ship["row"].toInt();
        int col = ship["col"].toInt();
        bool horizontal = ship["horizontal"].toBool();
        
        // Format chính xác theo thứ tự server expect
        shipsJson += QString("{\"name\":\"%1\",\"size\":%2,\"row\":%3,\"col\":%4,\"horizontal\":%5}")
                         .arg(name)
                         .arg(size)
                         .arg(row)
                         .arg(col)
                         .arg(horizontal ? "true" : "false");
    }
    shipsJson += "]";
    
    // Tạo message hoàn chỉnh
    QString fullMessage = QString("{\"cmd\":\"PLACE_SHIPS\",\"payload\":{\"ships\":%1}}")
                              .arg(shipsJson);
    
    gameClient->sendMessage(fullMessage);
    
    qDebug() << "Ships placed, sent to server:" << fullMessage;
}

void MainWindow::onChatMessageSent(const QString& message) {
    QJsonObject msg;
    msg["cmd"] = "CHAT";
    
    QJsonObject payload;
    payload["message"] = message;
    msg["payload"] = payload;
    
    gameClient->sendMessage(QJsonDocument(msg).toJson(QJsonDocument::Compact));
    
    qDebug() << "Chat message sent:" << message;
}

void MainWindow::showScreen(int index) {
    stackedWidget->setCurrentIndex(index);
}

void MainWindow::showGameEndDialog(const QString& result, const QString& reason, int newElo) {
    // Tạo custom dialog
    QDialog* dialog = new QDialog(this);
    dialog->setWindowTitle("Kết quả trận đấu");
    dialog->setModal(true);
    dialog->setMinimumWidth(400);
    
    QVBoxLayout* layout = new QVBoxLayout(dialog);
    
    // Icon và kết quả
    QLabel* iconLabel = new QLabel(dialog);
    QFont resultFont;
    resultFont.setPointSize(18);
    resultFont.setBold(true);
    
    QString resultText;
    QString iconColor;
    QString resultColor;
    
    if (result == "WIN") {
        resultText = "🎉 CHIẾN THẮNG! 🎉";
        iconColor = "green";
        resultColor = "color: green;";
    } else if (result == "LOSE") {
        resultText = "😢 THẤT BẠI 😢";
        iconColor = "red";
        resultColor = "color: red;";
    } else {
        resultText = "🤝 HÒA 🤝";
        iconColor = "blue";
        resultColor = "color: blue;";
    }
    
    QLabel* resultLabel = new QLabel(resultText, dialog);
    resultLabel->setFont(resultFont);
    resultLabel->setAlignment(Qt::AlignCenter);
    resultLabel->setStyleSheet(resultColor);
    layout->addWidget(resultLabel);
    
    layout->addSpacing(20);
    
    // Lý do
    QString reasonText = reason;
    if (reason == "SURRENDER") reasonText = "Đầu hàng";
    else if (reason == "DRAW_ACCEPTED") reasonText = "Đồng ý hòa";
    else if (reason == "ALL_SHIPS_SUNK") reasonText = "Tất cả tàu bị đánh chìm";
    else if (reason == "OPPONENT_DISCONNECTED") reasonText = "Đối thủ ngắt kết nối";
    
    QLabel* reasonLabel = new QLabel("Lý do: " + reasonText, dialog);
    reasonLabel->setAlignment(Qt::AlignCenter);
    QFont reasonFont = reasonLabel->font();
    reasonFont.setPointSize(12);
    reasonLabel->setFont(reasonFont);
    layout->addWidget(reasonLabel);
    
    layout->addSpacing(10);
    
    // ELO
    QLabel* eloLabel = new QLabel(QString("ELO mới: %1").arg(newElo), dialog);
    eloLabel->setAlignment(Qt::AlignCenter);
    QFont eloFont = eloLabel->font();
    eloFont.setPointSize(14);
    eloFont.setBold(true);
    eloLabel->setFont(eloFont);
    layout->addWidget(eloLabel);
    
    layout->addSpacing(30);
    
    // Buttons
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    
    QPushButton* rematchButton = new QPushButton("Đấu lại", dialog);
    rematchButton->setMinimumWidth(120);
    rematchButton->setMinimumHeight(40);
    rematchButton->setStyleSheet("background-color: #4CAF50; color: white; font-weight: bold; border-radius: 5px;");
    
    QPushButton* lobbyButton = new QPushButton("Về sảnh", dialog);
    lobbyButton->setMinimumWidth(120);
    lobbyButton->setMinimumHeight(40);
    lobbyButton->setStyleSheet("background-color: #2196F3; color: white; font-weight: bold; border-radius: 5px;");
    
    buttonLayout->addWidget(rematchButton);
    buttonLayout->addWidget(lobbyButton);
    layout->addLayout(buttonLayout);
    
    // Connect buttons
    connect(rematchButton, &QPushButton::clicked, [this, dialog, newElo]() {
        dialog->accept();
        
        // Cập nhật ELO trong lobby
        elo = newElo;
        lobbyWidget->setUserInfo(username, elo);
        
        // Về lobby
        showScreen(2);
        
        // Send challenge to last opponent
        if (!lastOpponent.isEmpty()) {
            QString jsonStr = QString("{\"cmd\":\"CHALLENGE\",\"payload\":{\"target_username\":\"%1\"}}")
                                  .arg(lastOpponent);
            gameClient->sendMessage(jsonStr);
            lobbyWidget->showWaitingMessage("Rematch challenge sent to " + lastOpponent + "...");
        }
    });
    
    connect(lobbyButton, &QPushButton::clicked, [this, dialog, newElo]() {
        dialog->accept();
        
        // Cập nhật ELO trong lobby
        elo = newElo;
        lobbyWidget->setUserInfo(username, elo);
        
        // Về lobby
        showScreen(2);
    });
    
    dialog->exec();
    delete dialog;
}

void MainWindow::showChallengeDialog(const QString& challenger) {
    // Create custom dialog
    QDialog* dialog = new QDialog(this);
    dialog->setWindowTitle("Challenge Received");
    dialog->setModal(true);
    dialog->setMinimumSize(400, 200);
    
    QVBoxLayout* layout = new QVBoxLayout(dialog);
    
    // Title
    QLabel* titleLabel = new QLabel("⚔️ CHALLENGE REQUEST ⚔️", dialog);
    QFont titleFont = titleLabel->font();
    titleFont.setPointSize(16);
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet("color: #FF6600;");
    layout->addWidget(titleLabel);
    
    layout->addSpacing(20);
    
    // Challenger info
    QLabel* challengerLabel = new QLabel(
        QString("%1 wants to challenge you!").arg(challenger),
        dialog);
    challengerLabel->setAlignment(Qt::AlignCenter);
    QFont challengerFont = challengerLabel->font();
    challengerFont.setPointSize(14);
    challengerLabel->setFont(challengerFont);
    layout->addWidget(challengerLabel);
    
    layout->addSpacing(30);
    
    // Buttons
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    
    QPushButton* acceptButton = new QPushButton("Accept", dialog);
    acceptButton->setMinimumWidth(120);
    acceptButton->setMinimumHeight(40);
    acceptButton->setStyleSheet("background-color: #4CAF50; color: white; font-weight: bold; border-radius: 5px;");
    
    QPushButton* declineButton = new QPushButton("Decline", dialog);
    declineButton->setMinimumWidth(120);
    declineButton->setMinimumHeight(40);
    declineButton->setStyleSheet("background-color: #f44336; color: white; font-weight: bold; border-radius: 5px;");
    
    buttonLayout->addWidget(acceptButton);
    buttonLayout->addWidget(declineButton);
    layout->addLayout(buttonLayout);
    
    // Connect buttons
    connect(acceptButton, &QPushButton::clicked, [this, dialog, challenger]() {
        QString jsonStr = QString("{\"cmd\":\"CHALLENGE_REPLY\",\"payload\":{\"challenger_username\":\"%1\",\"status\":\"ACCEPT\"}}")
                              .arg(challenger);
        gameClient->sendMessage(jsonStr);
        lobbyWidget->showWaitingMessage("Challenge accepted! Waiting for game to start...");
        dialog->accept();
    });
    
    connect(declineButton, &QPushButton::clicked, [this, dialog, challenger]() {
        QString jsonStr = QString("{\"cmd\":\"CHALLENGE_REPLY\",\"payload\":{\"challenger_username\":\"%1\",\"status\":\"REJECT\"}}")
                              .arg(challenger);
        gameClient->sendMessage(jsonStr);
        dialog->reject();
    });
    
    dialog->exec();
    delete dialog;
}

void MainWindow::sendPing() {
    // Start measuring time
    pingElapsed.start();
    
    // Send PING command
    QString jsonStr = "{\"cmd\":\"PING\",\"payload\":{}}";
    gameClient->sendMessage(jsonStr);
}

