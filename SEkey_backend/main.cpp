/**
  ******************************************************************************
  * File Name          : main.cpp
  * Description        : main of SEcube backend for SEkey GUI.
  ******************************************************************************
  *
  * Copyright ? 2016-present Blu5 Group <https://www.blu5group.com>
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

#include <fstream>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <ctime>
#include "GUI_Server_interface.h"

#define BUFLEN 1024*1024 // dimension of input/output buffer for communicating with the GUI (1 MB)

// offset and size of fields in requests and responses
#define T_OFFSET 0
#define T_SIZE 1
#define L_OFFSET 1
#define L_SIZE 4
#define V_OFFSET 5

int network(int listenPort);
void send_text(int s1, std::string msg);
void send_cereal(int s1, uint8_t* reply, uint8_t T, std::stringstream& ss, std::string& altmsg);
void inner_loop(bool& quit, uint8_t *request, uint8_t *reply, int s1, L1 *l1);

#define LOGGER_
void logger(const std::string& msg);

std::ofstream logfile("server_log.txt");

/* GUI REQUESTS
 *
 * GUI sends requests in TLV format.
 * T -> 1 byte, uint8_t value from 0 to 255 to identify each specific request
 * L -> 4 bytes, uint32_t value to tell how many bytes is V field
 * V -> L bytes, uint8_t* to be casted to stringstream and deserialized with Cereal library depending on T value
 *
 * */

/* SERVER RESPONSES
 *
 * The server (this program) sends responses to the GUI using the same TLV approach.
 * T -> 1 byte (uint8_t), it is the same value that the GUI sent
 * L -> 4 bytes (uint32_t), it is the dimension of the V field
 * V -> L bytes (uint8_t*), to be casted to stringstream and deserialized with Cereal library
 *
 * */

/* WARNING! T = 0 is reserved for requests and responses that contain exclusively "string-like" data, so Cereal is not really required in this case (i.e. the GUI sends "quit" to the server).
 * 			If T == 0, the GUI must not use Cereal...it simply needs to process the response in terms of the text that the server has sent (text-only responses are used to signal errors,
 * 			i.e. the server could not read the request coming from the GUI). */

void logger(const std::string& msg){
#ifdef LOGGER_
    if(!msg.empty()){
        std::time_t result = std::time(nullptr);
        std::string tmp = std::ctime(&result);
        logfile << tmp.substr(0, tmp.length()-1) << " | " << msg << std::endl;
    }
#endif
}

template <class T1>
void read_serial(uint8_t* request, T1& inputparams){
	std::stringstream ss;
	uint32_t vsize = 0;
	memcpy(&vsize, request+L_OFFSET, L_SIZE);
	ss.write((char*)request+V_OFFSET, vsize);
	cereal::BinaryInputArchive iarchive(ss);
	iarchive(inputparams);
}

template <class T2>
void write_serial(int s1, uint8_t T, uint8_t* reply, T2& inputparams){
	std::stringstream ss;
	{
		cereal::BinaryOutputArchive oarchive(ss);
		oarchive(inputparams);
	}
	send_cereal(s1, reply, T, ss, inputparams.errmessage);
	logger(inputparams.errmessage);
}

template <class T3>
void process_result(sekey_error rc, T3& inputparams){
	inputparams.reset();
	inputparams.retvalue = -1;
	switch(rc){
		case SEKEY_OK:
			inputparams.errmessage = "OK, no errors";
			inputparams.retvalue = 0;
			break;
		case SEKEY_ERR:
			inputparams.errmessage = "Error performing required operation";
			break;
		case SEKEY_ERR_AUTH:
			inputparams.errmessage = "SEkey authentication error";
			break;
		case SEKEY_USER_GROUP_DUP:
			inputparams.errmessage = "The selected group already contains the specified user";
			break;
		case SEKEY_KEY_DUP:
			inputparams.errmessage = "Key ID already used";
			break;
		case SEKEY_GROUP_NOT_FOUND:
			inputparams.errmessage = "Group not found in SEkey";
			break;
		case SEKEY_GROUP_DUP:
			inputparams.errmessage = "Group ID already used";
			break;
		case SEKEY_USER_DUP:
			inputparams.errmessage = "User ID already used";
			break;
		case SEKEY_USER_NOT_FOUND:
			inputparams.errmessage = "User not found in SEkey";
			break;
		case SEKEY_KEY_NOT_FOUND:
			inputparams.errmessage = "Key not found in SEkey";
			break;
		case SEKEY_ERR_PARAMS:
			inputparams.errmessage = "Wrong parameters";
			break;
		case SEKEY_UNCHANGED:
			inputparams.errmessage = "Error, try again";
			break;
		case SEKEY_RESTART:
			inputparams.errmessage = "Fatal error. Please restart the program...";
			break;
		case SEKEY_CORRUPTED:
			inputparams.errmessage = "Fatal error, the SEkey database appears to be corrupted.";
			break;
		case SEKEY_REPROG:
			inputparams.errmessage = "Error while initializing the SEcube. Device erase needed.";
			break;
		case SEKEY_RESTART_REPROG:
			inputparams.errmessage = "Error while initializing the SEcube. Device erase needed. Please restart also this program.";
			break;
		case SEKEY_BLOCKED:
			inputparams.errmessage = "Impossible to update SEkey with latest user data. SEkey is blocked until the update is completed. Try to restart this program.";
			break;
		case SEKEY_COMMON_GROUP_NOT_FOUND:
			inputparams.errmessage = "Impossible to find a common group for all users involved.";
			break;
		case SEKEY_UNSUPPORTED:
			inputparams.errmessage = "Unsupported key type. Be sure to use se_key_type::symmetric_data_encryption.";
			break;
		case SEKEY_INVALID_KEY:
			inputparams.errmessage = "Error, invalid key.";
			break;
		case SEKEY_INACTIVE_KEY:
			inputparams.errmessage = "Error, the key is not active.";
			break;
		case SEKEY_COMPROMISED_KEY:
			inputparams.errmessage = "Error, compromised key.";
			break;
		case SEKEY_DESTROYED_KEY:
			inputparams.errmessage = "Error, destroyed key.";
			break;
		case SEKEY_DEACTIVATED_KEY:
			inputparams.errmessage = "Error, deactivated key.";
			break;
		case SEKEY_PREACTIVE_KEY:
			inputparams.errmessage = "Error, the key is still in preactive status.";
			break;
		case SEKEY_SUSPENDED_KEY:
			inputparams.errmessage = "Error, the key is in suspended status.";
			break;
		default:
			inputparams.errmessage = "Unexpected error";
	}
}

int network(int listenPort){
	int s0 = socket(AF_INET, SOCK_STREAM, 0);
	if (s0 < 0) {
		std::string s("Error creating socket: ");
		logger(s.append(strerror(errno)));
		exit(1);
	}
	struct sockaddr_in address;
	memset(&address, 0, sizeof(struct sockaddr_in));
	address.sin_family = AF_INET;
	address.sin_port = htons(listenPort);
	address.sin_addr.s_addr = inet_addr("127.0.0.1");
	int res = bind(s0, (struct sockaddr*) &address, sizeof(address));
	if (res < 0) {
		std::string s("Error binding: ");
		logger(s.append(strerror(errno)));
		exit(1);
	}
	struct linger linger_opt = { 1, 0 }; // Linger active, timeout 0 (close the listen socket immediately at program termination)
	setsockopt(s0, SOL_SOCKET, SO_LINGER, &linger_opt, sizeof(linger_opt));
	res = listen(s0, 1);    // "1" is the maximal length of the queue
	if (res < 0) {
		std::string s("Error listening: ");
		logger(s.append(strerror(errno)));
		exit(1);
	}
	struct sockaddr_in peer_address;
	socklen_t peer_address_len;
	logger("Waiting for connection");
	int s1 = accept(s0, (struct sockaddr*) &peer_address, &peer_address_len);
	if (s1 < 0) {
		std::string s("Error accepting connection: ");
		logger(s.append(strerror(errno)));
		exit(1);
	}
	close(s0);
	return s1;
}

void send_text(int s1, std::string msg){
	uint32_t msgsize = msg.size();
	if(msgsize + T_SIZE + L_SIZE <= BUFLEN){
		std::unique_ptr<uint8_t[]> buf = std::make_unique<uint8_t[]>(msgsize);
		buf[T_OFFSET] = 0;
		memcpy(buf.get()+L_OFFSET, &msgsize, L_SIZE);
		memcpy(buf.get()+V_OFFSET, msg.data(), msgsize);
		send(s1, buf.get(), T_SIZE + L_SIZE + msgsize, 0);
	}
}

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

void inner_loop(bool& quit, uint8_t *request, uint8_t *reply, int s1, L1 *l1){
	bool logout = false;
	while (!quit && !logout) {
		memset(request, 0, BUFLEN);
		memset(reply, 0, BUFLEN);
		int res = read(s1, request, BUFLEN);
		if (res < 5) { // 4 bytes is the minimum size of a request because we need at least 1B for T and 4B for L
			std::string s("Error reading: ");
			logger(s.append(strerror(errno)));
			send_text(s1, "Server error reading GUI request");
			continue;
		}
		switch(request[T_OFFSET]){ // process request depending on type
			case 0:
			{
				uint32_t len = 0;
				memcpy(&len, request+L_OFFSET, L_SIZE);
				if(len == 4 && strncmp((char*)request+V_OFFSET, "quit", 4) == 0){ // client asked to quit
					logger("SEcube server quit");
					quit = true;
					sekey_stop();
					logger("SEkey stopped");
					try{
						l1->L1Logout();
					} catch (...) {
						// do nothing
					}
					logger("Logout");
				} else if (len == 6 && strncmp((char*)request+V_OFFSET, "logout", 6) == 0){
					logger("SEcube server logout");
					logout = true;
					sekey_stop();
					logger("SEkey stopped");
					try{
						l1->L1Logout();
					} catch (...) {
						// do nothing
					}
					logger("Logout");
				} else if(len == 16 && strncmp((char*)request+V_OFFSET, "disconnectsecube", 16) == 0){
					std::unique_ptr<L0> l0_ = std::make_unique<L0>();
					if(l0_->GetNumberDevices() == 1){
						send_text(s1, "OK");
					} else {
						logger("Still 2 SEcube connected");
						send_text(s1, "ERR");
					}
				} else {
					logger("Wrong request from client");
					send_text(s1, "Wrong request from client");
					continue;
				}
				break;
			}
			// case 1 not considered because it is related to login...so it must be handled as "default"
			case 2: // sekey_add_user()
			{
				SEkey_Add_User_Params inputparams;
				read_serial<SEkey_Add_User_Params>(request, inputparams);
				sekey_error rc = (sekey_error)sekey_add_user(inputparams.ID, inputparams.name);
				process_result<SEkey_Add_User_Params>(rc, inputparams);
				write_serial<SEkey_Add_User_Params>(s1, 2, reply, inputparams);
				break;
			}
			case 3: // sekey_delete_user()
			{
				SEkey_Del_User_Params inputparams;
				read_serial<SEkey_Del_User_Params>(request, inputparams);
				sekey_error rc = (sekey_error)sekey_delete_user(inputparams.ID);
				process_result<SEkey_Del_User_Params>(rc, inputparams);
				write_serial<SEkey_Del_User_Params>(s1, 3, reply, inputparams);
				break;
			}

			case 4: // sekey_add_user_group()
			{
				SEkey_Add_User_Group_Params inputparams;
				read_serial<SEkey_Add_User_Group_Params>(request, inputparams);
				sekey_error rc = (sekey_error)sekey_add_user_group(inputparams.userID, inputparams.groupID);
				process_result<SEkey_Add_User_Group_Params>(rc, inputparams);
				write_serial<SEkey_Add_User_Group_Params>(s1, 4, reply, inputparams);
				break;
			}

			case 5: // sekey_delete_user_group()
			{
				SEkey_Del_User_Group_Params inputparams;
				read_serial<SEkey_Del_User_Group_Params>(request, inputparams);
				sekey_error rc = (sekey_error)sekey_delete_user_group(inputparams.userID, inputparams.groupID);
				process_result<SEkey_Del_User_Group_Params>(rc, inputparams);
				write_serial<SEkey_Del_User_Group_Params>(s1, 5, reply, inputparams);
				break;
			}
			case 6: // sekey_user_change_name()
			{
				SEkey_User_Change_Name_Params inputparams;
				read_serial<SEkey_User_Change_Name_Params>(request, inputparams);
				sekey_error rc = (sekey_error)sekey_user_change_name(inputparams.ID, inputparams.name);
				process_result<SEkey_User_Change_Name_Params>(rc, inputparams);
				write_serial<SEkey_User_Change_Name_Params>(s1, 6, reply, inputparams);
				break;
			}
			case 7: // sekey_user_get_info_all()
			{
				SEkey_User_Get_Info_All_Params inputparams;
				read_serial<SEkey_User_Get_Info_All_Params>(request, inputparams);
				sekey_error rc = (sekey_error)sekey_user_get_info_all(&inputparams.users);
				process_result<SEkey_User_Get_Info_All_Params>(rc, inputparams);
				write_serial<SEkey_User_Get_Info_All_Params>(s1, 7, reply, inputparams);
				break;
			}
			case 8: // sekey_user_get_info()
			{
				SEkey_User_Get_Info_Params inputparams;
				read_serial<SEkey_User_Get_Info_Params>(request, inputparams);
				sekey_error rc = (sekey_error)sekey_user_get_info(inputparams.ID, &inputparams.user);
				process_result<SEkey_User_Get_Info_Params>(rc, inputparams);
				write_serial<SEkey_User_Get_Info_Params>(s1, 8, reply, inputparams);
				break;
			}
			case 9: // sekey_add_group()
			{
				SEkey_Add_Group_Params inputparams;
				read_serial<SEkey_Add_Group_Params>(request, inputparams);
				sekey_error rc = (sekey_error)sekey_add_group(inputparams.ID, inputparams.name, inputparams.policy);
				process_result<SEkey_Add_Group_Params>(rc, inputparams);
				write_serial<SEkey_Add_Group_Params>(s1, 9, reply, inputparams);
				break;
			}
			case 10: // sekey_delete_group()
			{
				SEkey_Del_Group_Params inputparams;
				read_serial<SEkey_Del_Group_Params>(request, inputparams);
				sekey_error rc = (sekey_error)sekey_delete_group(inputparams.ID);
				process_result<SEkey_Del_Group_Params>(rc, inputparams);
				write_serial<SEkey_Del_Group_Params>(s1, 10, reply, inputparams);
				break;
			}
			case 11: // sekey_group_change_name()
			{
				SEkey_Group_Change_Name_Params inputparams;
				read_serial<SEkey_Group_Change_Name_Params>(request, inputparams);
				sekey_error rc = (sekey_error)sekey_group_change_name(inputparams.ID, inputparams.name);
				process_result<SEkey_Group_Change_Name_Params>(rc, inputparams);
				write_serial<SEkey_Group_Change_Name_Params>(s1, 11, reply, inputparams);
				break;
			}
			case 12: // // sekey_group_change_max_keys()
			{
				SEkey_Group_Change_MaxKeys_Params inputparams;
				read_serial<SEkey_Group_Change_MaxKeys_Params>(request, inputparams);
				sekey_error rc = (sekey_error)sekey_group_change_max_keys(inputparams.ID, inputparams.maxkeys);
				process_result<SEkey_Group_Change_MaxKeys_Params>(rc, inputparams);
				write_serial<SEkey_Group_Change_MaxKeys_Params>(s1, 12, reply, inputparams);
				break;
			}
			case 13: // sekey_group_change_default_cryptoperiod()
			{
				SEkey_Group_Change_Cryptoperiod_Params inputparams;
				read_serial<SEkey_Group_Change_Cryptoperiod_Params>(request, inputparams);
				sekey_error rc = (sekey_error)sekey_group_change_default_cryptoperiod(inputparams.ID, inputparams.cryptoperiod);
				process_result<SEkey_Group_Change_Cryptoperiod_Params>(rc, inputparams);
				write_serial<SEkey_Group_Change_Cryptoperiod_Params>(s1, 13, reply, inputparams);
				break;
			}
			case 14: // sekey_group_get_info_all()
			{
				SEkey_Group_Get_Info_All_Params inputparams;
				read_serial<SEkey_Group_Get_Info_All_Params>(request, inputparams);
				sekey_error rc = (sekey_error)sekey_group_get_info_all(&inputparams.groups);
				process_result<SEkey_Group_Get_Info_All_Params>(rc, inputparams);
				write_serial<SEkey_Group_Get_Info_All_Params>(s1, 14, reply, inputparams);
				break;
			}
			case 15: // sekey_group_get_info()
			{
				SEkey_Group_Get_Info_Params inputparams;
				read_serial<SEkey_Group_Get_Info_Params>(request, inputparams);
				sekey_error rc = (sekey_error)sekey_group_get_info(inputparams.ID, &inputparams.group);
				process_result<SEkey_Group_Get_Info_Params>(rc, inputparams);
				write_serial<SEkey_Group_Get_Info_Params>(s1, 15, reply, inputparams);
				break;
			}
			case 16: // sekey_add_key()
			{
				SEkey_Add_Key_Params inputparams;
				read_serial<SEkey_Add_Key_Params>(request, inputparams);
				sekey_error rc = (sekey_error)sekey_add_key(inputparams.ID, inputparams.name, inputparams.owner, inputparams.cryptoperiod, se_key_type::symmetric_data_encryption);
				process_result<SEkey_Add_Key_Params>(rc, inputparams);
				write_serial<SEkey_Add_Key_Params>(s1, 16, reply, inputparams);
				break;
			}
			case 17: // sekey_activate_key()
			{
				SEkey_Activate_Key_Params inputparams;
				read_serial<SEkey_Activate_Key_Params>(request, inputparams);
				sekey_error rc = (sekey_error)sekey_activate_key(inputparams.ID);
				process_result<SEkey_Activate_Key_Params>(rc, inputparams);
				write_serial<SEkey_Activate_Key_Params>(s1, 17, reply, inputparams);
				break;
			}
			case 18: // sekey_key_change_status()
			{
				SEkey_Change_Key_Status_Params inputparams;
				read_serial<SEkey_Change_Key_Status_Params>(request, inputparams);
				sekey_error rc = (sekey_error)sekey_key_change_status(inputparams.ID, inputparams.status);
				process_result<SEkey_Change_Key_Status_Params>(rc, inputparams);
				write_serial<SEkey_Change_Key_Status_Params>(s1, 18, reply, inputparams);
				break;
			}
			case 19: // sekey_key_change_name()
			{
				SEkey_Key_Change_Name_Params inputparams;
				read_serial<SEkey_Key_Change_Name_Params>(request, inputparams);
				sekey_error rc = (sekey_error)sekey_key_change_name(inputparams.ID, inputparams.name);
				process_result<SEkey_Key_Change_Name_Params>(rc, inputparams);
				write_serial<SEkey_Key_Change_Name_Params>(s1, 19, reply, inputparams);
				break;
			}
			case 20: // sekey_key_get_info_all()
			{
				SEkey_Key_Get_Info_All_Params inputparams;
				read_serial<SEkey_Key_Get_Info_All_Params>(request, inputparams);
				sekey_error rc = (sekey_error)sekey_key_get_info_all(&inputparams.keys);
				process_result<SEkey_Key_Get_Info_All_Params>(rc, inputparams);
				write_serial<SEkey_Key_Get_Info_All_Params>(s1, 20, reply, inputparams);
				break;
			}
			case 21: // sekey_key_get_info()
			{
				SEkey_Key_Get_Info_Params inputparams;
				read_serial<SEkey_Key_Get_Info_Params>(request, inputparams);
				sekey_error rc = (sekey_error)sekey_key_get_info(inputparams.ID, &inputparams.key);
				process_result<SEkey_Key_Get_Info_Params>(rc, inputparams);
				write_serial<SEkey_Key_Get_Info_Params>(s1, 21, reply, inputparams);
				break;
			}
			case 22: // sekey_find_key_v1()
			{
				SEkey_Find_Key_v1_Params inputparams;
				read_serial<SEkey_Find_Key_v1_Params>(request, inputparams);
				sekey_error rc = (sekey_error)sekey_find_key_v1(inputparams.key, inputparams.destination, inputparams.keytype);
				process_result<SEkey_Find_Key_v1_Params>(rc, inputparams);
				write_serial<SEkey_Find_Key_v1_Params>(s1, 22, reply, inputparams);
				break;
			}
			case 23: // sekey_find_key_v2()
			{
				SEkey_Find_Key_v2_Params inputparams;
				read_serial<SEkey_Find_Key_v2_Params>(request, inputparams);
				sekey_error rc = (sekey_error)sekey_find_key_v2(inputparams.key, inputparams.destination, inputparams.keytype);
				process_result<SEkey_Find_Key_v2_Params>(rc, inputparams);
				write_serial<SEkey_Find_Key_v2_Params>(s1, 23, reply, inputparams);
				break;
			}
			case 24: // sekey_find_key_v3()
			{
				SEkey_Find_Key_v3_Params inputparams;
				read_serial<SEkey_Find_Key_v3_Params>(request, inputparams);
				sekey_error rc = (sekey_error)sekey_find_key_v3(inputparams.key, inputparams.destination, inputparams.keytype);
				process_result<SEkey_Find_Key_v3_Params>(rc, inputparams);
				write_serial<SEkey_Find_Key_v3_Params>(s1, 24, reply, inputparams);
				break;
			}
			case 25:
			{
				SEkey_GUI_info inputparams;
				read_serial<SEkey_GUI_info>(request, inputparams);
				getuserinfo(inputparams.ID, inputparams.name, inputparams.serialnumber, inputparams.accessmode);
				inputparams.retvalue = 0;
				inputparams.errmessage = "Ok, no errors.";
				write_serial<SEkey_GUI_info>(s1, 25, reply, inputparams);
				break;
			}
			case 27:
			{
				SEcubeInitParamsV2 inputparams;
				read_serial<SEcubeInitParamsV2>(request, inputparams);
				std::array<uint8_t, L1Parameters::Size::PIN> emptyPIN = {0};
				std::vector<std::array<uint8_t, L1Parameters::Size::PIN>> pins;
				pins.push_back(emptyPIN);
				sekey_error rc = (sekey_error)sekey_init_user_SEcube(inputparams.userID, inputparams.userPIN, inputparams.adminPIN, pins);
				process_result<SEcubeInitParamsV2>(rc, inputparams);
				write_serial<SEcubeInitParamsV2>(s1, 27, reply, inputparams);
				break;
			}
			default:
				logger("Wrong request from client");
				send_text(s1, "Wrong request from client");
				continue;
		}
	}
}

int main(){
	int listenPort = 1235;
	int s1 = network(listenPort);

	/* enter loop to manage SEkey GUI requests */
	std::unique_ptr<L0> l0;
	std::unique_ptr<L1> l1;
	bool quit = false;
	uint8_t request[BUFLEN] = {0};
	uint8_t reply[BUFLEN] = {0};
	int res = 0;

	while (!quit) {
		memset(request, 0, BUFLEN);
		memset(reply, 0, BUFLEN);
		res = read(s1, request, BUFLEN);
		if (res < 5) { // 4 bytes is the minimum size of a request because we need at least 1B for T and 4B for L
			std::string s("Error reading: ");
			logger(s.append(strerror(errno)));
			send_text(s1, "Server error reading GUI request");
			continue;
		} else if(request[T_OFFSET] == 0){ // process request depending on type
			uint32_t L = 0;
			memcpy(&L, request+L_OFFSET, L_SIZE);
			if(L == 4 && strncmp((char*)request+V_OFFSET, "quit", 4) == 0){ // client asked to quit
				logger("SEcube server quit");
				quit = true;
			} else {
				logger("Wrong request from client");
				send_text(s1, "Wrong request from client");
				continue;
			}
		} else if(request[T_OFFSET] == 26){ // request to initialize the SEcube of the administrator of SEkey
			l0 = std::make_unique<L0>();
			l1 = std::make_unique<L1>();
			SEcubeInitParams inputparams;
			read_serial<SEcubeInitParams>(request, inputparams);
			std::vector<std::array<uint8_t, L1Parameters::Size::PIN>> pins;
			std::array<uint8_t, L1Parameters::Size::PIN> emptyPIN = {0};
			pins.push_back(emptyPIN);
			uint8_t numDevices = l0->GetNumberDevices();
			if(numDevices != 1){
				inputparams.reset();
				inputparams.retvalue = -1;
				inputparams.errmessage = "Error while initializing SEcube for SEkey administrator!";
			} else {
				if(sekey_admin_init(*l1, pins, inputparams.userPIN, inputparams.adminPIN) != SEKEY_OK){
					inputparams.reset();
					inputparams.retvalue = -1;
					inputparams.errmessage = "Error while initializing SEcube for SEkey administrator!";
				} else {
					inputparams.reset();
					inputparams.retvalue = 0;
					inputparams.errmessage = "Ok, no errors.";
				}
			}
			write_serial<SEcubeInitParams>(s1, 26, reply, inputparams);
		} else if(request[T_OFFSET] == 1){ // T = 1 for login request
			// first check if the SEcube is correctly connected
			l0 = std::make_unique<L0>();
			l1 = std::make_unique<L1>();
			uint8_t numDevices = l0->GetNumberDevices();
			if (numDevices == 0) {
				logger("SEcube not connected");
				send_text(s1, "SEcube not connected");
				continue;
			}
			else if (numDevices > 1) {
				logger("Too many SEcube connected");
				send_text(s1, "Too many SEcube connected");
				continue;
			}
			// SEcube ok, deserialize input parameters coming from the GUI
			Login_Params inputparams;
			read_serial<Login_Params>(request, inputparams);

			// attempt login on the SEcube
			try {
				l1->L1Login(inputparams.pin, inputparams.type, inputparams.force);
			} catch(...) {
				logger("Error during login");
				inputparams.reset();
				inputparams.errmessage = "Error. The PIN might be wrong!";
				inputparams.retvalue = -1;
				write_serial<Login_Params>(s1, 1, reply, inputparams);
				continue;
			}
			logger("SEcube login OK");

			// check if there is a new sekey update path
			if(!inputparams.sekey_update_path.empty()){
				if(!set_sekey_update_path(inputparams.sekey_update_path, *l0.get(), l1.get())){
					logger("Error during SEkey setup");
					inputparams.reset();
					inputparams.errmessage = "Error during SEkey setup. New login required.";
					inputparams.retvalue = -1;
					try{
						l1->L1Logout();
					} catch (...){
						inputparams.errmessage = "Error during login";
						write_serial<Login_Params>(s1, 1, reply, inputparams);
						continue;
					}
					write_serial<Login_Params>(s1, 1, reply, inputparams);
					continue;
				}
			} else { // read input path from encrypted config file
				if(!read_sekey_update_path(*l0.get(), l1.get())){
					logger("Error during SEkey setup");
					inputparams.reset();
					inputparams.errmessage = "Error during SEkey setup. New login required.";
					inputparams.retvalue = -1;
					try{
						l1->L1Logout();
					} catch (...){
						inputparams.errmessage = "Error during login";
						write_serial<Login_Params>(s1, 1, reply, inputparams);
						continue;
					}
					write_serial<Login_Params>(s1, 1, reply, inputparams);
					continue;
				}
			}

			// start SEkey
			if(sekey_start(*l0, l1.get()) != 0){
				logger("Error starting SEkey");
				inputparams.reset();
				inputparams.errmessage = "Error starting SEkey. New login required.";
				inputparams.retvalue = -1;
				try{
					l1->L1Logout();
				} catch (...){
					inputparams.errmessage = "Error during login";
					write_serial<Login_Params>(s1, 1, reply, inputparams);
					continue;
				}
				write_serial<Login_Params>(s1, 1, reply, inputparams);
				continue;
			} else {
				logger("SEkey started");
				inputparams.reset();
				inputparams.errmessage = "OK, no errors.";
				inputparams.retvalue = 0;
				write_serial<Login_Params>(s1, 1, reply, inputparams);
			}

			// enter inner loop to handle requests about SEkey
			inner_loop(quit, request, reply, s1, l1.get());

		} else { // no other operation is allowed before login
			logger("Wrong request from client");
			send_text(s1, "Wrong request from client");
			continue;
		}
	}
	close(s1);
    return 0;
}
