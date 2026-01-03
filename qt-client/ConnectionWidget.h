#ifndef CONNECTIONWIDGET_H
#define CONNECTIONWIDGET_H

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>

class ConnectionWidget : public QWidget {
    Q_OBJECT
    
public:
    explicit ConnectionWidget(QWidget *parent = nullptr);
    
signals:
    void connectClicked(QString host, int port);
    
private slots:
    void onConnectButtonClicked();
    
private:
    void setupUI();
    
    QLineEdit* hostEdit;
    QLineEdit* portEdit;
    QPushButton* connectButton;
    QLabel* statusLabel;
};

#endif // CONNECTIONWIDGET_H
