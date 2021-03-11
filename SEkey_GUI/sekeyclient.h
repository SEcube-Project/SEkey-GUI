/**
  ******************************************************************************
  * File Name          : sekeyclient.h
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

#ifndef SEKEYCLIENT_H
#define SEKEYCLIENT_H

#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <fstream>

#include "../SEkey_backend/cereal/archives/binary.hpp"
#include "../SEkey_backend/cereal/types/array.hpp"
#include "../SEkey_backend/cereal/types/vector.hpp"
#include "../SEkey_backend/cereal/types/polymorphic.hpp"
#include "../SEkey_backend/cereal/cereal.hpp"
#include "../SEkey_backend/GUI_Server_interface.h"
//#include "../SEkey_backend/sekey/SEkey.h"
//#include "../SEkey_backend/sources/L1/L1 Base/L1_base.h"

#define BUFLEN 1024*1024 // dimension of input/output buffer for communicating with the SEcube SDK Server (1 MB)

// offset and size of fields in requests and responses
#define T_OFFSET 0
#define T_SIZE 1
#define L_OFFSET 1
#define L_SIZE 4
#define V_OFFSET 5

#define LOGGER_
extern std::ofstream logfile; // file where events are logged
void logger(const std::string& msg);

/* list of commands that can be sent to the backend */
typedef enum class sekeyCommand {
    login = 0,
    Add_User,
    DeleteUser,
    AddUserToGroup,
    DeleteUserFromGroup,
    ChangeUserName,
    GetUserInfo,
    GetAllUserInfo,
    AddGroup,
    DeleteGroup,
    ChangeGroupName,
    ChangeGroupMaxKeys,
    ChangeGroupCryptoperiod,
    GetGroupInfo,
    getAllGroupInfo,
    AddKey,
    ActivateKey,
    ChangeKeyStatus,
    ChangeKeyName,
    GetKeyInfo,
    GetAllKeyInfo,
    FindKeyV1,
    FindKeyV2,
    FindKeyV3,
    SEkeyGUIinfo,
    adminSEcubeInit,
    InitUserSEcube,
    DisconnectSEcube
} sekeyCom;

/* Object managing the interaction with the SEcube SDK Server */
class sekeyClient
{
private:
    int sock, valread;
    struct sockaddr_in serv_addr;
    uint8_t buffer[BUFLEN];
public:
    explicit sekeyClient();
    int sekeyClient_init();
    int sekeyClient_txrx(sekeyCom command, void* params);
    int sekeyClient_close();
    void sekeyClient_logout();
};

#endif // SEKEYCLIENT_H
