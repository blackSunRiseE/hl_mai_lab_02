#ifndef PRPDUCT_SERVICE_H
#define PRPDUCT_SERVICE_H

#include "Poco/JSON/Object.h"
#include "Poco/JSON/Array.h"

#include "../database/user.h"
#include "../database/product.h"
#include "../utils/request_utils.h"
#include "../utils/validation_utils.h"
#include "../service_excpetion/service_exceptions.h"

bool validate_product(database::Product &product, std::string &message) {
    bool result = true;

    if (product.get_name().length() < 3) {
        result = false;
        message += "name should be at least 3 signs;";
    }

    if (product.get_description().length() < 3) {
        result = false;
        message += "description should be at least 3 digits;";
    }

    if (product.get_cost() < 0) {
        result = false;
        message += "cast can't be negetive;";
    }

    return result;
}

static long create_product(long &user_id, std::string user_login, std::string &body) {
    if (body.length() == 0) {
        throw validation_exception("Body is missing!");
    }
    std::cout << "Creating new product by " << user_login << " :: " + user_id << std::endl;
    std::cout << body << std::endl;

    database::Product product = database::Product::fromJson(body);
    std::string validation_result;
    if (!validate_product(product, validation_result)) {
        throw validation_exception(validation_result);
    }
    database::User seller = database::User::get_by_id(user_id);

    if (seller.get_id() <= 0) {
        throw validation_exception("Can't find user by id " + std::to_string(user_id));
    }
    product.seller_id() = seller.get_id();
    product.save_to_db();

    return product.get_id();
}

void edit_product(long &user_id, std::string &body) {
    if (body.length() == 0) {
        throw validation_exception("Body is missing!");
    }
    Poco::JSON::Object::Ptr obj = parseJson(body);
    if (!obj->has("id")) {
        throw validation_exception("Can't edit without product id!");
    }

    database::Product product = database::Product::get_by_id(obj->getValue<long>("id"));
    if (product.get_id() <= 0) {
        throw not_found_exception("Can't find product with id " + std::to_string(product.get_id()));
    }
    if (product.get_seller_id() == user_id || database::User::have_role(product.get_seller_id(), "admin")) {
        std::string validation_message;
        if (obj->has("name")) {
            std::string value = obj->getValue<std::string>("name");
            check_name(value, validation_message);
            product.name() = obj->getValue<std::string>("name");
        }
        if (obj->has("description")) {
            std::string value = obj->getValue<std::string>("description");
            check_email(value, validation_message);
            product.description() = obj->getValue<std::string>("description");
        }
        if (obj->has("cost")) {
            if (obj->getValue<float>("cost") > 0) {
                product.cost() = obj->getValue<float>("cost");
            } else {
                validation_message += "cost can't be negative;";
            }
        }
        if (validation_message.length() > 0) {
            throw validation_exception(validation_message);
        }

        product.save_to_db();
    } else {
        throw access_denied_exception("Current user can't edit this product");
    }
}

void delete_product(long &user_id, std::vector<std::pair<std::string, std::string>> params) {
    std::string product_id;
    for (auto key_value: params) {
        if (key_value.first == "id") {
            product_id = key_value.second;
        }
    }
    if (product_id.length() == 0) {
        throw validation_exception("Id param is missing!");
    }
    database::Product product = database::Product::get_by_id(atoi(product_id.c_str()));
    if (product.get_id() <= 0) {
        throw not_found_exception("Can't find product by id " + product_id);
    }

    if (product.get_seller_id() == user_id || database::User::have_role(product.get_seller_id(), "admin")) {
        product.deleted() = true;
        product.save_to_db();
        std::cout << "User " << user_id << " deleted product " << product_id << std::endl;
    } else {
        throw access_denied_exception("Current user can't deleted this product");
    }

}

#endif