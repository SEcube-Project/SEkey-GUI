/**
  ******************************************************************************
  * File Name          : GUI_Server_interface.h
  * Description        : data structures for communication with SEkey GUI
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

#ifndef GUI_SERVER_INTERFACE_H_
#define GUI_SERVER_INTERFACE_H_

#include "cereal/archives/binary.hpp" // use "binary.hpp" instead of "portable_binary.hpp" if endianness is not important
#include "cereal/types/array.hpp"
#include "cereal/types/vector.hpp"
#include "cereal/types/polymorphic.hpp"
#include "cereal/cereal.hpp"
#include "sources/L1/L1 Base/L1_base.h"
#include "sekey/SEkey.h"

/* From Cereal library docs: When using a binary archive and a file stream (std::fstream), remember to specify the binary flag (std::ios::binary)
 * when constructing the stream. This prevents the stream from interpreting your data as ASCII characters and altering them.*/

// T = 1
struct Login_Params{
	int retvalue; // this holds the result of the API called on the server
	std::string errmessage; // this holds an error message or exception message
	std::array<uint8_t, L1Parameters::Size::PIN> pin;
	se3_access_type type;
	bool force;
	std::string sekey_update_path;
	void reset(){
		retvalue = 0;
		errmessage.clear();
		pin = {0};
		type = SE3_ACCESS_NONE;
		sekey_update_path.clear();
	}
	// This method lets cereal know which data members to serialize
	template <class Archive>
	void serialize(Archive& archive){
		archive(retvalue, errmessage, pin, type, force, sekey_update_path); // serialize things by passing them to the archive
	}
};

// sekey_add_user()
struct SEkey_Add_User_Params{ // T = 2
	int retvalue;
	std::string errmessage;
	std::string ID;
	std::string name;
	void reset(){
		retvalue = 0;
		errmessage.clear();
		ID.clear();
		name.clear();
	}
	template <class Archive>
	void serialize(Archive& archive){
		archive(retvalue, errmessage, ID, name);
	}
};

// sekey_delete_user()
struct SEkey_Del_User_Params{ // T = 3
	int retvalue;
	std::string errmessage;
	std::string ID;
	void reset(){
		retvalue = 0;
		errmessage.clear();
		ID.clear();
	}
	template <class Archive>
	void serialize(Archive& archive){
		archive(retvalue, errmessage, ID);
	}
};

// sekey_add_user_group()
struct SEkey_Add_User_Group_Params{ // T = 4
	int retvalue;
	std::string errmessage;
	std::string userID;
	std::string groupID;
	void reset(){
		retvalue = 0;
		errmessage.clear();
		userID.clear();
		groupID.clear();
	}
	template <class Archive>
	void serialize(Archive& archive){
		archive(retvalue, errmessage, userID, groupID);
	}
};

// sekey_delete_user_group()
struct SEkey_Del_User_Group_Params{ // T = 5
	int retvalue;
	std::string errmessage;
	std::string userID;
	std::string groupID;
	void reset(){
		retvalue = 0;
		errmessage.clear();
		userID.clear();
		groupID.clear();
	}
	template <class Archive>
	void serialize(Archive& archive){
		archive(retvalue, errmessage, userID, groupID);
	}
};

// sekey_user_change_name()
struct SEkey_User_Change_Name_Params{ // T = 6
	int retvalue;
	std::string errmessage;
	std::string ID;
	std::string name;
	void reset(){
		retvalue = 0;
		errmessage.clear();
		ID.clear();
		name.clear();
	}
	template <class Archive>
	void serialize(Archive& archive){
		archive(retvalue, errmessage, ID, name);
	}
};

// sekey_user_get_info_all()
struct SEkey_User_Get_Info_All_Params{ // T = 7
	int retvalue;
	std::string errmessage;
	std::vector<se_user> users;
	void reset(){
		retvalue = 0;
		errmessage.clear();
	}
	template <class Archive>
	void serialize(Archive& archive){
		archive(retvalue, errmessage, users);
	}
};

// sekey_user_get_info()
struct SEkey_User_Get_Info_Params{ // T = 8
	int retvalue;
	std::string errmessage;
	std::string ID;
	se_user user;
	void reset(){
		retvalue = 0;
		errmessage.clear();
		ID.clear();
	}
	template <class Archive>
	void serialize(Archive& archive){
		archive(retvalue, errmessage, ID, user);
	}
};

// sekey_add_group()
struct SEkey_Add_Group_Params{ // T = 9
	int retvalue;
	std::string errmessage;
	std::string ID;
	std::string name;
	group_policy policy;
	void reset(){
		retvalue = 0;
		errmessage.clear();
		ID.clear();
		name.clear();
	}
	template <class Archive>
	void serialize(Archive& archive){
		archive(retvalue, errmessage, ID, name, policy);
	}
};

// sekey_delete_group()
struct SEkey_Del_Group_Params{ // T = 10
	int retvalue;
	std::string errmessage;
	std::string ID;
	void reset(){
		retvalue = 0;
		errmessage.clear();
		ID.clear();
	}
	template <class Archive>
	void serialize(Archive& archive){
		archive(retvalue, errmessage, ID);
	}
};

// sekey_group_change_name()
struct SEkey_Group_Change_Name_Params{ // T = 11
	int retvalue;
	std::string errmessage;
	std::string ID;
	std::string name;
	void reset(){
		retvalue = 0;
		errmessage.clear();
		ID.clear();
		name.clear();
	}
	template <class Archive>
	void serialize(Archive& archive){
		archive(retvalue, errmessage, ID, name);
	}
};

// sekey_group_change_max_keys()
struct SEkey_Group_Change_MaxKeys_Params{ // T = 12
	int retvalue;
	std::string errmessage;
	std::string ID;
	uint32_t maxkeys;
	void reset(){
		retvalue = 0;
		errmessage.clear();
		ID.clear();
		maxkeys = 0;
	}
	template <class Archive>
	void serialize(Archive& archive){
		archive(retvalue, errmessage, ID, maxkeys);
	}
};

// sekey_group_change_default_cryptoperiod()
struct SEkey_Group_Change_Cryptoperiod_Params{ // T = 13
	int retvalue;
	std::string errmessage;
	std::string ID;
	uint32_t cryptoperiod;
	void reset(){
		retvalue = 0;
		errmessage.clear();
		ID.clear();
		cryptoperiod = 0;
	}
	template <class Archive>
	void serialize(Archive& archive){
		archive(retvalue, errmessage, ID, cryptoperiod);
	}
};

// sekey_group_get_info_all()
struct SEkey_Group_Get_Info_All_Params{ // T = 14
	int retvalue;
	std::string errmessage;
	std::vector<se_group> groups;
	void reset(){
		retvalue = 0;
		errmessage.clear();
	}
	template <class Archive>
	void serialize(Archive& archive){
		archive(retvalue, errmessage, groups);
	}
};

// sekey_group_get_info()
struct SEkey_Group_Get_Info_Params{ // T = 15
	int retvalue;
	std::string errmessage;
	std::string ID;
	se_group group;
	void reset(){
		retvalue = 0;
		errmessage.clear();
		ID.clear();
	}
	template <class Archive>
	void serialize(Archive& archive){
		archive(retvalue, errmessage, ID, group);
	}
};

// sekey_add_key()
struct SEkey_Add_Key_Params{ // T = 16
	int retvalue;
	std::string errmessage;
	std::string ID;
	std::string name;
	std::string owner;
	uint32_t cryptoperiod;
	void reset(){
		retvalue = 0;
		errmessage.clear();
		ID.clear();
		owner.clear();
		cryptoperiod = 0;
	}
	template <class Archive>
	void serialize(Archive& archive){
		archive(retvalue, errmessage, ID, name, owner, cryptoperiod);
	}
};

// sekey_activate_key()
struct SEkey_Activate_Key_Params{ // T = 17
	int retvalue;
	std::string errmessage;
	std::string ID;
	void reset(){
		retvalue = 0;
		errmessage.clear();
		ID.clear();
	}
	template <class Archive>
	void serialize(Archive& archive){
		archive(retvalue, errmessage, ID);
	}
};

// sekey_key_change_status()
struct SEkey_Change_Key_Status_Params{ // T = 18
	int retvalue;
	std::string errmessage;
	std::string ID;
	se_key_status status;
	void reset(){
		retvalue = 0;
		errmessage.clear();
		ID.clear();
		status = se_key_status::statusmin;
	}
	template <class Archive>
	void serialize(Archive& archive){
		archive(retvalue, errmessage, ID, status);
	}
};

// sekey_key_change_name()
struct SEkey_Key_Change_Name_Params{ // T = 19
	int retvalue;
	std::string errmessage;
	std::string ID;
	std::string name;
	void reset(){
		retvalue = 0;
		errmessage.clear();
		ID.clear();
		name.clear();
	}
	template <class Archive>
	void serialize(Archive& archive){
		archive(retvalue, errmessage, ID, name);
	}
};

// sekey_key_get_info_all()
struct SEkey_Key_Get_Info_All_Params{ // T = 20
	int retvalue;
	std::string errmessage;
	std::vector<se_key> keys;
	void reset(){
		retvalue = 0;
		errmessage.clear();
	}
	template <class Archive>
	void serialize(Archive& archive){
		archive(retvalue, errmessage, keys);
	}
};

// sekey_key_get_info()
struct SEkey_Key_Get_Info_Params{ // T = 21
	int retvalue;
	std::string errmessage;
	std::string ID;
	se_key key;
	void reset(){
		retvalue = 0;
		errmessage.clear();
		ID.clear();
	}
	template <class Archive>
	void serialize(Archive& archive){
		archive(retvalue, errmessage, ID, key);
	}
};

// sekey_find_key_v1()
struct SEkey_Find_Key_v1_Params{ // T = 22
	int retvalue;
	std::string errmessage;
	std::string destination;
	std::string key;
	se_key_type keytype;
	void reset(){
		retvalue = 0;
		errmessage.clear();
		destination.clear();
		keytype = se_key_type::typemin;
	}
	template <class Archive>
	void serialize(Archive& archive){
		archive(retvalue, errmessage, destination, key, keytype);
	}
};

// sekey_find_key_v2()
struct SEkey_Find_Key_v2_Params{ // T = 23
	int retvalue;
	std::string errmessage;
	std::string destination;
	std::string key;
	se_key_type keytype;
	void reset(){
		retvalue = 0;
		errmessage.clear();
		destination.clear();
		keytype = se_key_type::typemin;
	}
	template <class Archive>
	void serialize(Archive& archive){
		archive(retvalue, errmessage, destination, key, keytype);
	}
};

// sekey_find_key_v3()
struct SEkey_Find_Key_v3_Params{ // T = 24
	int retvalue;
	std::string errmessage;
	std::vector<std::string> destination;
	std::string key;
	se_key_type keytype;
	void reset(){
		retvalue = 0;
		errmessage.clear();
		destination.clear();
		keytype = se_key_type::typemin;
	}
	template <class Archive>
	void serialize(Archive& archive){
		archive(retvalue, errmessage, destination, key, keytype);
	}
};

struct SEkey_GUI_info{ // T = 25
	int retvalue;
	std::string errmessage;
	std::string ID;
	std::string name;
	std::string serialnumber;
	std::string accessmode;
	void reset(){
		retvalue = 0;
		errmessage.clear();
		ID.clear();
		name.clear();
		serialnumber.clear();
		accessmode.clear();
	}
	template <class Archive>
	void serialize(Archive& archive){
		archive(retvalue, errmessage, ID, name, serialnumber, accessmode);
	}
};

struct SEcubeInitParams{ // T = 26
    int retvalue;
    std::string errmessage;
    std::array<uint8_t, L1Parameters::Size::PIN> adminPIN;
    std::array<uint8_t, L1Parameters::Size::PIN> userPIN;
    void reset(){
        retvalue = 0;
        errmessage.clear();
        adminPIN.fill(0);
        userPIN.fill(0);
    }
    template <class Archive>
    void serialize(Archive& archive){
        archive(retvalue, errmessage, adminPIN, userPIN);
    }
};

struct SEcubeInitParamsV2{ // T = 27
    int retvalue;
    std::string errmessage;
    std::string userID;
    std::array<uint8_t, L1Parameters::Size::PIN> adminPIN;
    std::array<uint8_t, L1Parameters::Size::PIN> userPIN;
    void reset(){
        retvalue = 0;
        errmessage.clear();
        userID.clear();
        adminPIN.fill(0);
        userPIN.fill(0);
    }
    template <class Archive>
    void serialize(Archive& archive){
        archive(retvalue, errmessage, userID, adminPIN, userPIN);
    }
};

#endif
