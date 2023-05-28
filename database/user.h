#ifndef USER_H
#define USER_H

#include <string>
#include <vector>
#include "Poco/JSON/Object.h"

namespace database {
    class User {
        private:
            long _id;
            std::string _login;
            std::string _name;
            std::string _email;
            std::string _password;
            bool _deleted;

            bool insert_entity();
            bool update_entity();
            bool is_exist();
        public:
            static User fromJson(const std::string &json);
            Poco::JSON::Object::Ptr toJSON() const;
            static User empty() {
                User user;
                user._id = -1;
                return user;
            }

            long get_id() const;
            const std::string &get_name() const;
            const std::string &get_email() const;
            const std::string &get_login() const;
            const std::string &get_password() const;
            bool is_deleted() const;

            long& id();
            std::string &name();
            std::string &email();
            std::string &login();
            std::string &password();
            bool &deleted();

            static long auth(std::string &login, std::string &password);
            static User get_by_id(long id);
            static std::vector<User> search(User likeUser);
            static bool have_role(long user_id, std::string role_name);
            static void add_role(long user_id, std::string role_name);
            static void remove_role(long user_id, std::string role_name);
            
            bool save_to_db();

            static void create_test_users();
    };
}

#endif