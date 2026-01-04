#ifndef GAMEWIDGET_H
#define GAMEWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QGridLayout>
#include <QVector>
#include <QListWidget>
#include <QRadioButton>
#include <QButtonGroup>
#include <QJsonObject>
#include <QTextEdit>
#include <QLineEdit>

// Struct để lưu thông tin tàu
struct Ship {
    QString name;
    int size;
    QColor color;
};

class GameWidget : public QWidget {
    Q_OBJECT
    
public:
    explicit GameWidget(QWidget *parent = nullptr);
    
    void startMatch(const QString& myName, const QString& opponentName, int opponentElo);
    void showShipPlacement();
    void showWaitingMessage(const QString& message);
    void setYourTurn(bool isTurn);
    void activateGame(); // Kích hoạt game sau khi cả 2 đặt thuyền xong
    void updateMove(int row, int col, bool hit, bool sunk, bool isMyMove, const QString& shipName = "");
    void showGameOver(const QString& winner, const QString& reason, int eloChange);
    void addChatMessage(const QString& sender, const QString& message);
    void updateMyPing(int ping);
    void updateOpponentPing(int ping);
    
signals:
    void moveClicked(int row, int col);
    void surrenderClicked();
    void offerDrawClicked();
    void shipsPlaced(const QVector<QJsonObject>& ships);
    void chatMessageSent(const QString& message);
    
private slots:
    void onMyBoardCellClicked(int row, int col);
    void onOpponentBoardCellClicked(int row, int col);
    void onSurrenderButtonClicked();
    void onDrawButtonClicked();
    void onShipSelected(int index);
    void onOrientationChanged();
    void onReadyButtonClicked();
    void onRandomPlaceClicked();
    void onSendChatClicked();
    
private:
    void setupUI();
    void createBoards();
    void clearBoards();
    void setupShipPlacementUI();
    bool canPlaceShip(int row, int col, int size, bool horizontal);
    void placeShip(int row, int col, int size, bool horizontal, int shipIndex = -1);
    void randomPlaceShips();
    void clearAllShips();
    
    QLabel* playerLabel;
    QLabel* myPingLabel;
    QLabel* opponentLabel;
    QLabel* opponentPingLabel;
    QLabel* statusLabel;
    QPushButton* surrenderButton;
    QPushButton* drawButton;
    
    // Hai bảng 10x10
    QGridLayout* myBoardLayout;
    QGridLayout* opponentBoardLayout;
    QVector<QVector<QPushButton*>> myCells;
    QVector<QVector<QPushButton*>> opponentCells;
    
    // Ship placement UI
    QWidget* shipPlacementPanel;
    QListWidget* shipList;
    QRadioButton* horizontalRadio;
    QRadioButton* verticalRadio;
    QPushButton* readyButton;
    QPushButton* randomButton;
    QButtonGroup* orientationGroup;
    
    // Ship data
    QVector<Ship> availableShips;
    int currentShipIndex;
    bool isHorizontal;
    QVector<QJsonObject> placedShips;
    QVector<QVector<int>> myBoard; // 0 = empty, >0 = ship ID
    
    bool myTurn;
    bool gameActive;
    bool inPlacementMode;
    
    // Chat UI
    QTextEdit* chatDisplay;
    QLineEdit* chatInput;
    QPushButton* sendChatButton;
    QString myName;
    QString opponentName;
};

#endif // GAMEWIDGET_H
