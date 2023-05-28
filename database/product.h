#ifndef SERVICE_LOT_H
#define SERVICE_LOT_H

#include <string>
#include <vector>
#include "./user.h"
#include "Poco/JSON/Object.h"
#include <ctime>

namespace database {
    class Product {
        private:
            long _id;
            std::string _name;
            std::string _description;
            float _cost;
            long _seller_id;
            bool _deleted;
            User _seller = User::empty();
            Poco::DateTime _creation_date;

            void insert_entity();
            void update_entity();
        public:
            static Product empty() {
                Product lot;
                lot._id = -1;
                return lot;
            }
            static Product fromJson(const std::string &json);
            Poco::JSON::Object::Ptr toJSON() const;

            long get_id() const;
            const std::string &get_name() const;
            const std::string &get_description() const;
            float get_cost() const;
            const Poco::DateTime &get_creation_date() const;
            bool is_deleted() const;
            long get_seller_id() const;
            const User &get_seller() const;

            long &id();
            std::string &name();
            std::string &description();
            float &cost();
            Poco::DateTime &creattion_date();
            bool &deleted();
            long &seller_id();
            User &seller();

            static std::vector<Product> search(std::vector<std::pair<std::string, std::string>> params);
            static Product get_by_id(long id);
            void save_to_db();
    };
}

#endif