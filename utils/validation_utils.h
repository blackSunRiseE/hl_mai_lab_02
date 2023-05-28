#include <string>
#include "Poco/Net/HTMLForm.h"
#include "../database/user.h"

bool check_name(const std::string &name, std::string &reason) {
    if (name.length() < 3) {
        reason = "Name must be at least 3 signs;";
        return false;
    }

    return true;
};

bool check_email(const std::string &email, std::string &reason) {
    bool res = true;
    if (email.find('@') == std::string::npos) {
        reason = "Email must contain @;";
        res = false;
    }

    if (email.find(' ') != std::string::npos) {
        reason = "EMail can't contain spaces;";
        res = false;
    }

    if (email.find('\t') != std::string::npos) {
        reason = "EMail can't contain spaces;";
        res = false;
    }

    return res;
};


bool validate_user(database::User &user, std::string &message) {
    bool result = true;
    std::string reason;

    if (!check_email(user.get_email(), reason)) {
        message += reason;
        result = false;
    }

    if (!check_name(user.get_name(), reason)) {
        message += reason;
        result = false;
    }

    if (user.get_login().length() < 3) {
        message += "login must be at least 3 signs;";
        result = false;
    }

    if (user.get_password().length() < 3) {
        message += "password must be at least 3 signs;";
        result = false;
    }

    return result;
};