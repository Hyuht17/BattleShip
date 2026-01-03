#include "ConnectionWidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QMessageBox>

ConnectionWidget::ConnectionWidget(QWidget *parent)
    : QWidget(parent)
{
    setupUI();
}

void ConnectionWidget::setupUI() {
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
    
    // Connection form
    QGroupBox* connectionBox = new QGroupBox("Server Connection", this);
    QFormLayout* formLayout = new QFormLayout(connectionBox);
    
    hostEdit = new QLineEdit(this);
    hostEdit->setPlaceholderText("127.0.0.1 or example.com");
    hostEdit->setText("127.0.0.1");  // Default
    hostEdit->setMinimumWidth(250);
    formLayout->addRow("Host:", hostEdit);
    
    portEdit = new QLineEdit(this);
    portEdit->setPlaceholderText("8080");
    portEdit->setText("8080");  // Default
    formLayout->addRow("Port:", portEdit);
    
    mainLayout->addWidget(connectionBox);
    
    // Connect button
    connectButton = new QPushButton("Connect to Server", this);
    connectButton->setMinimumHeight(40);
    connectButton->setMinimumWidth(200);
    connect(connectButton, &QPushButton::clicked, this, &ConnectionWidget::onConnectButtonClicked);
    
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    buttonLayout->addWidget(connectButton);
    buttonLayout->addStretch();
    mainLayout->addLayout(buttonLayout);
    
    // Status label
    statusLabel = new QLabel(this);
    statusLabel->setAlignment(Qt::AlignCenter);
    statusLabel->setStyleSheet("color: gray; font-size: 12px;");
    statusLabel->setText("Enter server address and port to connect");
    mainLayout->addWidget(statusLabel);
    
    // Add stretch at bottom
    mainLayout->addStretch();
    
    // Enter key triggers connect
    connect(hostEdit, &QLineEdit::returnPressed, this, &ConnectionWidget::onConnectButtonClicked);
    connect(portEdit, &QLineEdit::returnPressed, this, &ConnectionWidget::onConnectButtonClicked);
}

void ConnectionWidget::onConnectButtonClicked() {
    QString host = hostEdit->text().trimmed();
    QString portStr = portEdit->text().trimmed();
    
    if (host.isEmpty()) {
        statusLabel->setStyleSheet("color: red;");
        statusLabel->setText("Host cannot be empty");
        return;
    }
    
    bool ok;
    int port = portStr.toInt(&ok);
    
    if (!ok || port <= 0 || port > 65535) {
        statusLabel->setStyleSheet("color: red;");
        statusLabel->setText("Invalid port number (1-65535)");
        return;
    }
    
    statusLabel->setStyleSheet("color: blue;");
    statusLabel->setText(QString("Connecting to %1:%2...").arg(host).arg(port));
    
    connectButton->setEnabled(false);
    
    emit connectClicked(host, port);
}
