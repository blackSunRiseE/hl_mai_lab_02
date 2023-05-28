#include "product.h"
#include "database.h"
#include "../config/config.h"

#include <Poco/Data/MySQL/Connector.h>
#include <Poco/Data/MySQL/MySQLException.h>
#include <Poco/Data/SessionFactory.h>
#include <Poco/Data/RecordSet.h>
#include <Poco/JSON/Parser.h>
#include <Poco/Dynamic/Var.h>

#include <sstream>
#include <exception>
#include <ctime>
#include <stdlib.h>

#define TABLE_NAME "_product"

using namespace Poco::Data::Keywords;
using Poco::Data::Session;
using Poco::Data::Statement;

namespace database {
    std::vector<Product> Product::search(std::vector<std::pair<std::string, std::string>> params) {
        try {
            Poco::Data::Session session = database::Database::get().create_session();
            Poco::Data::Statement select(session);
            
            std::vector<Product> result;
            Product lot;
            User seller;

            std::vector<std::string> conditions;

            select << "select p.id, p.name, p.description, p.cost, p.seller_id, p.creation_date, "
                << "u.id, u.login, u.name, u.email, u.deleted from "  << TABLE_NAME
                << " p inner join _user u on u.id = p.seller_id where p.deleted = false",
                into(lot._id),
                into(lot._name),
                into(lot._description),
                into(lot._cost),
                into(lot._seller_id),
                into(lot._creation_date),
                into(lot._seller.id()),
                into(lot._seller.login()),
                into(lot._seller.name()),
                into(lot._seller.email()),
                into(lot._seller.deleted()),
                range(0, 1);

            
            for(std::pair<std::string, std::string> key_value: params) {
                std::string value = key_value.second;
                if (key_value.first == "id") {
                    select << "and p.id = ? ";
                    select.bind(atoi(value.c_str()));
                } else if (key_value.first == "cost_min") {
                    select << "and p.cost >= ? ";
                    select.bind(atoi(value.c_str()));
                } else if (key_value.first == "cost_max") {
                    select << "and p.cost <= ? ";
                    select.bind(atoi(value.c_str()));
                } else if (key_value.first == "seller_id") {
                    select << "and p.seller_id = ? ";
                    select.bind(atoi(value.c_str()));
                } else if (key_value.first == "name") {
                    std::replace(value.begin(), value.end(), ' ', '%');
                    select << "and p.lower(name) like lower(?) ";
                    select.bind("%" + value + "%");
                } else if (key_value.first == "creation_date_start") {
                    int tzd;
                    Poco::DateTime dateTime;
                    Poco::DateTimeParser::parse(key_value.first, dateTime, tzd);
                    select << "and creation_date >= ? ";
                    select.bind(dateTime);
                } else if (key_value.first == "creation_date_end") {
                    int tzd;
                    Poco::DateTime dateTime;
                    Poco::DateTimeParser::parse(key_value.first, dateTime, tzd);
                    select << "and creation_date <= ? ";
                    select.bind(dateTime);
                }
            }

            std::cout << "SQL search: " << select.toString() << std::endl;
        
            while (!select.done()){
                if (select.execute()){
                    lot.seller() = seller;
                    result.push_back(lot);
                }
            }

            return result;
        } catch (Poco::Data::MySQL::ConnectionException &e) {
            std::cout << "connection:" << e.what() << " :: " << e.message() << std::endl;
            throw;
        } catch (Poco::Data::MySQL::StatementException &e) {
            std::cout << "statement:" << e.what() << " :: " << e.message() << std::endl;
            throw;
        }
    }

    Product Product::get_by_id(long id) {
        if (id < 1) {
            return Product::empty();
        }
        try {
            Poco::Data::Session session = database::Database::get().create_session();
            Poco::Data::Statement select(session);
            Product lot;

            select << "select p.id, p.name, p.description, p.cost, p.seller_id, p.creation_date, p.deleted, "
                << "u.id, u.login, u.name, u.email, u.deleted from "  << TABLE_NAME
                << " p inner join _user u on u.id = p.seller_id where p.id = ?",
                into(lot._id),
                into(lot._name),
                into(lot._description),
                into(lot._cost),
                into(lot._seller_id),
                into(lot._creation_date),
                into(lot._deleted),
                into(lot._seller.id()),
                into(lot._seller.login()),
                into(lot._seller.name()),
                into(lot._seller.email()),
                into(lot._seller.deleted()),
                use(id),
                range(0, 1);
        
            select.execute();
            Poco::Data::RecordSet rs(select);
            if (rs.moveFirst())
                return lot;

            return Product::empty();
        } catch (Poco::Data::MySQL::ConnectionException &e) {
            std::cout << "connection:" << e.what() << " :: " << e.message() << std::endl;
            throw;
        } catch (Poco::Data::MySQL::StatementException &e) {
            std::cout << "statement:" << e.what() << " :: " << e.message() << std::endl;
            throw;
        }
    }

    void Product::update_entity() {
        Poco::Data::Session session = database::Database::get().create_session();
        session.begin();
        try {
            Poco::Data::Statement statement(session);

            statement << "update " << TABLE_NAME 
                << " set name = ?, description = ? , cost = ? , seller_id = ?, deleted = ? where id = ?",
                use(_name),
                use(_description),
                use(_cost),
                use(_seller_id),
                use(_deleted),
                use(_id);

            statement.execute();

            Poco::Data::Statement select(session);
            int updated = -1;
            select << "SELECT ROW_COUNT();",
                into(updated),
                range(0, 1);

            if (!select.done()){
                select.execute();
            }
            session.commit();

            std::cout << "Updated:" << updated << std::endl;
        } catch (Poco::Data::MySQL::ConnectionException &e) {
            session.rollback();
            std::cout << "connection:" << e.what() << " :: " << e.message() << std::endl;
            throw;
        } catch (Poco::Data::MySQL::StatementException &e) {
            session.rollback();
            std::cout << "statement:" << e.what() << " :: " << e.message() << std::endl;
            throw;
        }
    }

    void Product::insert_entity() {
        Poco::Data::Session session = database::Database::get().create_session();
        session.begin();
        try {
            Poco::Data::Statement statement(session);

            statement << "INSERT INTO " << TABLE_NAME 
                << " (name, description, cost, seller_id) VALUES(?, ?, ?, ?)",
                use(_name),
                use(_description),
                use(_cost),
                use(_seller_id);

            statement.execute();

            Poco::Data::Statement select(session);
            select << "SELECT LAST_INSERT_ID()",
                into(_id),
                range(0, 1);

            if (!select.done()){
                select.execute();
            }
            session.commit();
            
            std::cout << "New entity id:" << _id << std::endl;
        } catch (Poco::Data::MySQL::ConnectionException &e) {
            session.rollback();
            std::cout << "connection:" << e.what() << " :: " << e.message() << std::endl;
            throw;
        } catch (Poco::Data::MySQL::StatementException &e) {
            session.rollback();
            std::cout << "statement:" << e.what() << " :: " << e.message() << std::endl;
            throw;
        }
    }

    void Product::save_to_db() {
        if (_id > 0) {
            update_entity();
        } else {
            insert_entity();
        }
    }

    Poco::JSON::Object::Ptr Product::toJSON() const {
        Poco::JSON::Object::Ptr root = new Poco::JSON::Object();

        root->set("id", _id);
        root->set("name", _name);
        root->set("description", _description);
        root->set("cost", _cost);
        root->set("seller_id", _seller_id);
        root->set("creation_date", _creation_date);
        root->set("deleted", _deleted);

        if (_seller.get_id() > 0) {
            root->set("seller", _seller.toJSON());
        }

        return root;
    }

    template<typename T>
    T getOrDefault(Poco::JSON::Object::Ptr object, std::string field, T defaultValue) {
        if (object->has(field)) {
            return object->getValue<T>(field);
        }
        return defaultValue;
    }

    Product Product::fromJson(const std::string &str) {
        Product lot;
        Poco::JSON::Parser parser;
        Poco::Dynamic::Var result = parser.parse(str);
        Poco::JSON::Object::Ptr object = result.extract<Poco::JSON::Object::Ptr>();
        
        lot.id() = getOrDefault<long>(object, "id", 0);
        lot.name() = getOrDefault<std::string>(object, "name", "");
        lot.description() = getOrDefault<std::string>(object, "name", "");
        lot.cost() = getOrDefault<float>(object, "cost", 0);
        lot.seller_id() = getOrDefault<long>(object, "seller_id", 0);
        lot.deleted() = getOrDefault<bool>(object, "deleted", false);

        if (object->has("creation_date")) {
            lot.creattion_date() = object->getValue<Poco::DateTime>("creation_date");
        }

        if (object->has("seller")) {
            std::string seller_json = object->getValue<std::string>("seller");
            lot.seller() = User::fromJson(seller_json);
        }

        return lot;
    }

    long &Product::id() {
        return _id;
    }

    float &Product::cost() {
        return _cost;
    }
    
     std::string &Product::name() {
        return _name;
    }

    std::string &Product::description() {
        return _description;
    }

    long &Product::seller_id() {
        return _seller_id;
    }

    User &Product::seller() {
        return _seller;
    }

    long Product::get_id() const {
        return _id;
    }

    float Product::get_cost() const {
        return _cost;
    }
    
    const std::string &Product::get_name() const {
        return _name;
    }

    const std::string &Product::get_description() const {
        return _description;
    }

    long Product::get_seller_id() const {
        return _seller_id;
    }

    const User &Product::get_seller() const {
        return _seller;
    }

    const Poco::DateTime &Product::get_creation_date() const {
        return _creation_date;
    }

    Poco::DateTime &Product::creattion_date() {
        return _creation_date;
    }

    bool Product::is_deleted() const {
        return _deleted;
    }

    bool &Product::deleted() {
        return _deleted;
    }
}
