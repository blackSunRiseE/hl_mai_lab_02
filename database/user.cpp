#include "user.h"
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

#define TABLE_NAME "_user"

using namespace Poco::Data::Keywords;
using Poco::Data::Session;
using Poco::Data::Statement;

namespace database {
    long User::auth(std::string &login, std::string &password) {
        try {
            std::cout << "Trying to auth " << login << "::" << password << std::endl;
            Poco::Data::Session session = database::Database::get().create_session();
            
            long id;
            std::vector<std::string> shards = database::Database::get_all_hints();
            for (const std::string &shard: shards) {
                Poco::Data::Statement select(session);
                select << "SELECT id FROM " << TABLE_NAME << " where login=? and password=? and deleted = false"
                    << shard,
                    into(id),
                    use(login),
                    use(password),
                    range(0, 1);

                std::cout << "[DEBUG SQL] " << select.toString() << std::endl; 

                select.execute();
                Poco::Data::RecordSet rs(select);
                if (rs.moveFirst()) {
                    std::cout << id <<std::endl;
                    return id;
                }
            }
            
        } catch (Poco::Data::DataException &e) {
            std::cout << "Exception: " << e.what() << " :: " << e.message() << std::endl;
            return -1;
        }
        return 0;
    }

    bool User::have_role(long user_id, std::string role_name) {
        try {
            Poco::Data::Session session = database::Database::get().create_session();
            Poco::Data::Statement select(session);
            
            bool result = false;

            select << "select true from _users_roles "
                << "where user_id = ? and role_name = ?"
                << database::Database::sharding_hint(user_id),
                into(result),
                use(user_id),
                use(role_name),
                range(0, 1);
        
            std::cout << "[DEBUG SQL] " << select.toString() << std::endl; 
                
            select.execute();
            Poco::Data::RecordSet rs(select);
            if (rs.moveFirst())
                return result;

            return false;
        } catch (Poco::Data::MySQL::ConnectionException &e) {
            std::cout << "connection:" << e.what() << " :: " << e.message() << std::endl;
            throw;
        } catch (Poco::Data::MySQL::StatementException &e) {
            std::cout << "statement:" << e.what() << " :: " << e.message() << std::endl;
            throw;
        }
    }
    void User::add_role(long user_id, std::string role) {
        if (have_role(user_id, role)) {
            std::cout << "User " << user_id << " already have role " << role << std::endl;
            return;
        }

        Poco::Data::Session session = database::Database::get().create_session();
        session.begin();
        try {
            Poco::Data::Statement statement(session);

            statement << "INSERT INTO _users_roles (role_name, user_id) VALUES(?, ?)"
                << database::Database::sharding_hint(user_id),
                use(role),
                use(user_id);

            std::cout << "[DEBUG SQL] " << statement.toString() << std::endl; 

            statement.execute();
            session.commit();
            std::cout << "Added role [" << role << "] to user " << user_id << std::endl;
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
    void User::remove_role(long user_id, std::string role) {
        Poco::Data::Session session = database::Database::get().create_session();
        session.begin();
        try {
            Poco::Data::Statement statement(session);

            statement << "delete from _users_roles where role_name = ? and user_id = ? "
                << database::Database::sharding_hint(user_id),
                use(role),
                use(user_id);

            std::cout << "[DEBUG SQL] " << statement.toString() << std::endl; 

            statement.execute();
            session.commit();
            std::cout << "Removed role [" << role << "] to user " << user_id << std::endl;
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

    User User::get_by_id(long id) {
        try {
            Poco::Data::Session session = database::Database::get().create_session();
            Poco::Data::Statement select(session);
            User user;

            select << "select id, login, email, name, deleted from "  << TABLE_NAME << " where id = ?"
                << database::Database::sharding_hint(id),
                into(user._id),
                into(user._login),
                into(user._email),
                into(user._name),
                into(user._deleted),
                use(id),
                range(0, 1);

            std::cout << "[DEBUG SQL] " << select.toString() << std::endl; 
        
            select.execute();
            Poco::Data::RecordSet rs(select);
            if (rs.moveFirst())
                return user;

            return User::empty();
        } catch (Poco::Data::MySQL::ConnectionException &e) {
            std::cout << "connection:" << e.what() << std::endl;
            throw;
        } catch (Poco::Data::MySQL::StatementException &e) {
            std::cout << "statement:" << e.what() << std::endl;
            throw;
        }
    }

    bool User::save_to_db() {
        if (_id > 0) {
            return update_entity();
        } else {
            return insert_entity();
        }
    }   

    bool User::insert_entity() {
        if (is_exist()) {
            std::cout << "User already exist" << std::endl;
            return false;
        }

        _id = database::Database::generate_new_id();

        Poco::Data::Session session = database::Database::get().create_session();
        session.begin();
        try {
            Poco::Data::Statement statement(session);

            // todo check if email and login exists
            statement << "INSERT INTO " << TABLE_NAME << " (id, name, email, login, password) VALUES(?, ?, ?, ?, ?)"
                << database::Database::sharding_hint(_id),
                use(_id),
                use(_name),
                use(_email),
                use(_login),
                use(_password);

            std::cout << "[DEBUG SQL] " << statement.toString() << std::endl; 

            statement.execute();
            session.commit();
            std::cout << "New user entity created, id = " << _id << std::endl;
            return true;
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

    bool User::update_entity() {
        Poco::Data::Session session = database::Database::get().create_session();
        session.begin();
        try {
            Poco::Data::Statement statement(session);

            statement << "update " << TABLE_NAME << " set name = ?, email = ? , login = ? , password = ?, deleted = ? where id = ?"
                << database::Database::sharding_hint(_id),
                use(_name),
                use(_email),
                use(_login),
                use(_password),
                use(_deleted),
                use(_id);

            std::cout << "[DEBUG SQL] " << statement.toString() << std::endl; 

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
            return true;
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

    std::vector<User> User::search(User likeUser) {
        try {
            Poco::Data::Session session = database::Database::get().create_session();
            
            std::vector<User> result;

            std::string sql = "select id, login, email, name, deleted from ";
            sql += TABLE_NAME;
            sql += " where deleted = false";

            if (likeUser.get_name().length() > 0) {
                std::replace(likeUser.name().begin(), likeUser.name().end(), ' ', '%');
                sql += " and lower(name) like '%" + likeUser.get_name() + "%'";
            }

            if (likeUser.get_login().length() > 0) {
                std::replace(likeUser.login().begin(), likeUser.login().end(), ' ', '%');
                sql += " and lower(login) like '%" + likeUser.get_login() + "%'";
            }

            if (likeUser.get_email().length() > 0) {
                sql += " and lower(email) like '%" + likeUser.get_email() + "%'";
            }

            std::vector<std::string> shards = database::Database::get_all_hints();
            for (const std::string &shard: shards) {
                User user;
                Poco::Data::Statement select(session);
                select << sql << shard,
                    into(user._id),
                    into(user._login),
                    into(user._email),
                    into(user._name),
                    into(user._deleted),
                    range(0, 1);

                std::cout << "[DEBUG SQL] " << select.toString() << std::endl; 

                while (!select.done()){
                    if (select.execute())
                        result.push_back(user);
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

    bool User::is_exist() {
        try {
            std::cout << "Checking existence of user " << _login << " :: " << _email << std::endl;
            Poco::Data::Session session = database::Database::get().create_session();
            bool result = false;

            std::vector<std::string> shards = database::Database::get_all_hints();
            for (const std::string &shard: shards) {
                Poco::Data::Statement select(session);
                select << "select true from "  << TABLE_NAME 
                    << " where login = ? or email = ?" << shard,
                    into(result),
                    use(_login),
                    use(_email),
                    range(0, 1);

                std::cout << "[DEBUG SQL] " << select.toString() << std::endl; 
            
                select.execute();
                Poco::Data::RecordSet rs(select);
                if (rs.moveFirst())
                    return result;
            }
            return result;
        } catch (Poco::Data::MySQL::ConnectionException &e) {
            std::cout << "connection:" << e.what() << std::endl;
            throw;
        } catch (Poco::Data::MySQL::StatementException &e) {
            std::cout << "statement:" << e.what() << std::endl;
            throw;
        }
    }

    void User::create_test_users() {
        try {
            std::cout << "Creating test users" << std::endl;

            // todo fix creation
            // std::vector<std::string> names = {"Autotest user", "Some Test User", "Another user", "Just test"};
            // int i = 0;
            // for (const std::string &name: names) {
            //     User user;
            //     user.name() = name;
            //     user.login() = "autotest_user" + std::to_string(i);
            //     user.email() = std::to_string(i) + "email@cool2.com";
            //     user.password() = "123";
            //     user.insert_entity();
            //     i++;
            // }
             
            User admin;
            admin.name() = "Autotest admin";
            admin.login() = "autotest_admin";
            admin.email() = "email@cool.com";
            admin.password() = "123";
            if (admin.insert_entity()) {
                add_role(admin.get_id(), "admin");
            }

            std::cout << "Test users created" << std::endl;
        } catch (std::exception &e) {
            std::cout << "Failed to create test users: " << e.what() << std::endl;
        }
    }

    Poco::JSON::Object::Ptr User::toJSON() const {
        Poco::JSON::Object::Ptr root = new Poco::JSON::Object();

        root->set("id", _id);
        root->set("name", _name);
        root->set("email", _email);
        root->set("login", _login);
        root->set("deleted", _deleted);

        return root;
    }

    template<typename T>
    T getOrDefault(Poco::JSON::Object::Ptr object, std::string field, T defaultValue) {
        if (object->has(field)) {
            return object->getValue<T>(field);
        }
        return defaultValue;
    }

    User User::fromJson(const std::string &str) {
        User user;
        Poco::JSON::Parser parser;
        Poco::Dynamic::Var result = parser.parse(str);
        Poco::JSON::Object::Ptr object = result.extract<Poco::JSON::Object::Ptr>();
        
        user.id() = getOrDefault<long>(object, "id", 0);
        user.name() = getOrDefault<std::string>(object, "name", "");
        user.email() = getOrDefault<std::string>(object, "email", "");
        user.login() = getOrDefault<std::string>(object, "login", "");
        user.password() = getOrDefault<std::string>(object, "password", "");
        user.deleted() = getOrDefault<bool>(object, "deleted", false);

        return user;
    }

    const std::string &User::get_login() const {
        return _login;
    }

    const std::string &User::get_password() const {
        return _password;
    }

    std::string &User::login() {
        return _login;
    }

    std::string &User::password() {
        return _password;
    }

    long User::get_id() const {
        return _id;
    }

    const std::string &User::get_name() const {
        return _name;
    }

    const std::string &User::get_email() const {
        return _email;
    }

    long &User::id() {
        return _id;
    }

    std::string &User::name() {
        return _name;
    }

    std::string &User::email() {
        return _email;
    }

    bool User::is_deleted() const {
        return _deleted;
    }

    bool &User::deleted() {
        return _deleted;
    }
}