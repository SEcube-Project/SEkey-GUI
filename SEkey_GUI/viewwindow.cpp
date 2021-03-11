/**
  ******************************************************************************
  * File Name          : viewwindow.cpp
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

#include "viewwindow.h"
#include <QMessageBox>
#include <QInputDialog>
#include <QDebug>
#include <QDialog>
#include "options_view.h"
#include "../SEkey_backend/sekey/SEkey.h"

static void setup_actions(QComboBox* actions, loginMode mode);
std::string cryptoperiod_to_days(uint32_t n);

/* This maps a selected action on the GUI to an actual request to the SEcube SDK Server */
static std::map<QString, sekeyCommand> actionMap = {
    {"Add User", sekeyCom::Add_User},
    {"Delete User", sekeyCom::DeleteUser},
    {"Add User To Group", sekeyCom::AddUserToGroup},
    {"Delete User From Group", sekeyCom::DeleteUserFromGroup},
    {"Change User Name", sekeyCom::ChangeUserName},
    {"Get User Info", sekeyCom::GetUserInfo},
    {"Get All User Info", sekeyCom::GetAllUserInfo},
    {"Add Group", sekeyCom::AddGroup},
    {"Delete Group", sekeyCom::DeleteGroup},
    {"Change Group Name", sekeyCom::ChangeGroupName},
    {"Change Group Maximum Keys", sekeyCom::ChangeGroupMaxKeys},
    {"Change Group Cryptoperiod", sekeyCom::ChangeGroupCryptoperiod},
    {"Get Group Info", sekeyCom::GetGroupInfo},
    {"Get All Group Info", sekeyCom::getAllGroupInfo},
    {"Add Key", sekeyCom::AddKey},
    {"Activate Key", sekeyCom::ActivateKey},
    {"Change Key Status", sekeyCom::ChangeKeyStatus},
    {"Change Key Name", sekeyCom::ChangeKeyName},
    {"Get Key Info", sekeyCom::GetKeyInfo},
    {"Get All Key Info", sekeyCom::GetAllKeyInfo},
    {"Find Key V1", sekeyCom::FindKeyV1},
    {"Find Key V2", sekeyCom::FindKeyV2},
    {"Find Key V3", sekeyCom::FindKeyV3},
    {"Initialize user SEcube", sekeyCom::InitUserSEcube}
};

static std::map<se_key_status, QString> statusMapv1 = {
    {se_key_status::preactive, "Preactive"},
    {se_key_status::active, "Active"},
    {se_key_status::suspended, "Suspend"},
    {se_key_status::deactivated, "Deactivated"},
    {se_key_status::compromised, "Compromised"},
    {se_key_status::destroyed, "Destroyed",}
};

static std::map<QString, se_key_status> statusMapv2 = {
    {"Preactive", se_key_status::preactive},
    {"Active", se_key_status::active},
    {"Suspend", se_key_status::suspended},
    {"Deactivated", se_key_status::deactivated},
    {"Compromised", se_key_status::compromised},
    {"Destroyed", se_key_status::destroyed}
};

static std::map<uint8_t, QString> algorithmMap = {
    {L1Algorithms::Algorithms::AES, "AES"},
    {L1Algorithms::Algorithms::SHA256, "SHA256"},
    {L1Algorithms::Algorithms::HMACSHA256, "HMACSHA256"},
    {L1Algorithms::Algorithms::AES_HMACSHA256, "AES_HMACSHA256"},
    {L1Algorithms::Algorithms::ALGORITHM_MAX, "ALGORITHM_MAX"}
};

static std::map<QString, uint8_t> algorithmMapv2 = {
    {"AES", L1Algorithms::Algorithms::AES},
    {"SHA256", L1Algorithms::Algorithms::SHA256},
    {"HMACSHA256", L1Algorithms::Algorithms::HMACSHA256},
    {"AES_HMACSHA256", L1Algorithms::Algorithms::AES_HMACSHA256},
    {"ALGORITHM_MAX", L1Algorithms::Algorithms::ALGORITHM_MAX}
};

std::string cryptoperiod_to_days(uint32_t n){
    int day = n / (24 * 3600);
    n = n % (24 * 3600);
    int hour = n / 3600;
    n %= 3600;
    int minutes = n / 60 ;
    n %= 60;
    int seconds = n;
    std::string s = std::to_string(day) + " days, " + std::to_string(hour) + " hours, " + std::to_string(minutes) + " minutes, " + std::to_string(seconds) + " seconds.";
    return s;
}

viewwindow::viewwindow(MainWindow *parent, sekeyClient* client, loginMode mode)
{
    this->client = client;
    this->setWindowTitle("SEkey");
    this->setMinimumSize(400,400);
    grid = new QGridLayout(this);
    menubar = new QMenuBar(this);
    menubar->setMaximumHeight(30);

    creatMenus();
    creatActions();

    result_view = new QTreeView(this);

    command_gb = new QGroupBox(this);
    command_gb->setTitle("Action");
    command_gb->setMaximumHeight(100);
    command_gb_layout = new QVBoxLayout(this);
    command_gb->setLayout(command_gb_layout);
    commands_cb = new QComboBox(this);
    setup_actions(commands_cb, mode);
    run_pb = new QPushButton(this);
    run_pb->setText("Run");

    command_gb_layout->addWidget(commands_cb);
    command_gb_layout->addWidget(run_pb);

    grid->addWidget(menubar, 0, 0, 1, 2);
    grid->addWidget(result_view, 1, 0);
    grid->addWidget(command_gb, 1, 1);

    this->setLayout(grid);

    connect(this->run_pb, &QPushButton::clicked, this, &viewwindow::on_run_pb_press_wrapper);

    view_model = new QStandardItemModel(this);
    result_view->setModel(view_model);
    result_view->setHeaderHidden(true);
}

viewwindow::~viewwindow()
{
    disconnect(logoutAction);
    disconnect(quitAction);
    disconnect(aboutAction);
    delete this->grid;
    delete this->result_view;
    delete this->run_pb;
}

void viewwindow::creatActions(){

    logoutAction = new QAction(QString("Logout"), this);
    quitAction = new QAction(QString("Quit"), this);
    aboutAction = new QAction(QString("License"), this);
    detailsAction = new QAction(QString("Details"), this);

    secubeMenu->addAction(detailsAction);
    secubeMenu->addSeparator();
    secubeMenu->addAction(logoutAction);
    secubeMenu->addSeparator();
    secubeMenu->addAction(quitAction);
    aboutMenu->addAction(aboutAction);

    /* Show details about current login session on the SEcube */
    connect(detailsAction, &QAction::triggered, this,  [this](){
        SEkey_GUI_info param;
        int rv = client->sekeyClient_txrx(sekeyCom::SEkeyGUIinfo, (void*)&param);
        if(rv){ // error
            QMessageBox msgBox;
            msgBox.setText("Error retrieving information about current session.");
            msgBox.setStandardButtons(QMessageBox::Ok);
            msgBox.setDefaultButton(QMessageBox::Ok);
            msgBox.setIcon(QMessageBox::Warning);
            msgBox.exec();
        } else {
            std::string s1 = "Username: " + param.name + "\n\nUser ID: " + param.ID + "\n\nSEcube serial number: " + param.serialnumber + "\n\nAccess mode: " + param.accessmode;
            QMessageBox::information(this, "Session info", QString::fromStdString(s1));
        }
    });

    connect(logoutAction, &QAction::triggered, this,  [this](){
        this->done(1);
    });

    connect(quitAction, &QAction::triggered, this, [this](){
        this->done(0);
    });

    connect(aboutAction, &QAction::triggered, [](){
        const QString about_msg = "Copyright © 2016-present Blu5 Group\n<https://www.blu5group.com>\
\n\nThis is a free software. you can redistribute it and/or \
modify it under the terms of the GNU Lesser General Public \
License as published by the Free Software Foundation, either \
version 3 of the License, or (at your option) any later version.\n\n\
This software is distributed in the hope that it will be useful, \
but WITHOUT ANY WARRANTY, without even the implied warranty of \
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU \
Lesser General Public License for more details.\n\n\
You should have received a copy of the GNU Lesser General Public \
License along with this library; if not, see:\n<https://www.gnu.org/licenses/> \
\n\nFor the development of this application it has been used the QT community version\
which modules are available under the LGPL V3 and GPL V3 open source license. \
\n\nYou can find more information at the following link:\n"
"https://www.qt.io/download-open-source?hsCtaTracking=9f6a2170-a938-42df-a8e2-a9f0b1d6cdce%7C6cb0de4f-9bb5-4778-ab02-bfb62735f3e5";

     QMessageBox::about(nullptr, "Blu5 Group", about_msg);
    });
}

void viewwindow::creatMenus(){
    secubeMenu = menubar->addMenu("SEcube");
    aboutMenu = menubar->addMenu("About");
}

/* Defines which options are visible depending on the login on the SEcube */
static inline void setup_actions(QComboBox* actions, loginMode mode){
    if(mode == loginMode::Admin){
        actions->addItem(QString("Initialize user SEcube"));
        actions->addItem(QString("Add User"));
        actions->addItem(QString("Delete User"));
        actions->addItem(QString("Add User To Group"));
        actions->addItem(QString("Delete User From Group"));
        actions->addItem(QString("Change User Name"));
        actions->addItem(QString("Get User Info"));
        actions->addItem(QString("Get All User Info"));
        actions->addItem(QString("Add Group"));
        actions->addItem(QString("Delete Group"));
        actions->addItem(QString("Change Group Name"));
        actions->addItem(QString("Change Group Maximum Keys"));
        actions->addItem(QString("Change Group Cryptoperiod"));
        actions->addItem(QString("Get Group Info"));
        actions->addItem(QString("Get All Group Info"));
        actions->addItem(QString("Add Key"));
        actions->addItem(QString("Activate Key"));
        actions->addItem(QString("Change Key Status"));
        actions->addItem(QString("Change Key Name"));
        actions->addItem(QString("Get Key Info"));
        actions->addItem(QString("Get All Key Info"));
    } else {
        actions->addItem(QString("Get User Info"));
        actions->addItem(QString("Get All User Info"));
        actions->addItem(QString("Get Group Info"));
        actions->addItem(QString("Get All Group Info"));
        actions->addItem(QString("Get Key Info"));
        actions->addItem(QString("Get All Key Info"));
        actions->addItem(QString("Find Key V1"));
        actions->addItem(QString("Find Key V2"));
        actions->addItem(QString("Find Key V3"));
    }
    actions->setMaxVisibleItems(5);
    return;
}

/* Handlers for performing the action selected in the GUI */
void viewwindow::on_run_pb_press_wrapper(){
    this->setCursor(QCursor(Qt::WaitCursor));
    this->on_run_pb_press();
    this->setCursor(QCursor(Qt::ArrowCursor));
}
void viewwindow::on_run_pb_press() {
    QString action = commands_cb->currentText();

    /* Action to initialize the SEcube of a user */
    if(actionMap[action] == sekeyCom::InitUserSEcube){
        SEcubeInitParamsV2 param;
        bool ok = false;
        QString userId = QInputDialog::getText(this, tr("Add User"), tr("User ID:"),QLineEdit::Normal, "",&ok);
        if(!ok){
            return;
        }
        if(userId.isEmpty()){
            QMessageBox::critical(this, "", "The user ID cannot be empty!");
            return;
        } else {
            param.userID = userId.toStdString();
        }
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
        param.adminPIN = {0};
        param.userPIN = {0};
        memcpy(&(param.adminPIN), PINadmin.toLocal8Bit(), sizeof(uint8_t) * PINadmin.size());
        memcpy(&(param.userPIN), PINuser.toLocal8Bit(), sizeof(uint8_t) * PINuser.size());
        if(this->client->sekeyClient_txrx(sekeyCommand::InitUserSEcube, &param) == 0){
            QMessageBox::information(this, "SEkey GUI", "The SEcube of the user has been successfully initialized for SEkey.");
        } else {
            QMessageBox::critical(this, "SEkey GUI", "Error initializing the SEcube of the user. Please try again.\nIf the error persists, try to erase the device and reprogram the firmware.");
        }
        while(true){
            QMessageBox::information(this, "", "Please disconnect the SEcube of the user to proceed.");
            if(this->client->sekeyClient_txrx(sekeyCommand::DisconnectSEcube, nullptr) == 0){
                break;
            }
        }
    }

    /* Actions that modify users */
    if(actionMap[action] == sekeyCom::Add_User){
        SEkey_Add_User_Params param;
        bool ok = false;
        QString userId = QInputDialog::getText(this, tr("Add User"), tr("User ID:"),QLineEdit::Normal, "", &ok);
        if(!ok){
        	return;
        }
        if(userId.isEmpty()){
            QMessageBox::warning(this, "Error", "User ID cannot be empty!");
            return;
        }
        QString userName = QInputDialog::getText(this, tr("Add User"), tr("User Name:"),QLineEdit::Normal, "", &ok);
        if(!ok){
        	return;
        }
        if(userName.isEmpty()){
            QMessageBox::warning(this, "Error", "User name cannot be empty!");
            return;
        }
		param.ID = userId.toStdString();
        param.name = userName.toStdString();
        if(client->sekeyClient_txrx(sekeyCom::Add_User, (void*)&param) != 0){
            logger(param.errmessage);
            QMessageBox::warning(this, "Add User Error", QString::fromStdString(param.errmessage), QMessageBox::Abort);
        } else {
        	QMessageBox::information(this, "SEkey GUI", "User added correctly!");
        }
    }
    if(actionMap[action] == sekeyCom::DeleteUser){
        bool ok;
        QString userId = QInputDialog::getText(this, tr("Delete User"), tr("User ID:"),QLineEdit::Normal, "",&ok);
        if(!ok){
            return;
        }
        if(userId.isEmpty()){
            QMessageBox::warning(this, "Delete User", "User ID cannot be empty!");
            return;
        }
        SEkey_Del_User_Params param;
        param.ID = userId.toStdString();
        if(client->sekeyClient_txrx(sekeyCom::DeleteUser, (void*)&param) != 0){
            logger(param.errmessage);
            QMessageBox::warning(this, "Delete User", QString::fromStdString(param.errmessage), QMessageBox::Abort);
        } else {
            QMessageBox::information(this, "Delete User", "User deleted correctly!");
        }
    }
    if(actionMap[action] == sekeyCom::AddUserToGroup){
        bool ok = false;
        QString userId = QInputDialog::getText(this, tr("Add User To Group"), tr("User ID:"),QLineEdit::Normal, "",&ok);
        if(!ok){
            return;
        }
        if(userId.isEmpty()){
            QMessageBox::warning(this, "Add User To Group", "User ID cannot be empty!");
            return;
        }
        QString groupId = QInputDialog::getText(this, tr("Add User To Group"), tr("Group ID:"),QLineEdit::Normal, "",&ok);
        if(!ok){
            return;
        }
        if(groupId.isEmpty()){
            QMessageBox::warning(this, "Add User To Group", "Group ID cannot be empty!");
            return;
        }
        SEkey_Del_User_Group_Params param;
        param.userID = userId.toStdString();
        param.groupID = groupId.toStdString();
        if(client->sekeyClient_txrx(sekeyCom::AddUserToGroup, (void*)&param) != 0){
            logger(param.errmessage);
            QMessageBox::warning(this, "Add User To Group", QString::fromStdString(param.errmessage), QMessageBox::Abort);
        } else {
            QMessageBox::information(this, "Add User To Group", "User added correctly to group!");
        }
    }
    if(actionMap[action] == sekeyCom::DeleteUserFromGroup){
        bool ok = false;
        QString userId = QInputDialog::getText(this, tr("Delete User From Group"), tr("User ID:"),QLineEdit::Normal, "",&ok);
        if(!ok){
            return;
        }
        if(userId.isEmpty()){
            QMessageBox::warning(this, "Delete User From Group", "User ID cannot be empty!");
            return;
        }
        QString groupId = QInputDialog::getText(this, tr("Delete User From Group"), tr("Group ID:"),QLineEdit::Normal, "",&ok);
        if(!ok){
            return;
        }
        if(groupId.isEmpty()){
            QMessageBox::warning(this, "Delete User From Group", "Group ID cannot be empty!");
            return;
        }
        SEkey_Del_User_Group_Params param;
        param.userID = userId.toStdString();
        param.groupID = groupId.toStdString();
        if(client->sekeyClient_txrx(sekeyCom::DeleteUserFromGroup, (void*)&param) != 0){
            logger(param.errmessage);
            QMessageBox::warning(this, "Delete User From Group", QString::fromStdString(param.errmessage), QMessageBox::Abort);
        } else {
            QMessageBox::information(this, "Delete User From Group", "User deleted correctly from group!");
        }
    }
    if(actionMap[action] == sekeyCom::ChangeUserName){
        bool ok = false;
        QString userId = QInputDialog::getText(this, tr("Change User Name"), tr("User ID:"),QLineEdit::Normal, "",&ok);
        if(!ok){
            return;
        }
        if(userId.isEmpty()){
            QMessageBox::warning(this, "Change User Name", "User ID cannot be empty!");
            return;
        }
        QString userName = QInputDialog::getText(this, tr("Change User Name"), tr("User Name:"),QLineEdit::Normal, "",&ok);
        if(!ok){
            return;
        }
        if(userName.isEmpty()){
            QMessageBox::warning(this, "Change User Name", "User name cannot be empty!");
            return;
        }
        SEkey_User_Change_Name_Params param;
        param.name = userName.toStdString();
        param.ID = userId.toStdString();
        if(client->sekeyClient_txrx(sekeyCom::ChangeUserName, (void*)&param) != 0){
            logger(param.errmessage);
            QMessageBox::warning(this, "Change User Name", QString::fromStdString(param.errmessage), QMessageBox::Abort);
        } else {
            QMessageBox::information(this, "Change User Name", "User name changed correctly!");
        }
    }

    /* Actions to retrieve info about users */
    if(actionMap[action] == sekeyCom::GetAllUserInfo){
        SEkey_User_Get_Info_All_Params param;
        if(client->sekeyClient_txrx(sekeyCom::GetAllUserInfo, (void*)&param) != 0){
            logger(param.errmessage);
            QMessageBox::warning(this, "Get User Information Error", QString::fromStdString(param.errmessage), QMessageBox::Abort);
        } else {
            view_model->clear();
            QStandardItem *parentItem = view_model->invisibleRootItem();
            QStandardItem *userNameItem[param.users.size()];
            QStandardItem *userIdItem[param.users.size()];
            QStandardItem *userSerialnumberItem[param.users.size()];
            QStandardItem *userGroups[param.users.size()];
            QStandardItem *userNameItemValue[param.users.size()];
            QStandardItem *userIdItemValue[param.users.size()];
            QStandardItem *userSerialnumberItemValue[param.users.size()];
            QStandardItem *separatorItem[param.users.size()-1];
            for(unsigned long i=0; i<param.users.size(); i++){
                auto groups = param.users[i].GUI_getGroups();
                std::string tmps = "The user belongs to " + std::to_string(groups.size()) + " groups";
                if(groups.size() == 1){
                    tmps = "The user belongs to " + std::to_string(groups.size()) + " group";
                }
                userNameItem[i] = new QStandardItem(QString("User name"));
                userNameItem[i]->setForeground(QBrush(Qt::GlobalColor::blue));
                userIdItem[i] = new QStandardItem(QString("User ID"));
                userIdItem[i]->setForeground(QBrush(Qt::GlobalColor::blue));
                userSerialnumberItem[i] = new QStandardItem(QString("User SEcube Serial Number"));
                userSerialnumberItem[i]->setForeground(QBrush(Qt::GlobalColor::blue));
                userGroups[i] = new QStandardItem(QString::fromStdString(tmps));
                userGroups[i]->setForeground(QBrush(Qt::GlobalColor::blue));
                parentItem->appendRow(userNameItem[i]);
                parentItem->appendRow(userIdItem[i]);
                parentItem->appendRow(userSerialnumberItem[i]);
                parentItem->appendRow(userGroups[i]);
                userNameItemValue[i] = new QStandardItem(QString::fromStdString(param.users[i].GUI_getName()));
                userIdItemValue[i] = new QStandardItem(QString::fromStdString(param.users[i].GUI_getId()));
                userSerialnumberItemValue[i] = new QStandardItem(QString::fromStdString(param.users[i].GUI_getSn()));
                userNameItem[i]->appendRow(userNameItemValue[i]);
                userIdItem[i]->appendRow(userIdItemValue[i]);
                userSerialnumberItem[i]->appendRow(userSerialnumberItemValue[i]);
                QStandardItem *groupIdsOfUser[groups.size()];
                for(unsigned long j=0; j<groups.size(); j++){
                    groupIdsOfUser[j] = new QStandardItem(QString::fromStdString(groups[j]));
                    userGroups[i]->appendRow(groupIdsOfUser[j]);
                }
                if(i != (param.users.size()-1)){
                    separatorItem[i] = new QStandardItem(QString("--------------"));
                    parentItem->appendRow(separatorItem[i]);
                }
            }
        }
    }
    if(actionMap[action] == sekeyCom::GetUserInfo){
        SEkey_User_Get_Info_Params param;
        bool ok;
        QString text = QInputDialog::getText(this, tr("Get User Information"), tr("User ID:"),QLineEdit::Normal, "",&ok);
        if(!ok){
            return;
        }
        if(text.isEmpty()){
            QMessageBox::warning(this, "Error", "The user ID cannot be empty!", QMessageBox::Ok);
            return;
        }
        param.ID = text.toStdString();
        if(client->sekeyClient_txrx(sekeyCom::GetUserInfo, (void*)&param) != 0){
            logger(param.errmessage);
            QMessageBox::warning(this, "Get User Information Error", QString::fromStdString(param.errmessage), QMessageBox::Abort);
        } else {
            view_model->clear();
            std::vector<std::string> grouplist = param.user.GUI_getGroups();
            std::string tmps = "The user belongs to " + std::to_string(grouplist.size()) + " groups";
            if(grouplist.size() == 1){
                tmps = "The user belongs to " + std::to_string(grouplist.size()) + " group";
            }
            QStandardItem *parentItem = view_model->invisibleRootItem();
            QStandardItem *userNameItem = new QStandardItem(QString("User Name"));
            userNameItem->setForeground(QBrush(Qt::GlobalColor::blue));
            QStandardItem *userIdItem = new QStandardItem(QString("User ID"));
            userIdItem->setForeground(QBrush(Qt::GlobalColor::blue));
            QStandardItem *userSerialnumberItem = new QStandardItem(QString("User SEcube Serial Number"));
            userSerialnumberItem->setForeground(QBrush(Qt::GlobalColor::blue));
            QStandardItem *userGroups = new QStandardItem(QString::fromStdString(tmps));
            userGroups->setForeground(QBrush(Qt::GlobalColor::blue));
            parentItem->appendRow(userNameItem);
            parentItem->appendRow(userIdItem);
            parentItem->appendRow(userSerialnumberItem);
            parentItem->appendRow(userGroups);
            QStandardItem *userNameItemValue = new QStandardItem(QString::fromStdString(param.user.GUI_getName()));
            QStandardItem *userIdItemValue = new QStandardItem(QString::fromStdString(param.user.GUI_getId()));
            QStandardItem *userSerialnumberItemValue = new QStandardItem(QString::fromStdString(param.user.GUI_getSn()));
            userNameItem->appendRow(userNameItemValue);
            userIdItem->appendRow(userIdItemValue);
            userSerialnumberItem->appendRow(userSerialnumberItemValue);
            if(grouplist.size() > 0){
                QStandardItem *groups[grouplist.size()];
                int i= 0;
                for(std::string& s : grouplist){
                    groups[i] = new QStandardItem(QString::fromStdString(s));
                    userGroups->appendRow(groups[i]);
                    i++;
                }
            }
        }
    }

    /* Actions that modify groups */
    if(actionMap[action] == sekeyCom::AddGroup){
        bool ok = false;
        QString groupId = QInputDialog::getText(this, tr("Add group"), tr("Group ID:"),QLineEdit::Normal, "",&ok);
        if(!ok){
            return;
        }
        if(groupId.isEmpty()){
            QMessageBox::warning(this, "Add group", "Group ID cannot be empty!");
            return;
        }
        QString groupName = QInputDialog::getText(this, tr("Add Group"), tr("Group Name:"),QLineEdit::Normal, "",&ok);
        if(!ok){
            return;
        }
        if(groupName.isEmpty()){
            QMessageBox::warning(this, "Add group", "Group name cannot be empty!");
            return;
        }
        QString groupPolicyMaxKeys = QInputDialog::getText(this, tr("Add Group/Set Policy"), tr("Maximum number of keys:"),QLineEdit::Normal, "",&ok);
        if(!ok){
            return;
        }
        if(groupPolicyMaxKeys.isEmpty()){
            QMessageBox::warning(this, "Add group", "The maximum number of keys cannot be empty!");
            return;
        }
        QStringList options = {"AES", "HMACSHA256", "AES_HMACSHA256"};
        options_view view(nullptr,"Set Algorithm",  &options);
        view.exec();
        QString groupPolicDefaultCryptoperiod = QInputDialog::getText(this, tr("Add Group/Set Policy"), tr("Default Cryptoperiod (seconds):"),QLineEdit::Normal, "",&ok);
        if(!ok){
            return;
        }
        if(groupPolicDefaultCryptoperiod.isEmpty()){
            QMessageBox::warning(this, "Add group", "The cryptoperiod cannot be empty!");
            return;
        }
        group_policy mypolicy(groupPolicyMaxKeys.toUInt(), (uint32_t)algorithmMapv2[view.option_value], groupPolicDefaultCryptoperiod.toUInt());
        SEkey_Add_Group_Params param;
        param.ID = groupId.toStdString();
        param.name = groupName.toStdString();
        param.policy= mypolicy;
        if(client->sekeyClient_txrx(sekeyCom::AddGroup, (void*)&param) != 0){
            logger(param.errmessage);
            QMessageBox::warning(this, "Add group", QString::fromStdString(param.errmessage), QMessageBox::Abort);
        } else {
            QMessageBox::information(this, "Add group", "Group added correctly!");
        }
    }
    if(actionMap[action] == sekeyCom::DeleteGroup){
        bool ok = false;
        QString groupId = QInputDialog::getText(this, tr("Delete group"), tr("Group ID:"),QLineEdit::Normal, "",&ok);
        if(!ok){
            return;
        }
        if(groupId.isEmpty()){
            QMessageBox::warning(this, "Delete group", "Group ID cannot be empty!");
            return;
        }
        SEkey_Del_Group_Params param;
        param.ID = groupId.toStdString();
        if(client->sekeyClient_txrx(sekeyCom::DeleteGroup, (void*)&param) != 0){
            logger(param.errmessage);
            QMessageBox::warning(this, "Delete group", QString::fromStdString(param.errmessage), QMessageBox::Abort);
        } else {
            QMessageBox::information(this, "Delete group", "Group deleted correctly!");
        }
    }
    if(actionMap[action] == sekeyCom::ChangeGroupName){
        bool ok = false;
        QString groupId = QInputDialog::getText(this, tr("Change Group Name"), tr("Group ID:"),QLineEdit::Normal, "",&ok);
        if(!ok){
            return;
        }
        if(groupId.isEmpty()){
            QMessageBox::warning(this, "Change Group Name", "Group ID cannot be empty!");
            return;
        }
        QString groupName = QInputDialog::getText(this, tr("Change Group Name"), tr("Group Name:"),QLineEdit::Normal, "",&ok);
        if(!ok){
            return;
        }
        if(groupName.isEmpty()){
            QMessageBox::warning(this, "Change Group Name", "Group name cannot be empty!");
            return;
        }
        SEkey_Group_Change_Name_Params param;
        param.ID = groupId.toStdString();
        param.name = groupName.toStdString();
        if(client->sekeyClient_txrx(sekeyCom::ChangeGroupName, (void*)&param) != 0){
            logger(param.errmessage);
            QMessageBox::warning(this, "Change Group Name", QString::fromStdString(param.errmessage), QMessageBox::Abort);
        } else {
            QMessageBox::information(this, "Change Group Name", "Group name changed correctly!");
        }
    }
    if(actionMap[action] == sekeyCom::ChangeGroupMaxKeys){
        bool ok = false;
        QString groupId = QInputDialog::getText(this, tr("Change Group Maximum Keys"), tr("Group ID:"),QLineEdit::Normal, "",&ok);
        if(!ok){
            return;
        }
        if(groupId.isEmpty()){
            QMessageBox::warning(this, "Change Group Maximum Keys", "Group ID cannot be empty!");
            return;
        }
        QString maxkeys = QInputDialog::getText(this, tr("Change Group MaxKeys"), tr("Maximum number of keys:"),QLineEdit::Normal, "",&ok);
        if(!ok){
            return;
        }
        if(maxkeys.isEmpty()){
            QMessageBox::warning(this, "Change Group Maximum Keys", "The maximum number of keys cannot be empty!");
            return;
        }
        SEkey_Group_Change_MaxKeys_Params param;
        param.ID = groupId.toStdString();
        param.maxkeys = maxkeys.toUInt();
        if(client->sekeyClient_txrx(sekeyCom::ChangeGroupMaxKeys, (void*)&param) != 0){
            logger(param.errmessage);
            QMessageBox::warning(this, "Change Group Maximum Keys", QString::fromStdString(param.errmessage), QMessageBox::Abort);
        } else {
            QMessageBox::information(this, "Change Group Maximum Keys", "Maximum number of keys changed correctly!");
        }
    }
    if(actionMap[action] == sekeyCom::ChangeGroupCryptoperiod){
        bool ok = false;
        QString groupId = QInputDialog::getText(this, tr("Change Group Cryptoperiod"), tr("Group ID:"),QLineEdit::Normal, "",&ok);
        if(!ok){
            return;
        }
        if(groupId.isEmpty()){
            QMessageBox::warning(this, "Error", "Group ID cannot be empty!");
            return;
        }
        QString cryptoperiod = QInputDialog::getText(this, tr("Change Group Cryptoperiod"), tr("Cryptoperiod (seconds):"),QLineEdit::Normal, "",&ok);
        if(!ok){
            return;
        }
        if(cryptoperiod.isEmpty()){
            QMessageBox::warning(this, "Error", "Cryptoperiod cannot be empty!");
            return;
        }
        SEkey_Group_Change_Cryptoperiod_Params param;
        param.ID = groupId.toStdString();
        param.cryptoperiod = cryptoperiod.toUInt();
        if(client->sekeyClient_txrx(sekeyCom::ChangeGroupCryptoperiod, (void*)&param) != 0){
            logger(param.errmessage);
            QMessageBox::warning(this, "Change Group Cryptoperiod", QString::fromStdString(param.errmessage), QMessageBox::Abort);
        } else {
            QMessageBox::information(this, "Change Group Cryptoperiod", "Cryptoperiod changed correctly!");
        }
    }

    /* Actions to retrieve info about groups */
    if(actionMap[action] == sekeyCom::getAllGroupInfo){
        SEkey_Group_Get_Info_All_Params param;
        if(client->sekeyClient_txrx(sekeyCom::getAllGroupInfo, (void*)&param) != 0){
            logger(param.errmessage);
            QMessageBox::warning(this, "Get Group Information", QString::fromStdString(param.errmessage), QMessageBox::Abort);
        } else {
            view_model->clear();
            QStandardItem *parentItem = view_model->invisibleRootItem();
            QStandardItem *groupNameItem[param.groups.size()];
            QStandardItem *groupdItem[param.groups.size()];
            QStandardItem *groupUserCountItem[param.groups.size()];
            QStandardItem *groupKeyCountItem[param.groups.size()];
            QStandardItem *groupPolicyItem[param.groups.size()];
            QStandardItem *groupPolicyMaxkeysItem[param.groups.size()];
            QStandardItem *groupPolicyAlgorithmItem[param.groups.size()];
            QStandardItem *groupPolicyDefaultCryptoperiodItem[param.groups.size()];
            QStandardItem *groupNameItemValue[param.groups.size()];
            QStandardItem *groupdItemValue[param.groups.size()];
            QStandardItem *groupUserCountItemValue[param.groups.size()];
            QStandardItem *groupKeyCountItemValue[param.groups.size()];
            QStandardItem *groupPolicyMaxkeysItemValue[param.groups.size()];
            QStandardItem *groupPolicyAlgorithmItemValue[param.groups.size()];
            QStandardItem *groupPolicyDefaultCryptoperiodItemValue[param.groups.size()];
            QStandardItem *separatorItem[param.groups.size()-1];
            for(unsigned long i=0; i<param.groups.size(); i++){
                groupNameItem[i] = new QStandardItem(QString("Name"));
                groupNameItem[i]->setForeground(QBrush(Qt::GlobalColor::blue));
                groupdItem[i] = new QStandardItem(QString("ID"));
                groupdItem[i]->setForeground(QBrush(Qt::GlobalColor::blue));
                groupUserCountItem[i] = new QStandardItem(QString("Number Of Users"));
                groupUserCountItem[i]->setForeground(QBrush(Qt::GlobalColor::blue));
                groupKeyCountItem[i] = new QStandardItem(QString("Number Of Keys"));
                groupKeyCountItem[i]->setForeground(QBrush(Qt::GlobalColor::blue));
                groupPolicyItem[i] = new QStandardItem(QString("Policy"));
                groupPolicyItem[i]->setForeground(QBrush(Qt::GlobalColor::blue));
                groupPolicyMaxkeysItem[i] = new QStandardItem(QString("Maximum number of keys"));
                groupPolicyMaxkeysItem[i]->setForeground(QBrush(Qt::GlobalColor::blue));
                groupPolicyAlgorithmItem[i] = new QStandardItem(QString("Algorithm"));
                groupPolicyAlgorithmItem[i]->setForeground(QBrush(Qt::GlobalColor::blue));
                groupPolicyDefaultCryptoperiodItem[i] = new QStandardItem(QString("Default Cryptoperiod"));
                groupPolicyDefaultCryptoperiodItem[i]->setForeground(QBrush(Qt::GlobalColor::blue));
                parentItem->appendRow(groupNameItem[i]);
                parentItem->appendRow(groupdItem[i]);
                parentItem->appendRow(groupUserCountItem[i]);
                parentItem->appendRow(groupKeyCountItem[i]);
                parentItem->appendRow(groupPolicyItem[i]);
                groupPolicyItem[i]->appendRow(groupPolicyMaxkeysItem[i]);
                groupPolicyItem[i]->appendRow(groupPolicyAlgorithmItem[i]);
                groupPolicyItem[i]->appendRow(groupPolicyDefaultCryptoperiodItem[i]);
                groupNameItemValue[i] = new QStandardItem(QString::fromStdString(param.groups[i].GUI_getName()));
                groupdItemValue[i] = new QStandardItem(QString::fromStdString(param.groups[i].GUI_getId()));
                groupUserCountItemValue[i] = new QStandardItem(QString::number(param.groups[i].GUI_getUserCount()));
                groupKeyCountItemValue[i] = new QStandardItem(QString::number(param.groups[i].GUI_getKeyCount()));
                groupPolicyMaxkeysItemValue[i] = new QStandardItem(QString::number(param.groups[i].GUI_getPolicy().GUI_getMaxKeys()));
                groupPolicyAlgorithmItemValue[i] = new QStandardItem(algorithmMap[param.groups[i].GUI_getPolicy().GUI_getAlgorithm()]);
                std::string cryptostring = cryptoperiod_to_days(param.groups[i].GUI_getPolicy().GUI_getDefaultCryptoperiod());
                groupPolicyDefaultCryptoperiodItemValue[i] = new QStandardItem(QString::fromStdString(cryptostring));
                groupNameItem[i]->appendRow(groupNameItemValue[i]);
                groupdItem[i]->appendRow(groupdItemValue[i]);
                groupUserCountItem[i]->appendRow(groupUserCountItemValue[i]);
                groupKeyCountItem[i]->appendRow(groupKeyCountItemValue[i]);
                groupPolicyMaxkeysItem[i]->appendRow(groupPolicyMaxkeysItemValue[i]);
                groupPolicyAlgorithmItem[i]->appendRow(groupPolicyAlgorithmItemValue[i]);
                groupPolicyDefaultCryptoperiodItem[i]->appendRow(groupPolicyDefaultCryptoperiodItemValue[i]);
                if(i != (param.groups.size()-1)){
                    separatorItem[i] = new QStandardItem(QString("--------------"));
                    parentItem->appendRow(separatorItem[i]);
                }
            }
        }
    }
    if(actionMap[action] == sekeyCom::GetGroupInfo){
        bool ok;
        QString text = QInputDialog::getText(this, tr("Get Group Information"), tr("Group ID:"),QLineEdit::Normal, "",&ok);
        if(!ok){
            return;
        }
        if(text.isEmpty()){
            QMessageBox::warning(this, "Error", "Group ID cannot be empty!");
            return;
        }
        SEkey_Group_Get_Info_Params param;
        param.ID = text.toStdString();
        if(client->sekeyClient_txrx(sekeyCom::GetGroupInfo, (void*)&param) != 0){
            logger(param.errmessage);
            QMessageBox::warning(this, "Get Group Information", QString::fromStdString(param.errmessage), QMessageBox::Abort);
        } else {
            view_model->clear();
            QStandardItem *parentItem = view_model->invisibleRootItem();
            QStandardItem *groupNameItem = new QStandardItem(QString("Name"));
            groupNameItem->setForeground(QBrush(Qt::GlobalColor::blue));
            QStandardItem *groupdItem = new QStandardItem(QString("ID"));
            groupdItem->setForeground(QBrush(Qt::GlobalColor::blue));
            QStandardItem *groupUserCountItem = new QStandardItem(QString("Number Of Users"));
            groupUserCountItem->setForeground(QBrush(Qt::GlobalColor::blue));
            QStandardItem *groupKeyCountItem = new QStandardItem(QString("Number Of Keys"));
            groupKeyCountItem->setForeground(QBrush(Qt::GlobalColor::blue));
            QStandardItem *groupPolicyItem = new QStandardItem(QString("Policy"));
            groupPolicyItem->setForeground(QBrush(Qt::GlobalColor::blue));
            QStandardItem *groupPolicyMaxkeysItem = new QStandardItem(QString("Maximum number of keys"));
            groupPolicyMaxkeysItem->setForeground(QBrush(Qt::GlobalColor::blue));
            QStandardItem *groupPolicyAlgorithmItem = new QStandardItem(QString("Algorithm"));
            groupPolicyAlgorithmItem->setForeground(QBrush(Qt::GlobalColor::blue));
            QStandardItem *groupPolicyDefaultCryptoperiodItem = new QStandardItem(QString("Default Cryptoperiod"));
            groupPolicyDefaultCryptoperiodItem->setForeground(QBrush(Qt::GlobalColor::blue));
            parentItem->appendRow(groupNameItem);
            parentItem->appendRow(groupdItem);
            parentItem->appendRow(groupUserCountItem);
            parentItem->appendRow(groupKeyCountItem);
            parentItem->appendRow(groupPolicyItem);
            groupPolicyItem->appendRow(groupPolicyMaxkeysItem);
            groupPolicyItem->appendRow(groupPolicyAlgorithmItem);
            groupPolicyItem->appendRow(groupPolicyDefaultCryptoperiodItem);
            QStandardItem *groupNameItemValue = new QStandardItem(QString::fromStdString(param.group.GUI_getName()));
            QStandardItem *groupdItemValue = new QStandardItem(QString::fromStdString(param.group.GUI_getId()));
            QStandardItem *groupUserCountItemValue = new QStandardItem(QString::number(param.group.GUI_getUserCount()));
            QStandardItem *groupKeyCountItemValue = new QStandardItem(QString::number(param.group.GUI_getKeyCount()));
            QStandardItem *groupPolicyMaxkeysItemValue = new QStandardItem(QString::number(param.group.GUI_getPolicy().GUI_getMaxKeys()));
            QStandardItem *groupPolicyAlgorithmItemValue = new QStandardItem(algorithmMap[param.group.GUI_getPolicy().GUI_getAlgorithm()]);
            std::string cryptostring = cryptoperiod_to_days(param.group.GUI_getPolicy().GUI_getDefaultCryptoperiod());
            QStandardItem *groupPolicyDefaultCryptoperiodItemValue = new QStandardItem(QString::fromStdString(cryptostring));
            groupNameItem->appendRow(groupNameItemValue);
            groupdItem->appendRow(groupdItemValue);
            groupUserCountItem->appendRow(groupUserCountItemValue);
            groupKeyCountItem->appendRow(groupKeyCountItemValue);
            groupPolicyMaxkeysItem->appendRow(groupPolicyMaxkeysItemValue);
            groupPolicyAlgorithmItem->appendRow(groupPolicyAlgorithmItemValue);
            groupPolicyDefaultCryptoperiodItem->appendRow(groupPolicyDefaultCryptoperiodItemValue);
        }
    }

    /* Actions that modify keys */
    if(actionMap[action] == sekeyCom::AddKey){
        bool ok = false;
        QString keyId = QInputDialog::getText(this, tr("Add Key"), tr("Key ID:"),QLineEdit::Normal, "",&ok);
        if(!ok){
            return;
        }
        if(keyId.isEmpty()){
            QMessageBox::warning(this, "Add Key", "Key ID cannot be empty!");
            return;
        }
        QString keyName = QInputDialog::getText(this, tr("Add Key"), tr("Key Name:"),QLineEdit::Normal, "",&ok);
        if(!ok){
            return;
        }
        if(keyName.isEmpty()){
            QMessageBox::warning(this, "Add Key", "Key name cannot be empty!");
            return;
        }
        QString ownerName = QInputDialog::getText(this, tr("Add Key"), tr("Owner ID:"),QLineEdit::Normal, "",&ok);
        if(!ok){
            return;
        }
        if(ownerName.isEmpty()){
            QMessageBox::warning(this, "Add Key", "Owner ID cannot be empty!");
            return;
        }
        QString cryptoperiodKey = QInputDialog::getText(this, tr("Add Key"), tr("Cryptoperiod (seconds):"),QLineEdit::Normal, "",&ok);
        if(!ok){
            return;
        }
        if(cryptoperiodKey.isEmpty()){
            QMessageBox::warning(this, "Add Key", "Cryptoperiod cannot be empty!");
            return;
        }
        SEkey_Add_Key_Params param;
        param.ID = keyId.toStdString();
        param.name = keyName.toStdString();
        param.owner = ownerName.toStdString();
        param.cryptoperiod = cryptoperiodKey.toUInt();
        if(client->sekeyClient_txrx(sekeyCom::AddKey, (void*)&param) != 0){
            logger(param.errmessage);
            QMessageBox::warning(this, "Add Key", QString::fromStdString(param.errmessage), QMessageBox::Abort);
        } else {
            QMessageBox::information(this, "Add Key", "Key added correctly!");
        }
    }
    if(actionMap[action] == sekeyCom::ActivateKey){
        bool ok;
        QString keyId = QInputDialog::getText(this, tr("Activate Key"), tr("Enter the ID of the key:"),QLineEdit::Normal, "",&ok);
        if(!ok){
            return;
        }
        if(keyId.isEmpty()){
            QMessageBox::warning(this, "Activate Key", "Key ID cannot be empty!");
            return;
        }
        SEkey_Activate_Key_Params param;
        param.ID = keyId.toStdString();
        if(client->sekeyClient_txrx(sekeyCom::ActivateKey, (void*)&param) != 0){
            logger(param.errmessage);
            QMessageBox::warning(this, "Activate Key", QString::fromStdString(param.errmessage), QMessageBox::Abort);
        } else {
            QMessageBox::information(this, "Activate Key", "Key activated correctly!");
        }
    }
    if(actionMap[action] == sekeyCom::ChangeKeyStatus){
        bool ok = false;
        QString keyId = QInputDialog::getText(this, tr("Change Key Status"), tr("Enter the ID of the key:"),QLineEdit::Normal, "",&ok);
        if(!ok){
            return;
        }
        if(keyId.isEmpty()){
            QMessageBox::warning(this, "Change Key Status", "Key ID cannot be empty!");
            return;
        }
        QStringList options = {"Preactive", "Active", "Suspend", "Deactivated", "Compromised", "Destroyed"};
        options_view view(nullptr,"Select the new key status",  &options);
        view.exec();
        SEkey_Change_Key_Status_Params param;
        param.ID = keyId.toStdString();
        param.status = statusMapv2[view.option_value];
        if(client->sekeyClient_txrx(sekeyCom::ChangeKeyStatus, (void*)&param) != 0){
            logger(param.errmessage);
            QMessageBox::warning(this, "Change Key Status", QString::fromStdString(param.errmessage), QMessageBox::Abort);
        } else {
            QMessageBox::information(this, "Change Key Status", "Key status changed correctly!");
        }
    }
    if(actionMap[action] == sekeyCom::ChangeKeyName){
        bool ok = false;
        QString keyId = QInputDialog::getText(this, tr("Change Key Name"), tr("Key ID:"),QLineEdit::Normal, "",&ok);
        if(!ok){
            return;
        }
        if(keyId.isEmpty()){
            QMessageBox::warning(this, "Change Key Name Error", "Key ID cannot be empty!");
            return;
        }
        QString keyName = QInputDialog::getText(this, tr("Change Key Name"), tr("Key Name:"),QLineEdit::Normal, "",&ok);
        if(!ok){
            return;
        }
        if(keyName.isEmpty()){
            QMessageBox::warning(this, "Change Key Name Error", "Key name cannot be empty!");
            return;
        }
        SEkey_Key_Change_Name_Params param;
        param.ID = keyId.toStdString();
        param.name = keyName.toStdString();
        if(client->sekeyClient_txrx(sekeyCom::ChangeKeyName, (void*)&param) != 0){
            logger(param.errmessage);
            QMessageBox::warning(this, "Change Key Name Error", QString::fromStdString(param.errmessage), QMessageBox::Abort);
        } else {
            QMessageBox::information(this, "Change Key Name", "Name changed correctly!");
        }
    }

    /* Actions to retrieve info about keys */
    if(actionMap[action] == sekeyCom::GetAllKeyInfo){
        SEkey_Key_Get_Info_All_Params param;
        if(client->sekeyClient_txrx(sekeyCom::GetAllKeyInfo, (void*)&param) != 0){
            logger(param.errmessage);
            QMessageBox::warning(this, "Get Key Information Error", QString::fromStdString(param.errmessage), QMessageBox::Abort);
        } else {
            view_model->clear();
            QStandardItem *parentItem = view_model->invisibleRootItem();
            QStandardItem *keyNameItem[param.keys.size()];
            QStandardItem *keyIdItem[param.keys.size()];
            QStandardItem *keyOwnerItem[param.keys.size()];
            QStandardItem *keyStatusItem[param.keys.size()];
            QStandardItem *keyTypeItem[param.keys.size()];
            QStandardItem *keyAlgorithmItem[param.keys.size()];
            QStandardItem *keyLengthItem[param.keys.size()];
            QStandardItem *keyNameItemValue[param.keys.size()];
            QStandardItem *keyIdItemValue[param.keys.size()];
            QStandardItem *keyOwnerItemValue[param.keys.size()];
            QStandardItem *keyStatusItemValue[param.keys.size()];
            QStandardItem *keyTypeItemValue[param.keys.size()];
            QStandardItem *keyAlgorithmItemValue[param.keys.size()];
            QStandardItem *keyLengthItemValue[param.keys.size()];
            QStandardItem *separatorItem[param.keys.size()-1];
            for(unsigned long i=0; i<param.keys.size(); i++){
                keyNameItem[i] = new QStandardItem(QString("Key label"));
                keyNameItem[i]->setForeground(QBrush(Qt::GlobalColor::blue));
                keyIdItem[i] = new QStandardItem(QString("Key ID"));
                keyIdItem[i]->setForeground(QBrush(Qt::GlobalColor::blue));
                keyOwnerItem[i] = new QStandardItem(QString("Key owner"));
                keyOwnerItem[i]->setForeground(QBrush(Qt::GlobalColor::blue));
                keyStatusItem[i] = new QStandardItem(QString("Key status"));
                keyStatusItem[i]->setForeground(QBrush(Qt::GlobalColor::blue));
                keyTypeItem[i] = new QStandardItem(QString("Key type"));
                keyTypeItem[i]->setForeground(QBrush(Qt::GlobalColor::blue));
                keyAlgorithmItem[i] = new QStandardItem(QString("Supported algorithm"));
                keyAlgorithmItem[i]->setForeground(QBrush(Qt::GlobalColor::blue));
                keyLengthItem[i] = new QStandardItem(QString("Key length (bit)"));
                keyLengthItem[i]->setForeground(QBrush(Qt::GlobalColor::blue));
                parentItem->appendRow(keyNameItem[i]);
                parentItem->appendRow(keyIdItem[i]);
                parentItem->appendRow(keyOwnerItem[i]);
                parentItem->appendRow(keyStatusItem[i]);
                parentItem->appendRow(keyAlgorithmItem[i]);
                parentItem->appendRow(keyLengthItem[i]);
                keyNameItemValue[i] = new QStandardItem(QString::fromStdString(param.keys[i].GUI_getName()));
                keyIdItemValue[i] = new QStandardItem(QString::fromStdString(param.keys[i].GUI_getId()));
                keyOwnerItemValue[i] = new QStandardItem(QString::fromStdString(param.keys[i].GUI_getOwner()));
                keyStatusItemValue[i] = new QStandardItem(statusMapv1[param.keys[i].GUI_getStatus()]);
                keyTypeItemValue[i] = new QStandardItem(QString("Symmetric"));
                keyAlgorithmItemValue[i] = new QStandardItem(algorithmMap[param.keys[i].GUI_getAlgorithm()]);
                keyLengthItemValue[i] = new QStandardItem(QString::number(8*param.keys[i].GUI_getLength()));
                keyNameItem[i]->appendRow(keyNameItemValue[i]);
                keyIdItem[i]->appendRow(keyIdItemValue[i]);
                keyOwnerItem[i]->appendRow(keyOwnerItemValue[i]);
                keyStatusItem[i]->appendRow(keyStatusItemValue[i]);
                keyTypeItem[i]->appendRow(keyTypeItemValue[i]);
                keyAlgorithmItem[i]->appendRow(keyAlgorithmItemValue[i]);
                keyLengthItem[i]->appendRow(keyLengthItemValue[i]);
                if(i != (param.keys.size()-1)){
                    separatorItem[i] = new QStandardItem(QString("--------------"));
                    parentItem->appendRow(separatorItem[i]);
                }
            }
        }
    }
    if(actionMap[action] == sekeyCom::GetKeyInfo){
        bool ok;
        QString text = QInputDialog::getText(this, tr("Get Key Information"), tr("Key ID:"), QLineEdit::Normal, "", &ok);
        if(!ok){
            return;
        }
        if(text.isEmpty()){
            QMessageBox::warning(this, "Error", "Key ID cannot be empty!");
            return;
        }
        SEkey_Key_Get_Info_Params param;
        param.ID = text.toStdString();
        if(client->sekeyClient_txrx(sekeyCom::GetKeyInfo, (void*)&param) != 0){
            logger(param.errmessage);
            QMessageBox::warning(this, "Get Key Information Error", QString::fromStdString(param.errmessage), QMessageBox::Abort);
        } else {
            view_model->clear();
            QStandardItem *parentItem = view_model->invisibleRootItem();
            QStandardItem *keyNameItem = new QStandardItem(QString("Key label"));
            keyNameItem->setForeground(QBrush(Qt::GlobalColor::blue));
            QStandardItem *keyIdItem = new QStandardItem(QString("Key ID"));
            keyIdItem->setForeground(QBrush(Qt::GlobalColor::blue));
            QStandardItem *keyOwnerItem = new QStandardItem(QString("Key owner"));
            keyOwnerItem->setForeground(QBrush(Qt::GlobalColor::blue));
            QStandardItem *keyStatusItem = new QStandardItem(QString("Key status"));
            keyStatusItem->setForeground(QBrush(Qt::GlobalColor::blue));
            QStandardItem *keyTypeItem = new QStandardItem(QString("Key type"));
            keyTypeItem->setForeground(QBrush(Qt::GlobalColor::blue));
            QStandardItem *keyAlgorithmItem = new QStandardItem(QString("Supported algorithm"));
            keyAlgorithmItem->setForeground(QBrush(Qt::GlobalColor::blue));
            QStandardItem *keyLengthItem = new QStandardItem(QString("Key length (bit)"));
            keyLengthItem->setForeground(QBrush(Qt::GlobalColor::blue));
            parentItem->appendRow(keyNameItem);
            parentItem->appendRow(keyIdItem);
            parentItem->appendRow(keyOwnerItem);
            parentItem->appendRow(keyStatusItem);
            parentItem->appendRow(keyAlgorithmItem);
            parentItem->appendRow(keyLengthItem);
            QStandardItem *keyNameItemValue = new QStandardItem(QString::fromStdString(param.key.GUI_getName()));
            QStandardItem *keyIdItemValue = new QStandardItem(QString::fromStdString(param.key.GUI_getId()));
            QStandardItem *keyOwnerItemValue = new QStandardItem(QString::fromStdString(param.key.GUI_getOwner()));
            QStandardItem *keyStatusItemValue = new QStandardItem(statusMapv1[param.key.GUI_getStatus()]);
            QStandardItem *keyTypeItemValue = new QStandardItem(QString("Symmetric"));
            QStandardItem *keyAlgorithmItemValue = new QStandardItem(algorithmMap[param.key.GUI_getAlgorithm()]);
            QStandardItem *keyLengthItemValue = new QStandardItem(QString::number(8*param.key.GUI_getLength()));
            keyNameItem->appendRow(keyNameItemValue);
            keyIdItem->appendRow(keyIdItemValue);
            keyOwnerItem->appendRow(keyOwnerItemValue);
            keyStatusItem->appendRow(keyStatusItemValue);
            keyTypeItem->appendRow(keyTypeItemValue);
            keyAlgorithmItem->appendRow(keyAlgorithmItemValue);
            keyLengthItem->appendRow(keyLengthItemValue);
        }
    }

    /* Actions to retrieve the ID of the most secure key to be used given specific recipients (useful to test and debug the KMS) */
    if(actionMap[action] == sekeyCom::FindKeyV1){
        bool ok;
        QString userId = QInputDialog::getText(this, tr("SEkey GUI"), tr("Enter ID of the user who is the recipient of the crypto operation:"), QLineEdit::Normal, "", &ok);
        if(!ok){
            return;
        }
        if(userId.isEmpty()){
            QMessageBox::warning(this, "Error", "User ID cannot be empty!");
            return;
        }
        SEkey_Find_Key_v1_Params param;
        param.destination = userId.toStdString();
        param.keytype = se_key_type::symmetric_data_encryption;
        if(client->sekeyClient_txrx(sekeyCom::FindKeyV1, (void*)&param) != 0){
            logger(param.errmessage);
            QMessageBox::warning(this, "Error", QString::fromStdString(param.errmessage), QMessageBox::Abort);
        } else {
            view_model->clear();
            QStandardItem *parentItem = view_model->invisibleRootItem();
            QStandardItem *keyIdItem = new QStandardItem(QString("ID"));
            keyIdItem->setForeground(QBrush(Qt::GlobalColor::blue));
            parentItem->appendRow(keyIdItem);
            QStandardItem *keyIdItemValue = new QStandardItem(QString::fromStdString(param.key));
            keyIdItem->appendRow(keyIdItemValue);
        }
    }
    if(actionMap[action] == sekeyCom::FindKeyV2){
        bool ok;
        QString groupID = QInputDialog::getText(this, tr("SEkey GUI"), tr("Enter ID of the group who is the recipient of the crypto operation:"), QLineEdit::Normal, "", &ok);
        if(!ok){
            return;
        }
        if(groupID.isEmpty()){
            QMessageBox::warning(this, "Error", "Group ID cannot be empty!");
            return;
        }
        SEkey_Find_Key_v2_Params param;
        param.destination = groupID.toStdString();
        param.keytype = se_key_type::symmetric_data_encryption;
        if(client->sekeyClient_txrx(sekeyCom::FindKeyV2, (void*)&param) != 0){
            logger(param.errmessage);
            QMessageBox::warning(this, "Error", QString::fromStdString(param.errmessage), QMessageBox::Abort);
        } else {
            view_model->clear();
            QStandardItem *parentItem = view_model->invisibleRootItem();
            QStandardItem *keyIdItem = new QStandardItem(QString("ID"));
            keyIdItem->setForeground(QBrush(Qt::GlobalColor::blue));
            parentItem->appendRow(keyIdItem);
            QStandardItem *keyIdItemValue = new QStandardItem(QString::fromStdString(param.key));
            keyIdItem->appendRow(keyIdItemValue);
        }
    }
    if(actionMap[action] == sekeyCom::FindKeyV3){
        SEkey_Find_Key_v3_Params param;
        bool ok;
        QRegExp sepExp(",");
        QString userIds_con = QInputDialog::getText(this, tr("SEkey GUI"), tr("Enter the IDs of the users who are recipient of the crypto operation:"), QLineEdit::Normal, "comma seperated values", &ok);
        if(!ok){
            return;
        }
        QStringList userIds = userIds_con.split(sepExp);
        QStringListIterator userIdsIterator(userIds);
        while(userIdsIterator.hasNext()){
            param.destination.insert(param.destination.end(), userIdsIterator.next().toStdString());
        }
        if(param.destination.size() == 0){
            QMessageBox::warning(this, "Error", "User IDs cannot be empty!");
            return;
        }
        param.keytype = se_key_type::symmetric_data_encryption;
        if(client->sekeyClient_txrx(sekeyCom::FindKeyV3, (void*)&param) != 0){
            logger(param.errmessage);
            QMessageBox::warning(this, "Error", QString::fromStdString(param.errmessage), QMessageBox::Abort);
        }
        else{
            view_model->clear();
            QStandardItem *parentItem = view_model->invisibleRootItem();
            QStandardItem *keyIdItem = new QStandardItem(QString("ID"));
            keyIdItem->setForeground(QBrush(Qt::GlobalColor::blue));
            parentItem->appendRow(keyIdItem);
            QStandardItem *keyIdItemValue = new QStandardItem(QString::fromStdString(param.key));
            keyIdItem->appendRow(keyIdItemValue);
        }
    }

}
