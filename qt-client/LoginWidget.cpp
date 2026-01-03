#include "LoginWidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>

LoginWidget::LoginWidget(QWidget *parent)
    : QWidget(parent)
{
    setupUI();
}

void LoginWidget::setupUI() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setAlignment(Qt::AlignCenter);
    
    // Title
    QLabel* titleLabel = new QLabel("Battleship Game", this);
    QFont titleFont = titleLabel->font();
    titleFont.setPointSize(24);
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);
    titleLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(titleLabel);
    
    mainLayout->addSpacing(30);
    
    // Login form
    QGroupBox* loginBox = new QGroupBox("Login", this);
    QFormLayout* formLayout = new QFormLayout(loginBox);
    
    usernameEdit = new QLineEdit(this);
    usernameEdit->setPlaceholderText("Enter username");
    usernameEdit->setMinimumWidth(250);
    formLayout->addRow("Username:", usernameEdit);
    
    passwordEdit = new QLineEdit(this);
    passwordEdit->setPlaceholderText("Enter password");
    passwordEdit->setEchoMode(QLineEdit::Password);
    formLayout->addRow("Password:", passwordEdit);
    
    mainLayout->addWidget(loginBox);
    
    // Buttons
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    
    loginButton = new QPushButton("Login", this);
    loginButton->setMinimumWidth(120);
    connect(loginButton, &QPushButton::clicked, this, &LoginWidget::onLoginButtonClicked);
    buttonLayout->addWidget(loginButton);
    
    registerButton = new QPushButton("Register", this);
    registerButton->setMinimumWidth(120);
    connect(registerButton, &QPushButton::clicked, this, &LoginWidget::onRegisterButtonClicked);
    buttonLayout->addWidget(registerButton);
    
    mainLayout->addLayout(buttonLayout);
    
    // Status label
    statusLabel = new QLabel(this);
    statusLabel->setAlignment(Qt::AlignCenter);
    statusLabel->setStyleSheet("color: red;");
    mainLayout->addWidget(statusLabel);
    
    // Add stretch at bottom
    mainLayout->addStretch();
    
    // Enter key triggers login
    connect(usernameEdit, &QLineEdit::returnPressed, this, &LoginWidget::onLoginButtonClicked);
    connect(passwordEdit, &QLineEdit::returnPressed, this, &LoginWidget::onLoginButtonClicked);
}

void LoginWidget::onLoginButtonClicked() {
    QString username = usernameEdit->text().trimmed();
    QString password = passwordEdit->text();
    
    if (username.isEmpty()) {
        statusLabel->setText("Username cannot be empty");
        return;
    }
    
    if (password.isEmpty()) {
        statusLabel->setText("Password cannot be empty");
        return;
    }
    
    statusLabel->clear();
    emit loginClicked(username, password);
}

void LoginWidget::onRegisterButtonClicked() {
    QString username = usernameEdit->text().trimmed();
    QString password = passwordEdit->text();
    
    if (username.isEmpty()) {
        statusLabel->setText("Username cannot be empty");
        return;
    }
    
    if (password.isEmpty()) {
        statusLabel->setText("Password cannot be empty");
        return;
    }
    
    if (password.length() < 4) {
        statusLabel->setText("Password must be at least 4 characters");
        return;
    }
    
    statusLabel->clear();
    emit registerClicked(username, password);
}
