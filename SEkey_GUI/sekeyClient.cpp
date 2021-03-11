/**
  ******************************************************************************
  * File Name          : sekeyClient.cpp
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

#include "sekeyclient.h"
#include <memory>
#include <ctime>
#define PORT 1235

void send_cereal(int s1, uint8_t* reply, uint8_t T, std::stringstream& ss, std::string& altmsg);
void send_text(int s1, const std::string& msg);

/* Template to read serialized data with Cereal library */
template <class T1>
static void read_serial(uint8_t* request, T1& inputparams){
    std::stringstream ss;
    uint32_t vsize = 0;
    memcpy(&vsize, request+L_OFFSET, L_SIZE);
    ss.write((char*)request+V_OFFSET, vsize);
    cereal::BinaryInputArchive iarchive(ss);
    iarchive(inputparams);
}

/* Template to serialize data with Cereal library and send them to the backend */
template <class T2>
void write_serial(int s1, uint8_t T, uint8_t* reply, T2& inputparams){
    std::stringstream ss;
    {
        cereal::BinaryOutputArchive oarchive(ss);
        oarchive(inputparams);
    }
    send_cereal(s1, reply, T, ss, inputparams.errmessage);
}

/* Initialize sekeyClient object */
sekeyClient::sekeyClient()
{
    this->sock = -1;
    this->valread = 0;
    memset((void*)(this->buffer), 0, BUFLEN);
}

/* Connect to the backend */
int sekeyClient::sekeyClient_init(){
    if((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        logger("Client: socket creation error");
        return -1;
    }
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0){
        logger("Client: Invalid address / Address not supported ");
        return -1;
    }
    int i = 0;
    while(true){
        i++;
        if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0){
            logger("Client: connection failed");
            if(i>=10){
                return -1;
            }
            usleep(200*1000);
        } else {
            logger("Client: connection done!");
            return 0;
        }
    }
}

/* Send data to the backend */
int sekeyClient::sekeyClient_txrx(sekeyCom command, void* params){
    //----------------------------------------------------------------
    //------------------------- LOGIN --------------------------------
    if(command == sekeyCom::login){
        Login_Params* inputparams = (Login_Params*)params;
        inputparams->force = true;
        write_serial<Login_Params>(sock, 1, buffer, *inputparams);
        memset(buffer, 0, BUFLEN);
        int valread = read(sock, buffer, BUFLEN);
        if(valread <= 5){ // minimum size is 1B for T and 4B for L
            return -1;
        } else {
            if(buffer[T_OFFSET] == 1){ // login is identified by T = 1
                read_serial<Login_Params>(buffer, *inputparams);
                if(inputparams->retvalue == 0){ // no errors on server side
                    return 0;
                }
            }
        }
        return -1;
    }

    /* Actions modifying users */
    if(command == sekeyCom::Add_User){
        SEkey_Add_User_Params* inputparams = (SEkey_Add_User_Params*)params;
        write_serial<SEkey_Add_User_Params>(sock, 2, buffer, *inputparams);
        memset(buffer, 0, BUFLEN);
        int valread = read(sock, buffer, BUFLEN);
        if(valread <= 5){
            return -1;
        }
        else {
            if(buffer[T_OFFSET] == 2){
                read_serial<SEkey_Add_User_Params>(buffer, *inputparams);
                if(inputparams->retvalue == 0){
                    return 0;
                }
            }
        }
        return -1;
    }
    if(command == sekeyCom::DeleteUser){
        SEkey_Del_User_Params* inputparams = (SEkey_Del_User_Params*)params;
        write_serial<SEkey_Del_User_Params>(sock, 3, buffer, *inputparams);
        memset(buffer, 0, BUFLEN);
        int valread = read(sock, buffer, BUFLEN);
        if(valread <= 5){
            return -1;
        }
        else {
            if(buffer[T_OFFSET] == 3){
                read_serial<SEkey_Del_User_Params>(buffer, *inputparams);
                if(inputparams->retvalue == 0){
                    return 0;
                }
            }
        }
        return -1;
    }
    if(command == sekeyCom::AddUserToGroup){
        SEkey_Add_User_Group_Params* inputparams = (SEkey_Add_User_Group_Params*)params;
        write_serial<SEkey_Add_User_Group_Params>(sock, 4, buffer, *inputparams);
        memset(buffer, 0, BUFLEN);
        int valread = read(sock, buffer, BUFLEN);
        if(valread <= 5){
            return -1;
        }
        else {
            if(buffer[T_OFFSET] == 4){
                read_serial<SEkey_Add_User_Group_Params>(buffer, *inputparams);
                if(inputparams->retvalue == 0){
                    return 0;
                }
            }
        }
        return -1;
    }
    if(command == sekeyCom::DeleteUserFromGroup){
        SEkey_Del_User_Group_Params* inputparams = (SEkey_Del_User_Group_Params*)params;
        write_serial<SEkey_Del_User_Group_Params>(sock, 5, buffer, *inputparams);
        memset(buffer, 0, BUFLEN);
        int valread = read(sock, buffer, BUFLEN);
        if(valread <= 5){
            return -1;
        }
        else {
            if(buffer[T_OFFSET] == 5){
                read_serial<SEkey_Del_User_Group_Params>(buffer, *inputparams);
                if(inputparams->retvalue == 0){
                    return 0;
                }
            }
        }
        return -1;
    }
    if(command == sekeyCom::ChangeUserName){
        SEkey_User_Change_Name_Params* inputparams = (SEkey_User_Change_Name_Params*)params;
        write_serial<SEkey_User_Change_Name_Params>(sock, 6, buffer, *inputparams);
        memset(buffer, 0, BUFLEN);
        int valread = read(sock, buffer, BUFLEN);
        if(valread <= 5){
            return -1;
        }
        else {
            if(buffer[T_OFFSET] == 6){
                read_serial<SEkey_User_Change_Name_Params>(buffer, *inputparams);
                if(inputparams->retvalue == 0){
                    return 0;
                }
            }
        }
        return -1;
    }

    /* Actions retrieving info about users */
    if(command == sekeyCom::GetAllUserInfo){
        SEkey_User_Get_Info_All_Params* inputparams = (SEkey_User_Get_Info_All_Params*)params;
        write_serial<SEkey_User_Get_Info_All_Params>(sock, 7, buffer, *inputparams);
        memset(buffer, 0, BUFLEN);
        int valread = read(sock, buffer, BUFLEN);
        if(valread <= 5){
            return -1;
        }
        else {
            if(buffer[T_OFFSET] == 7){
                read_serial<SEkey_User_Get_Info_All_Params>(buffer, *inputparams);
                if(inputparams->retvalue == 0){
                    return 0;
                }
            }
        }
        return -1;
    }
    if(command == sekeyCom::GetUserInfo){
        SEkey_User_Get_Info_Params* inputparams = (SEkey_User_Get_Info_Params*)params;
        write_serial<SEkey_User_Get_Info_Params>(sock, 8, buffer, *inputparams);
        memset(buffer, 0, BUFLEN);
        int valread = read(sock, buffer, BUFLEN);
        if(valread <= 5){ // minimum size is 1B for T and 4B for L
            return -1;
        }
        else {
            if(buffer[T_OFFSET] == 8){ // login is identified by T = 1
                read_serial<SEkey_User_Get_Info_Params>(buffer, *inputparams);
                if(inputparams->retvalue == 0){ // no errors on server side
                    return 0;
                }
            }
        }
        return -1;
    }

    /* Actions modifying groups */
    if(command == sekeyCom::AddGroup){
        SEkey_Add_Group_Params* inputparams = (SEkey_Add_Group_Params*)params;
        write_serial<SEkey_Add_Group_Params>(sock, 9, buffer, *inputparams);
        memset(buffer, 0, BUFLEN);
        int valread = read(sock, buffer, BUFLEN);
        if(valread <= 5){ // minimum size is 1B for T and 4B for L
            return -1;
        }
        else {
            if(buffer[T_OFFSET] == 9){ // login is identified by T = 1
                read_serial<SEkey_Add_Group_Params>(buffer, *inputparams);
                if(inputparams->retvalue == 0){ // no errors on server side
                    return 0;
                }
            }
        }
        return -1;
    }
    if(command == sekeyCom::DeleteGroup){
        SEkey_Del_Group_Params* inputparams = (SEkey_Del_Group_Params*)params;
        write_serial<SEkey_Del_Group_Params>(sock, 10, buffer, *inputparams);
        memset(buffer, 0, BUFLEN);
        int valread = read(sock, buffer, BUFLEN);
        if(valread <= 5){ // minimum size is 1B for T and 4B for L
            return -1;
        }
        else {
            if(buffer[T_OFFSET] == 10){ // login is identified by T = 1
                read_serial<SEkey_Del_Group_Params>(buffer, *inputparams);
                if(inputparams->retvalue == 0){ // no errors on server side
                    return 0;
                }
            }
        }
        return -1;
    }
    if(command == sekeyCom::ChangeGroupName){
        SEkey_Group_Change_Name_Params* inputparams = (SEkey_Group_Change_Name_Params*)params;
        write_serial<SEkey_Group_Change_Name_Params>(sock, 11, buffer, *inputparams);
        memset(buffer, 0, BUFLEN);
        int valread = read(sock, buffer, BUFLEN);
        if(valread <= 5){ // minimum size is 1B for T and 4B for L
            return -1;
        }
        else {
            if(buffer[T_OFFSET] == 11){ // login is identified by T = 1
                read_serial<SEkey_Group_Change_Name_Params>(buffer, *inputparams);
                if(inputparams->retvalue == 0){ // no errors on server side
                    return 0;
                }
            }
        }
        return -1;
    }
    if(command == sekeyCom::ChangeGroupMaxKeys){
        SEkey_Group_Change_MaxKeys_Params* inputparams = (SEkey_Group_Change_MaxKeys_Params*)params;
        write_serial<SEkey_Group_Change_MaxKeys_Params>(sock, 12, buffer, *inputparams);
        memset(buffer, 0, BUFLEN);
        int valread = read(sock, buffer, BUFLEN);
        if(valread <= 5){ // minimum size is 1B for T and 4B for L
            return -1;
        }
        else {
            if(buffer[T_OFFSET] == 12){ // login is identified by T = 1
                read_serial<SEkey_Group_Change_MaxKeys_Params>(buffer, *inputparams);
                if(inputparams->retvalue == 0){ // no errors on server side
                    return 0;
                }
            }
        }
        return -1;
    }
    if(command == sekeyCom::ChangeGroupCryptoperiod){
        SEkey_Group_Change_Cryptoperiod_Params* inputparams = (SEkey_Group_Change_Cryptoperiod_Params*)params;
        write_serial<SEkey_Group_Change_Cryptoperiod_Params>(sock, 13, buffer, *inputparams);
        memset(buffer, 0, BUFLEN);
        int valread = read(sock, buffer, BUFLEN);
        if(valread <= 5){ // minimum size is 1B for T and 4B for L
            return -1;
        }
        else {
            if(buffer[T_OFFSET] == 13){ // login is identified by T = 1
                read_serial<SEkey_Group_Change_Cryptoperiod_Params>(buffer, *inputparams);
                if(inputparams->retvalue == 0){ // no errors on server side
                    return 0;
                }
            }
        }
        return -1;
    }

    /* Actions retrieving info about groups */
    if(command == sekeyCom::getAllGroupInfo){
        SEkey_Group_Get_Info_All_Params* inputparams = (SEkey_Group_Get_Info_All_Params*)params;
        write_serial<SEkey_Group_Get_Info_All_Params>(sock, 14, buffer, *inputparams);
        memset(buffer, 0, BUFLEN);
        int valread = read(sock, buffer, BUFLEN);
        if(valread <= 5){
            return -1;
        }
        else {
            if(buffer[T_OFFSET] == 14){
                read_serial<SEkey_Group_Get_Info_All_Params>(buffer, *inputparams);
                if(inputparams->retvalue == 0){
                    return 0;
                }
            }
        }
        return -1;
    }
    if(command == sekeyCom::GetGroupInfo){
        SEkey_Group_Get_Info_Params* inputparams = (SEkey_Group_Get_Info_Params*)params;
        write_serial<SEkey_Group_Get_Info_Params>(sock, 15, buffer, *inputparams);
        memset(buffer, 0, BUFLEN);
        int valread = read(sock, buffer, BUFLEN);
        if(valread <= 5){
            return -1;
        }
        else {
            if(buffer[T_OFFSET] == 15){
                read_serial<SEkey_Group_Get_Info_Params>(buffer, *inputparams);
                if(inputparams->retvalue == 0){
                    return 0;
                }
            }
        }
        return -1;
    }

    /* Actions modifying keys */
    if(command == sekeyCom::AddKey){
        SEkey_Add_Key_Params* inputparams = (SEkey_Add_Key_Params*)params;
        write_serial<SEkey_Add_Key_Params>(sock, 16, buffer, *inputparams);
        memset(buffer, 0, BUFLEN);
        int valread = read(sock, buffer, BUFLEN);
        if(valread <= 5){ // minimum size is 1B for T and 4B for L
            return -1;
        }
        else {
            if(buffer[T_OFFSET] == 16){ // login is identified by T = 1
                read_serial<SEkey_Add_Key_Params>(buffer, *inputparams);
                if(inputparams->retvalue == 0){ // no errors on server side
                    return 0;
                }
            }
        }
        return -1;
    }
    if(command == sekeyCom::ActivateKey){
        SEkey_Activate_Key_Params* inputparams = (SEkey_Activate_Key_Params*)params;
        write_serial<SEkey_Activate_Key_Params>(sock, 17, buffer, *inputparams);
        memset(buffer, 0, BUFLEN);
        int valread = read(sock, buffer, BUFLEN);
        if(valread <= 5){ // minimum size is 1B for T and 4B for L
            return -1;
        }
        else {
            if(buffer[T_OFFSET] == 17){ // login is identified by T = 1
                read_serial<SEkey_Activate_Key_Params>(buffer, *inputparams);
                if(inputparams->retvalue == 0){ // no errors on server side
                    return 0;
                }
            }
        }
        return -1;
    }
    if(command == sekeyCom::ChangeKeyStatus){
        SEkey_Change_Key_Status_Params* inputparams = (SEkey_Change_Key_Status_Params*)params;
        write_serial<SEkey_Change_Key_Status_Params>(sock, 18, buffer, *inputparams);
        memset(buffer, 0, BUFLEN);
        int valread = read(sock, buffer, BUFLEN);
        if(valread <= 5){ // minimum size is 1B for T and 4B for L
            return -1;
        }
        else {
            if(buffer[T_OFFSET] == 18){ // login is identified by T = 1
                read_serial<SEkey_Change_Key_Status_Params>(buffer, *inputparams);
                if(inputparams->retvalue == 0){ // no errors on server side
                    return 0;
                }
            }
        }
        return -1;
    }
    if(command == sekeyCom::ChangeKeyName){
        SEkey_Key_Change_Name_Params* inputparams = (SEkey_Key_Change_Name_Params*)params;
        write_serial<SEkey_Key_Change_Name_Params>(sock, 19, buffer, *inputparams);
        memset(buffer, 0, BUFLEN);
        int valread = read(sock, buffer, BUFLEN);
        if(valread <= 5){ // minimum size is 1B for T and 4B for L
            return -1;
        }
        else {
            if(buffer[T_OFFSET] == 19){ // login is identified by T = 1
                read_serial<SEkey_Key_Change_Name_Params>(buffer, *inputparams);
                if(inputparams->retvalue == 0){ // no errors on server side
                    return 0;
                }
            }
        }
        return -1;
    }

    /* Actions retrieving info about keys */
    if(command == sekeyCom::GetAllKeyInfo){
        SEkey_Key_Get_Info_All_Params* inputparams = (SEkey_Key_Get_Info_All_Params*)params;
        write_serial<SEkey_Key_Get_Info_All_Params>(sock, 20, buffer, *inputparams);
        memset(buffer, 0, BUFLEN);
        int valread = read(sock, buffer, BUFLEN);
        if(valread <= 5){ // minimum size is 1B for T and 4B for L
            return -1;
        }
        else {
            if(buffer[T_OFFSET] == 20){ // login is identified by T = 1
                read_serial<SEkey_Key_Get_Info_All_Params>(buffer, *inputparams);
                if(inputparams->retvalue == 0){ // no errors on server side
                    return 0;
                }
            }
        }
        return -1;
    }
    if(command == sekeyCom::GetKeyInfo){
        SEkey_Key_Get_Info_Params* inputparams = (SEkey_Key_Get_Info_Params*)params;
        write_serial<SEkey_Key_Get_Info_Params>(sock, 21, buffer, *inputparams);
        memset(buffer, 0, BUFLEN);
        int valread = read(sock, buffer, BUFLEN);
        if(valread <= 5){ // minimum size is 1B for T and 4B for L
            return -1;
        }
        else {
            if(buffer[T_OFFSET] == 21){ // login is identified by T = 1
                read_serial<SEkey_Key_Get_Info_Params>(buffer, *inputparams);
                if(inputparams->retvalue == 0){ // no errors on server side
                    return 0;
                }
            }
        }
        return -1;
    }

    /* Actions retrieving info about most secure key */
    if(command == sekeyCom::FindKeyV1){
        SEkey_Find_Key_v1_Params* inputparams = (SEkey_Find_Key_v1_Params*)params;
        write_serial<SEkey_Find_Key_v1_Params>(sock, 22, buffer, *inputparams);
        memset(buffer, 0, BUFLEN);
        int valread = read(sock, buffer, BUFLEN);
        if(valread <= 5){ // minimum size is 1B for T and 4B for L
            return -1;
        }
        else {
            if(buffer[T_OFFSET] == 22){ // login is identified by T = 1
                read_serial<SEkey_Find_Key_v1_Params>(buffer, *inputparams);
                if(inputparams->retvalue == 0){ // no errors on server side
                    return 0;
                }
            }
        }
        return -1;
    }
    if(command == sekeyCom::FindKeyV2){
        SEkey_Find_Key_v2_Params* inputparams = (SEkey_Find_Key_v2_Params*)params;
        write_serial<SEkey_Find_Key_v2_Params>(sock, 23, buffer, *inputparams);
        memset(buffer, 0, BUFLEN);
        int valread = read(sock, buffer, BUFLEN);
        if(valread <= 5){ // minimum size is 1B for T and 4B for L
            return -1;
        }
        else {
            if(buffer[T_OFFSET] == 23){ // login is identified by T = 1
                read_serial<SEkey_Find_Key_v2_Params>(buffer, *inputparams);
                if(inputparams->retvalue == 0){ // no errors on server side
                    return 0;
                }
            }
        }
        return -1;
    }
    if(command == sekeyCom::FindKeyV3){
        SEkey_Find_Key_v3_Params* inputparams = (SEkey_Find_Key_v3_Params*)params;
        write_serial<SEkey_Find_Key_v3_Params>(sock, 24, buffer, *inputparams);
        memset(buffer, 0, BUFLEN);
        int valread = read(sock, buffer, BUFLEN);
        if(valread <= 5){ // minimum size is 1B for T and 4B for L
            return -1;
        }
        else {
            if(buffer[T_OFFSET] == 24){ // login is identified by T = 1
                read_serial<SEkey_Find_Key_v3_Params>(buffer, *inputparams);
                if(inputparams->retvalue == 0){ // no errors on server side
                    return 0;
                }
            }
        }
        return -1;
    }

    /* Other actions */
    if(command == sekeyCom::SEkeyGUIinfo){
        SEkey_GUI_info* inputparams = (SEkey_GUI_info*)params;
        write_serial<SEkey_GUI_info>(sock, 25, buffer, *inputparams);
        memset(buffer, 0, BUFLEN);
        int valread = read(sock, buffer, BUFLEN);
        if(valread <= 5){ // minimum size is 1B for T and 4B for L
            return -1;
        }
        else {
            if(buffer[T_OFFSET] == 25){ // login is identified by T = 1
                read_serial<SEkey_GUI_info>(buffer, *inputparams);
                if(inputparams->retvalue == 0){ // no errors on server side
                    return 0;
                }
            }
        }
        return -1;
    }
    if(command == sekeyCom::adminSEcubeInit){
        SEcubeInitParams* inputparams = (SEcubeInitParams*)params;
        write_serial<SEcubeInitParams>(sock, 26, buffer, *inputparams);
        memset(buffer, 0, BUFLEN);
        int valread = read(sock, buffer, BUFLEN);
        if(valread <= 5){ // minimum size is 1B for T and 4B for L
            return -1;
        }
        else {
            if(buffer[T_OFFSET] == 26){
                read_serial<SEcubeInitParams>(buffer, *inputparams);
                if(inputparams->retvalue == 0){ // no errors on server side
                    return 0;
                }
            }
        }
        return -1;
    }
    if(command == sekeyCom::InitUserSEcube){
        SEcubeInitParamsV2* inputparams = (SEcubeInitParamsV2*)params;
        write_serial<SEcubeInitParamsV2>(sock, 27, buffer, *inputparams);
        memset(buffer, 0, BUFLEN);
        int valread = read(sock, buffer, BUFLEN);
        if(valread <= 5){ // minimum size is 1B for T and 4B for L
            return -1;
        }
        else {
            if(buffer[T_OFFSET] == 27){
                read_serial<SEcubeInitParamsV2>(buffer, *inputparams);
                if(inputparams->retvalue == 0){ // no errors on server side
                    return 0;
                }
            }
        }
        return -1;
    }
    if(command == sekeyCom::DisconnectSEcube){
        send_text(this->sock, "disconnectsecube");
        memset(buffer, 0, BUFLEN);
        int valread = read(sock, buffer, BUFLEN);
        if(valread <= 5){ // minimum size is 1B for T and 4B for L
            return -1;
        }
        else {
            if(buffer[T_OFFSET] == 0){
                uint32_t len = 0;
                memcpy(&len, buffer+L_OFFSET, L_SIZE);
                if(len == 2 && strncmp((char*)buffer+V_OFFSET, "OK", 2) == 0){
                    return 0;
                }
            }
        }
        return -1;
    }
    return -1;
}

/* Send quit command to backend and close socket */
int sekeyClient::sekeyClient_close(){
    send_text(sock, "quit");
    return close(sock);
}

/* Send logout request to backend */
void sekeyClient::sekeyClient_logout(){
    send_text(sock, "logout");
}

/* Send text message to the backend */
void send_text(int s1, const std::string& msg){
    uint32_t msgsize = msg.size();
    if(msgsize + T_SIZE + L_SIZE <= BUFLEN){
        std::unique_ptr<uint8_t[]> buffer = std::make_unique<uint8_t[]>(msgsize);
        buffer[T_OFFSET] = 0;
        memcpy(buffer.get()+L_OFFSET, &msgsize, L_SIZE);
        memcpy(buffer.get()+V_OFFSET, msg.data(), msgsize);
        send(s1, buffer.get(), T_SIZE + L_SIZE + msgsize, 0);
    }
}

/* Send serialized data (with Cereal library) to the backend */
void send_cereal(int s1, uint8_t* reply, uint8_t T, std::stringstream& ss, std::string& altmsg){
    std::string s = ss.str();
    uint32_t replysize = s.size();
    if(replysize <= BUFLEN){
        reply[T_OFFSET] = T;
        memcpy(reply+L_OFFSET, &replysize, L_SIZE);
        memcpy(reply+V_OFFSET, s.c_str(), replysize);
        send(s1, reply, T_SIZE + L_SIZE + replysize, 0);
    } else {
        send_text(s1, altmsg);
    }
}

void logger(const std::string& msg){
#ifdef LOGGER_
    if(!msg.empty()){
        std::time_t result = std::time(nullptr);
        std::string tmp = std::ctime(&result);
        logfile << tmp.substr(0, tmp.length()-1) << " | " << msg << std::endl;
    }
#endif
}
