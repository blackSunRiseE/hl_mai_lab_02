#include "Poco/JSON/Object.h"
#include "Poco/JSON/Array.h"

#include "../database/user.h"
#include "../utils/request_utils.h"
#include "../utils/validation_utils.h"
#include "../service_excpetion/service_exceptions.h"

void edit_user(long user_id, std::string body) {
    Poco::JSON::Object::Ptr obj = parseJson(body);
    if (!obj->has("id")) {
        throw validation_exception("Can't edit without user id!");
    }

    database::User user = database::User::get_by_id(obj->getValue<long>("id"));
    if (user.get_id() <= 0) {
        throw not_found_exception("Can't find user");
    } else if (user.get_id() != user_id && !database::User::have_role(user_id, "admin")) {
        throw not_found_exception(std::to_string(user_id) + " user can't edit " + user.get_login());
    } 

    std::string validation_message;
    if (obj->has("name")) {
        std::string value = obj->getValue<std::string>("name");
        check_name(value, validation_message);
        user.name() = obj->getValue<std::string>("name");
    }
    if (obj->has("email")) {
        std::string value = obj->getValue<std::string>("email");
        check_email(value, validation_message);
        user.email() = obj->getValue<std::string>("email");
    }
    if (validation_message.length() > 0) {
        throw validation_exception(validation_message);
    }

    user.save_to_db();
}

std::vector<database::User> search(std::vector<std::pair<std::string, std::string>> params) {
    database::User likeUser;
    for(std::pair<std::string, std::string> key_value: params) {
        std::string key = key_value.first;
        std::string value = key_value.second;
        if (key == "login") {
            likeUser.login() = value;
        } else if (key == "name") {
            likeUser.name() = value;
        } else if (key == "email") {
            likeUser.email() = value;
        } else {
            std::cout << "Param " << key << " :: " << value << " ignored" << std::endl;
        }
    }
    
    return database::User::search(likeUser);
}

void delete_user(long user_id, std::vector<std::pair<std::string, std::string>> params) {
    long delete_user_id = -1;

    for (auto key_value: params) {
        if (key_value.first == "id") {
            if (key_value.second.length() == 0) {
                throw validation_exception("Id can't be blank");
            }
            delete_user_id = atoi(key_value.second.c_str());
        }
    }

    if (delete_user_id < 0) {
        throw validation_exception("Not valid user id");
    }


    database::User user = database::User::get_by_id(delete_user_id);
    if (user.get_id() <= 0) {
        throw not_found_exception("Can't find user");
    }

    if (user_id == user.get_id() || database::User::have_role(user_id, "admin")) {
        user.deleted() = true;
        user.save_to_db();

        std::cout << "User " << user_id << " deleted user " << delete_user_id << std::endl;
    } else {
        throw access_denied_exception(std::to_string(user_id) + " user can't delete user " + std::to_string(delete_user_id));
    }
}