#ifndef AUTH_SERVICE_H
#define AUTH_SERVICE_H

#include "Poco/JSON/Object.h"
#include "Poco/JSON/Array.h"

#include "../database/user.h"
#include "../utils/request_utils.h"
#include "../utils/validation_utils.h"
#include "../utils/token_utils.h"
#include "../service_excpetion/service_exceptions.h"


std::string auth_user(std::string info) {
    std::string login;
    std::string password;

    if (!decode_info(info, login, password)) {
        throw access_denied_exception("Can't decode info");
    }

    long id = database::User::auth(login, password);
    if (id <= 0) {
        throw access_denied_exception("Wrong login/password");
    }
    std::cout << "Found user with id " << id << std::endl;
    return generate_token(id, login);
}

database::User validate(std::string token) {
    std::string login;
    long id;
    if (extract_payload(token, id, login)) {
        database::User user = database::User::get_by_id(id);
        if (user.get_id() <= 0) {
            throw access_denied_exception("User not found");
        }
        if (user.is_deleted()) {
            throw access_denied_exception("User deleted");
        }
        return user;
    } else {
        throw access_denied_exception("Failed to deserialize token");
    }
}

long create_user(std::string body) {
    if (body.length() == 0) {
        throw validation_exception("Body is missing");
    }
    std::string validation_message;
    database::User user = database::User::fromJson(body);

    if (validate_user(user, validation_message)) {
        std::cout << "Creating new user: " << body << std::endl;
        user.save_to_db();
        return user.get_id();
    } else {
        throw validation_exception(validation_message);
    }
}

#endif