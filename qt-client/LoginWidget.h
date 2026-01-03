#ifndef LOGINWIDGET_H
#define LOGINWIDGET_H

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>

class LoginWidget : public QWidget {
    Q_OBJECT
    
public:
    explicit LoginWidget(QWidget *parent = nullptr);
    
signals:
    void loginClicked(QString username, QString password);
    void registerClicked(QString username, QString password);
    
private slots:
    void onLoginButtonClicked();
    void onRegisterButtonClicked();
    
private:
    void setupUI();
    
    QLineEdit* usernameEdit;
    QLineEdit* passwordEdit;
    QPushButton* loginButton;
    QPushButton* registerButton;
    QLabel* statusLabel;
};

#endif // LOGINWIDGET_H
