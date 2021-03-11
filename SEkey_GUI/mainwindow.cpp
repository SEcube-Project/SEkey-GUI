/**
  ******************************************************************************
  * File Name          : mainwindow.cpp
  * Description        :
  ******************************************************************************
  *
  * Copyright © 2016-present Blu5 Group <https://www.blu5group.com>
  *
  * This library is free software; you can redistribute it and/or
  * modify it under the terms of the GNU Lesser General Public
  * License as published by the Free Software Foundation; either
  * version 3 of the License, or (at your option) any later version.
  *
  * This library is distributed in the hope that it will be useful,
  * but WITHOUT ANY WARRANTY; without even the implied warranty of
  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
  * Lesser General Public License for more details.
  *
  * You should have received a copy of the GNU Lesser General Public
  * License along with this library; if not, see <https://www.gnu.org/licenses/>.
  *
  ******************************************************************************
  */

#include <QPixmap>
#include <QSize>
#include <QString>
#include <QMessageBox>
#include <QLineEdit>
#include <QGridLayout>
#include <QComboBox>
#include <QInputDialog>
#include <QDebug>
#include <QString>
#include "sekeyclient.h"
#include "viewwindow.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>

static std::map<QString, loginMode> loginModeMap = {
    {"User", loginMode::User},
    {"Admin", loginMode::Admin}
};

MainWindow::MainWindow(QWidget *parent, sekeyClient* client) : QMainWindow(parent), ui(new Ui::MainWindow){
    ui->setupUi(this);
    //---------------------------------------------------------------
    this->client = client;
    selected_dir = "";
    this->setWindowTitle("SEkey");
    this->setFixedSize(QSize(700, 300));
    QPixmap pix("../../resources/logo.png");
    int w = ui->label_logo->width();
    int h = ui->label_logo->height();

    QGridLayout *grid = new QGridLayout;
    grid->addWidget(ui->label_username, 0, 0);
    grid->addWidget(ui->comboBox_username, 0, 1);
    grid->addWidget(ui->label_password, 1, 0);
    grid->addWidget(ui->lineEdit_password, 1, 1);
    grid->addWidget(ui->loginButton, 2, 0 , 1, 2);
    grid->addWidget(ui->checkBox, 3, 0);
    ui->groupBox->setLayout(grid);

    ui->label_logo->setPixmap(pix.scaled(w,h,Qt::KeepAspectRatio));
    ui->lineEdit_password->setEchoMode(QLineEdit::Password);
    ui->lineEdit_password->setMaxLength(32);
    ui->comboBox_username->setCurrentIndex(1);

    connect(ui->loginButton, &QPushButton::clicked, this, &MainWindow::onLoginPress, Qt::UniqueConnection);
    connect(ui->checkBox, &QCheckBox::stateChanged, this, [this](){
        if(ui->lineEdit_password->echoMode() == QLineEdit::Password){
            ui->lineEdit_password->setEchoMode(QLineEdit::Normal);
        }
        else{
            ui->lineEdit_password->setEchoMode(QLineEdit::Password);
        }
    });

    /* This handles the setup of the SEkey folder for updates */
    connect(ui->actionSet_SEkey_update_folder, &QAction::triggered, this, [this](){
        QString dir = QFileDialog::getExistingDirectory(this, tr("Select SEkey update directory"),
                                                        "/home",
                                                        QFileDialog::ShowDirsOnly
                                                        | QFileDialog::ReadOnly);
        if(dir.isEmpty()){
            selected_dir = "";
        } else {
            selected_dir = dir;
        }
    });
    /* This handles the initialization of the SEcube of the KMS administrator */
    connect(ui->actionInitialize_admin_SEcube, &QAction::triggered, this, [this](){
        bool ok;
        QString PINadmin = QInputDialog::getText(this, tr("Admin PIN setup"), tr("Enter the new administrator PIN code:"), QLineEdit::Normal, "", &ok);
        if(!ok){
            return;
        }
        if(PINadmin.size() <= 0){
            QMessageBox::critical(this, "", "The PIN code cannot be empty!");
            return;
        }
        if(PINadmin.size() > 32){
            QMessageBox::critical(this, "", "The PIN code cannot be longer than 32 characters!");
            return;
        }
        QString PINuser = QInputDialog::getText(this, tr("User PIN setup"), tr("Enter the new user PIN code:"), QLineEdit::Normal, "", &ok);
        if(!ok){
            return;
        }
        if(PINuser.size() <= 0){
            QMessageBox::critical(this, "", "The PIN code cannot be empty!");
            return;
        }
        if(PINuser.size() > 32){
            QMessageBox::critical(this, "", "The PIN code cannot be longer than 32 characters!");
            return;
        }
        if(PINuser == PINadmin){
            QMessageBox::critical(this, "", "Admin PIN and user PIN cannot be the same!");
            return;
        }
        SEcubeInitParams inputparams;
        inputparams.adminPIN = {0};
        inputparams.userPIN = {0};
        memcpy(&(inputparams.adminPIN), PINadmin.toLocal8Bit(), sizeof(uint8_t) * PINadmin.size());
        memcpy(&(inputparams.userPIN), PINuser.toLocal8Bit(), sizeof(uint8_t) * PINuser.size());
        if(this->client->sekeyClient_txrx(sekeyCommand::adminSEcubeInit, &inputparams) == 0){
            QMessageBox::information(this, "", "The SEcube of the administrator has been successfully initialized for SEkey.\nPlease disconnect it and reconnect it before performing any other action.");
        } else {
            QMessageBox::critical(this, "", "Error initializing the SEcube of the administrator. Please try again.\nIf the error persists, try to erase the device and reprogram the firmware.");
        }
    });
    /* End of handler for the initialization of the SEcube of the administrator */
}

MainWindow::~MainWindow()
{
    delete ui;
}

/* Handler for login request from GUI to backend */
void MainWindow::onLoginPress() {
    QString password = ui->lineEdit_password->text();
    if(password.isEmpty()){
        QMessageBox::warning(this, "Login Error", "PIN cannot be empty!");
        return;
    }
    this->setCursor(QCursor(Qt::WaitCursor));
    QString username = ui->comboBox_username->currentText();
    Login_Params inputparams;
    if(loginModeMap[username] == loginMode::Admin){
        inputparams.type = SE3_ACCESS_ADMIN;
    } else {
        inputparams.type = SE3_ACCESS_USER;
    }
    inputparams.pin = {0};
    memcpy(&(inputparams.pin), password.toLocal8Bit(), sizeof(uint8_t) * password.size());
    if(!selected_dir.isEmpty()){
        inputparams.sekey_update_path = selected_dir.toStdString();
    } else {
        inputparams.sekey_update_path = "";
    }
    if(client->sekeyClient_txrx(sekeyCommand::login, &inputparams) == 0){
        selected_dir = "";
        logger("SEcube login OK");
        this->setCursor(QCursor(Qt::ArrowCursor));
        this->hide();
        viewwindow view(this, client, loginModeMap[username]);
        int ret = view.exec();
        if (ret == 0){
            QApplication::exit(0);
        }
        else if(ret == 1){
            QApplication::exit(1);
        }
    } else {
        this->setCursor(QCursor(Qt::ArrowCursor));
        logger("SEcube login error");
        QMessageBox::warning(this, "Login Error", QString::fromStdString(inputparams.errmessage));
    }
}
