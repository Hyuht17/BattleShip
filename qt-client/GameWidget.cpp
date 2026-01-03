#include "GameWidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QGroupBox>
#include <QJsonObject>
#include <QDebug>
#include <QTime>

GameWidget::GameWidget(QWidget *parent)
    : QWidget(parent)
    , myTurn(false)
    , gameActive(false)
    , inPlacementMode(false)
    , currentShipIndex(-1)
    , isHorizontal(true)
{
    // Khởi tạo các tàu
    availableShips.append({"Carrier (5)", 5, QColor(255, 0, 0)});
    availableShips.append({"Battleship (4)", 4, QColor(0, 255, 0)});
    availableShips.append({"Cruiser (3)", 3, QColor(0, 0, 255)});
    availableShips.append({"Submarine (3)", 3, QColor(255, 255, 0)});
    availableShips.append({"Destroyer (2)", 2, QColor(255, 0, 255)});
    
    // Khởi tạo bảng trống
    myBoard.resize(10);
    for (int i = 0; i < 10; ++i) {
        myBoard[i].resize(10, 0);
    }
    
    setupUI();
}

void GameWidget::setupUI() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    
    // Player info
    QHBoxLayout* infoLayout = new QHBoxLayout();
    
    playerLabel = new QLabel("You", this);
    QFont playerFont = playerLabel->font();
    playerFont.setPointSize(14);
    playerFont.setBold(true);
    playerLabel->setFont(playerFont);
    infoLayout->addWidget(playerLabel);
    
    infoLayout->addSpacing(15);
    
    // My ping label
    myPingLabel = new QLabel("Ping: --", this);
    QFont myPingFont = myPingLabel->font();
    myPingFont.setPointSize(11);
    myPingLabel->setFont(myPingFont);
    myPingLabel->setStyleSheet("color: #666;");
    infoLayout->addWidget(myPingLabel);
    
    infoLayout->addStretch();
    
    opponentLabel = new QLabel("Opponent", this);
    opponentLabel->setFont(playerFont);
    infoLayout->addWidget(opponentLabel);
    
    infoLayout->addSpacing(15);
    
    // Opponent ping label
    opponentPingLabel = new QLabel("Ping: --", this);
    QFont pingFont = opponentPingLabel->font();
    pingFont.setPointSize(11);
    opponentPingLabel->setFont(pingFont);
    opponentPingLabel->setStyleSheet("color: #666;");
    infoLayout->addWidget(opponentPingLabel);
    
    mainLayout->addLayout(infoLayout);
    
    // Status
    statusLabel = new QLabel("Waiting for match...", this);
    statusLabel->setAlignment(Qt::AlignCenter);
    statusLabel->setStyleSheet("font-size: 16px; color: blue;");
    mainLayout->addWidget(statusLabel);
    
    // Main game area với 2 bảng và panel đặt thuyền
    QHBoxLayout* gameLayout = new QHBoxLayout();
    
    // Khởi tạo cả 2 grid layouts trước
    myBoardLayout = new QGridLayout();
    myBoardLayout->setSpacing(1);
    
    opponentBoardLayout = new QGridLayout();
    opponentBoardLayout->setSpacing(1);
    
    // Tạo nội dung các bảng
    createBoards();
    
    // Bảng của bạn (bên trái)
    QVBoxLayout* myBoardContainer = new QVBoxLayout();
    QLabel* myBoardLabel = new QLabel("Your Board", this);
    myBoardLabel->setAlignment(Qt::AlignCenter);
    myBoardLabel->setStyleSheet("font-weight: bold;");
    myBoardContainer->addWidget(myBoardLabel);
    myBoardContainer->addLayout(myBoardLayout);
    
    gameLayout->addLayout(myBoardContainer);
    
    // Bảng đối thủ (giữa)
    QVBoxLayout* opponentBoardContainer = new QVBoxLayout();
    QLabel* opponentBoardLabel = new QLabel("Opponent Board", this);
    opponentBoardLabel->setAlignment(Qt::AlignCenter);
    opponentBoardLabel->setStyleSheet("font-weight: bold;");
    opponentBoardContainer->addWidget(opponentBoardLabel);
    opponentBoardContainer->addLayout(opponentBoardLayout);
    
    gameLayout->addLayout(opponentBoardContainer);
    
    // Ship placement panel (bên phải, ban đầu ẩn)
    setupShipPlacementUI();
    gameLayout->addWidget(shipPlacementPanel);
    shipPlacementPanel->hide();
    
    // Chat panel (bên phải)
    QVBoxLayout* chatContainer = new QVBoxLayout();
    QLabel* chatLabel = new QLabel("Chat", this);
    chatLabel->setAlignment(Qt::AlignCenter);
    chatLabel->setStyleSheet("font-weight: bold;");
    chatContainer->addWidget(chatLabel);
    
    chatDisplay = new QTextEdit(this);
    chatDisplay->setReadOnly(true);
    chatDisplay->setMaximumHeight(150);
    chatDisplay->setStyleSheet("background-color: #f0f0f0; border: 1px solid gray;");
    chatContainer->addWidget(chatDisplay);
    
    QHBoxLayout* chatInputLayout = new QHBoxLayout();
    chatInput = new QLineEdit(this);
    chatInput->setPlaceholderText("Type a message...");
    chatInput->setMaxLength(200);
    connect(chatInput, &QLineEdit::returnPressed, this, &GameWidget::onSendChatClicked);
    chatInputLayout->addWidget(chatInput);
    
    sendChatButton = new QPushButton("Send", this);
    sendChatButton->setMaximumWidth(60);
    connect(sendChatButton, &QPushButton::clicked, this, &GameWidget::onSendChatClicked);
    chatInputLayout->addWidget(sendChatButton);
    
    chatContainer->addLayout(chatInputLayout);
    gameLayout->addLayout(chatContainer);
    
    mainLayout->addLayout(gameLayout);
    
    // Action buttons
    QHBoxLayout* actionLayout = new QHBoxLayout();
    
    surrenderButton = new QPushButton("Surrender", this);
    surrenderButton->setEnabled(false);
    connect(surrenderButton, &QPushButton::clicked, this, &GameWidget::onSurrenderButtonClicked);
    actionLayout->addWidget(surrenderButton);
    
    drawButton = new QPushButton("Offer Draw", this);
    drawButton->setEnabled(false);
    connect(drawButton, &QPushButton::clicked, this, &GameWidget::onDrawButtonClicked);
    actionLayout->addWidget(drawButton);
    
    mainLayout->addLayout(actionLayout);
}

void GameWidget::createBoards() {
    // Tạo bảng của bạn
    myCells.resize(10);
    for (int row = 0; row < 10; ++row) {
        myCells[row].resize(10);
        for (int col = 0; col < 10; ++col) {
            QPushButton* cell = new QPushButton(this);
            cell->setFixedSize(30, 30);
            cell->setStyleSheet("background-color: lightblue; border: 1px solid gray;");
            
            connect(cell, &QPushButton::clicked, [this, row, col]() {
                onMyBoardCellClicked(row, col);
            });
            
            myCells[row][col] = cell;
            myBoardLayout->addWidget(cell, row, col);
        }
    }
    
    // Tạo bảng đối thủ
    opponentCells.resize(10);
    for (int row = 0; row < 10; ++row) {
        opponentCells[row].resize(10);
        for (int col = 0; col < 10; ++col) {
            QPushButton* cell = new QPushButton(this);
            cell->setFixedSize(30, 30);
            cell->setStyleSheet("background-color: lightblue; border: 1px solid gray;");
            cell->setEnabled(false);
            
            connect(cell, &QPushButton::clicked, [this, row, col]() {
                onOpponentBoardCellClicked(row, col);
            });
            
            opponentCells[row][col] = cell;
            opponentBoardLayout->addWidget(cell, row, col);
        }
    }
}

void GameWidget::setupShipPlacementUI() {
    shipPlacementPanel = new QWidget(this);
    QVBoxLayout* panelLayout = new QVBoxLayout(shipPlacementPanel);
    
    QLabel* titleLabel = new QLabel("Place Your Ships", this);
    titleLabel->setStyleSheet("font-weight: bold; font-size: 14px;");
    titleLabel->setAlignment(Qt::AlignCenter);
    panelLayout->addWidget(titleLabel);
    
    // Danh sách tàu
    QLabel* shipListLabel = new QLabel("Select Ship:", this);
    panelLayout->addWidget(shipListLabel);
    
    shipList = new QListWidget(this);
    for (const Ship& ship : availableShips) {
        shipList->addItem(ship.name);
    }
    connect(shipList, &QListWidget::currentRowChanged, this, &GameWidget::onShipSelected);
    panelLayout->addWidget(shipList);
    
    // Chọn hướng
    QLabel* orientationLabel = new QLabel("Orientation:", this);
    panelLayout->addWidget(orientationLabel);
    
    orientationGroup = new QButtonGroup(this);
    horizontalRadio = new QRadioButton("Horizontal", this);
    verticalRadio = new QRadioButton("Vertical", this);
    horizontalRadio->setChecked(true);
    
    orientationGroup->addButton(horizontalRadio);
    orientationGroup->addButton(verticalRadio);
    
    connect(horizontalRadio, &QRadioButton::toggled, this, &GameWidget::onOrientationChanged);
    
    panelLayout->addWidget(horizontalRadio);
    panelLayout->addWidget(verticalRadio);
    
    panelLayout->addSpacing(20);
    
    // Nút Ready
    readyButton = new QPushButton("Ready!", this);
    readyButton->setEnabled(false);
    readyButton->setStyleSheet("background-color: green; color: white; font-weight: bold; padding: 10px;");
    connect(readyButton, &QPushButton::clicked, this, &GameWidget::onReadyButtonClicked);
    panelLayout->addWidget(readyButton);
    
    panelLayout->addStretch();
    
    shipPlacementPanel->setMaximumWidth(250);
}

void GameWidget::clearBoards() {
    // Clear my board
    for (int row = 0; row < 10; ++row) {
        for (int col = 0; col < 10; ++col) {
            myCells[row][col]->setStyleSheet("background-color: lightblue; border: 1px solid gray;");
            myCells[row][col]->setText("");
            myCells[row][col]->setEnabled(true);
            myBoard[row][col] = 0;
        }
    }
    
    // Clear opponent board
    for (int row = 0; row < 10; ++row) {
        for (int col = 0; col < 10; ++col) {
            opponentCells[row][col]->setStyleSheet("background-color: lightblue; border: 1px solid gray;");
            opponentCells[row][col]->setText("");
            opponentCells[row][col]->setEnabled(false);
        }
    }
    
    placedShips.clear();
}

void GameWidget::startMatch(const QString& myName, const QString& opponentName, int opponentElo) {
    this->myName = myName;
    this->opponentName = opponentName;
    
    playerLabel->setText("You: " + myName);
    opponentLabel->setText(opponentName);
    
    clearBoards();
    gameActive = true;
    myTurn = false;
    inPlacementMode = false;
    
    statusLabel->setText("Match starting...");
    surrenderButton->setEnabled(true);
    drawButton->setEnabled(true);
    shipPlacementPanel->hide();
    
    // Clear chat
    chatDisplay->clear();
    chatInput->clear();
    chatInput->setEnabled(true);
    sendChatButton->setEnabled(true);
}

void GameWidget::showShipPlacement() {
    statusLabel->setText("Place your ships! (Select a ship first)");
    inPlacementMode = true;
    shipPlacementPanel->show();
    
    // Reset ship list - enable lại tất cả ships
    for (int i = 0; i < shipList->count(); ++i) {
        QListWidgetItem* item = shipList->item(i);
        item->setFlags(item->flags() | Qt::ItemIsEnabled);
        item->setBackground(Qt::white);
    }
    // Không chọn thuyền nào ban đầu - bắt buộc phải click chọn
    shipList->setCurrentRow(-1);
    
    // Reset ready button
    readyButton->setEnabled(false);
    currentShipIndex = -1;
    
    // Enable my board for placement
    for (int row = 0; row < 10; ++row) {
        for (int col = 0; col < 10; ++col) {
            myCells[row][col]->setEnabled(true);
        }
    }
}

void GameWidget::showWaitingMessage(const QString& message) {
    statusLabel->setText(message);
}

void GameWidget::activateGame() {
    gameActive = true;
    inPlacementMode = false;
    qDebug() << "Game activated! gameActive:" << gameActive;
}

void GameWidget::setYourTurn(bool isTurn) {
    myTurn = isTurn;
    qDebug() << "setYourTurn called - myTurn:" << myTurn << "gameActive:" << gameActive;
    
    if (isTurn) {
        statusLabel->setText("Your turn! Click on opponent's board to attack");
        statusLabel->setStyleSheet("font-size: 16px; color: green;");
        
        // Enable opponent board - enable tất cả cells chưa bị hit
        for (int row = 0; row < 10; ++row) {
            for (int col = 0; col < 10; ++col) {
                // Chỉ enable những ô chưa có text (chưa bị tấn công)
                if (opponentCells[row][col]->text().isEmpty()) {
                    opponentCells[row][col]->setEnabled(true);
                }
            }
        }
        qDebug() << "Opponent board enabled for attack";
    } else {
        statusLabel->setText("Opponent's turn...");
        statusLabel->setStyleSheet("font-size: 16px; color: red;");
        
        // Disable opponent board
        for (int row = 0; row < 10; ++row) {
            for (int col = 0; col < 10; ++col) {
                opponentCells[row][col]->setEnabled(false);
            }
        }
    }
}

void GameWidget::updateMove(int row, int col, bool hit, bool sunk, bool isMyMove) {
    QPushButton* cell = isMyMove ? opponentCells[row][col] : myCells[row][col];
    
    if (hit) {
        cell->setStyleSheet("background-color: red; border: 1px solid darkred;");
        cell->setText("X");
    } else {
        cell->setStyleSheet("background-color: gray; border: 1px solid darkgray;");
        cell->setText("O");
    }
    
    cell->setEnabled(false);
    
    if (sunk) {
        statusLabel->setText(isMyMove ? "You sunk a ship!" : "Opponent sunk your ship!");
    }
}

void GameWidget::showGameOver(const QString& winner, const QString& reason, int eloChange) {
    gameActive = false;
    myTurn = false;
    inPlacementMode = false;
    
    surrenderButton->setEnabled(false);
    drawButton->setEnabled(false);
    shipPlacementPanel->hide();
    
    QString eloText = (eloChange >= 0) ? "+" + QString::number(eloChange) : QString::number(eloChange);
    QString message = QString("Game Over!\nWinner: %1\nReason: %2\nELO Change: %3")
                          .arg(winner)
                          .arg(reason)
                          .arg(eloText);
    
    QMessageBox::information(this, "Game Over", message);
    
    statusLabel->setText("Game ended. " + reason);
}

// Slot handlers
void GameWidget::onMyBoardCellClicked(int row, int col) {
    if (!inPlacementMode || currentShipIndex < 0) {
        return;
    }
    
    Ship ship = availableShips[currentShipIndex];
    
    if (canPlaceShip(row, col, ship.size, isHorizontal)) {
        placeShip(row, col, ship.size, isHorizontal);
        
        // Lưu thông tin tàu đã đặt - format phải match với server
        QJsonObject shipData;
        shipData["name"] = ship.name;  // Server expects "name"
        shipData["size"] = ship.size;  // Server expects "size"
        shipData["row"] = row;         // Server expects "row" not "start_row"
        shipData["col"] = col;         // Server expects "col" not "start_col"
        shipData["horizontal"] = isHorizontal;  // Server expects "horizontal"
        placedShips.append(shipData);
        
        // Disable ship đã đặt trong list
        QListWidgetItem* item = shipList->item(currentShipIndex);
        item->setFlags(item->flags() & ~Qt::ItemIsEnabled);
        item->setBackground(QColor(200, 200, 200));
        
        // Chuyển sang tàu tiếp theo
        currentShipIndex = -1;
        for (int i = 0; i < shipList->count(); ++i) {
            if (shipList->item(i)->flags() & Qt::ItemIsEnabled) {
                shipList->setCurrentRow(i);
                break;
            }
        }
        
        // Nếu đã đặt đủ 5 tàu, enable nút Ready
        if (placedShips.size() == 5) {
            readyButton->setEnabled(true);
            statusLabel->setText("All ships placed! Click Ready when you're ready!");
        }
    } else {
        QMessageBox::warning(this, "Invalid Placement", "Cannot place ship here!");
    }
}

void GameWidget::onOpponentBoardCellClicked(int row, int col) {
    qDebug() << "Cell clicked - gameActive:" << gameActive << "myTurn:" << myTurn << "row:" << row << "col:" << col;
    
    if (!gameActive || !myTurn) {
        qDebug() << "Click rejected! gameActive:" << gameActive << "myTurn:" << myTurn;
        return;
    }
    
    qDebug() << "Sending move to server";
    
    // Disable clicked cell
    opponentCells[row][col]->setEnabled(false);
    
    emit moveClicked(row, col);
}

void GameWidget::onSurrenderButtonClicked() {
    emit surrenderClicked();
}

void GameWidget::onDrawButtonClicked() {
    emit offerDrawClicked();
}

void GameWidget::onShipSelected(int index) {
    if (index >= 0 && index < availableShips.size()) {
        currentShipIndex = index;
    }
}

void GameWidget::onOrientationChanged() {
    isHorizontal = horizontalRadio->isChecked();
}

void GameWidget::onReadyButtonClicked() {
    if (placedShips.size() != 5) {
        QMessageBox::warning(this, "Not Ready", "You must place all 5 ships first!");
        return;
    }
    
    inPlacementMode = false;
    shipPlacementPanel->hide();
    
    // Disable my board
    for (int row = 0; row < 10; ++row) {
        for (int col = 0; col < 10; ++col) {
            myCells[row][col]->setEnabled(false);
        }
    }
    
    statusLabel->setText("Waiting for opponent to place ships...");
    
    emit shipsPlaced(placedShips);
}

// Helper functions
bool GameWidget::canPlaceShip(int row, int col, int size, bool horizontal) {
    if (horizontal) {
        // Kiểm tra nằm ngang
        if (col + size > 10) return false;
        
        for (int c = col; c < col + size; ++c) {
            if (myBoard[row][c] != 0) return false;
        }
    } else {
        // Kiểm tra nằm dọc
        if (row + size > 10) return false;
        
        for (int r = row; r < row + size; ++r) {
            if (myBoard[r][col] != 0) return false;
        }
    }
    
    return true;
}

void GameWidget::placeShip(int row, int col, int size, bool horizontal) {
    int shipId = placedShips.size() + 1;
    QColor color = availableShips[currentShipIndex].color;
    
    QString styleSheet = QString("background-color: rgb(%1, %2, %3); border: 1px solid black;")
                             .arg(color.red())
                             .arg(color.green())
                             .arg(color.blue());
    
    if (horizontal) {
        for (int c = col; c < col + size; ++c) {
            myBoard[row][c] = shipId;
            myCells[row][c]->setStyleSheet(styleSheet);
            myCells[row][c]->setText("■");
        }
    } else {
        for (int r = row; r < row + size; ++r) {
            myBoard[r][col] = shipId;
            myCells[r][col]->setStyleSheet(styleSheet);
            myCells[r][col]->setText("■");
        }
    }
}

void GameWidget::onSendChatClicked() {
    QString message = chatInput->text().trimmed();
    if (message.isEmpty() || !gameActive) {
        return;
    }
    
    // Add to local chat display
    addChatMessage("You", message);
    
    // Send to server
    emit chatMessageSent(message);
    
    // Clear input
    chatInput->clear();
}

void GameWidget::addChatMessage(const QString& sender, const QString& message) {
    QString timestamp = QTime::currentTime().toString("HH:mm:ss");
    QString color = (sender == "You" || sender == myName) ? "#0066cc" : "#cc6600";
    
    QString html = QString("<span style='color: gray;'>[%1]</span> "
                          "<span style='color: %2; font-weight: bold;'>%3:</span> "
                          "<span>%4</span><br>")
                      .arg(timestamp)
                      .arg(color)
                      .arg(sender)
                      .arg(message.toHtmlEscaped());
    
    chatDisplay->append(html);
    
    // Auto scroll to bottom
    QTextCursor cursor = chatDisplay->textCursor();
    cursor.movePosition(QTextCursor::End);
    chatDisplay->setTextCursor(cursor);
}

void GameWidget::updateMyPing(int ping) {
    QString color;
    if (ping < 50) {
        color = "#00cc00"; // Green
    } else if (ping < 100) {
        color = "#ffaa00"; // Yellow/Orange
    } else {
        color = "#ff0000"; // Red
    }
    
    myPingLabel->setText(QString("Ping: %1ms").arg(ping));
    myPingLabel->setStyleSheet(QString("color: %1;").arg(color));
}

void GameWidget::updateOpponentPing(int ping) {
    QString color;
    if (ping < 50) {
        color = "#00cc00"; // Green
    } else if (ping < 100) {
        color = "#ffaa00"; // Yellow/Orange
    } else {
        color = "#ff0000"; // Red
    }
    
    opponentPingLabel->setText(QString("Ping: %1ms").arg(ping));
    opponentPingLabel->setStyleSheet(QString("color: %1;").arg(color));
}
