#include "database.h"
#include "../config/config.h"

#include "Poco/Data/Session.h"
#include "Poco/Data/SQLite/Connector.h"

using namespace Poco::Data::Keywords;
using Poco::Data::Session;
using Poco::Data::Statement;


namespace database{
    Database::Database(){
        _connection_string+="host=";
        _connection_string+=Config::get().get_host();
        _connection_string+=";user=";
        _connection_string+=Config::get().get_login();
        _connection_string+=";db=";
        _connection_string+=Config::get().get_database();
        _connection_string+=";port=";
        _connection_string+=Config::get().get_port();
        _connection_string+=";password=";
        _connection_string+=Config::get().get_password();

        std::cout << "Connection string: " << _connection_string << std::endl; 
        Poco::Data::MySQL::Connector::registerConnector();
        std::cout << "Connector registered" << std::endl; 
        _pool = std::make_unique<Poco::Data::SessionPool>(Poco::Data::MySQL::Connector::KEY, _connection_string);
        std::cout << "Session pool created" << std::endl; 
    }

    Database& Database::get(){
        static Database _instance;
        return _instance;
    }

    Poco::Data::Session Database::create_session(){
        return Poco::Data::Session(_pool->get());
    }

    size_t Database::get_max_shard() {
        return atoi(std::getenv("MAX_SHARD"));
    }

    size_t Database::get_sequence_shard() {
        return atoi(std::getenv("SEQUENCE_SHARD"));
    }

    std::vector<std::string> Database::get_all_hints(){
        std::vector<std::string> result;
        for(size_t i=0;i<=get_max_shard();++i){
            std::string shard_name = "-- sharding:";
            shard_name += std::to_string(i);
            result.push_back(shard_name);
        }
        return result;
    }

    std::string Database::squence_sharding_hint(){
        // size_t shard_number = std::hash<std::string>{}(login)%get_max_shard();
        std::string result = "-- sharding:";
        result += std::to_string(get_sequence_shard());
        return result;
    }

    std::string Database::sharding_hint(long id){
        // size_t shard_number = std::hash<std::string>{}(login)%get_max_shard();
        size_t shard_number = id % (get_max_shard() + 1);
        std::string result = "-- sharding:";
        result += std::to_string(shard_number);
        return result;
    }


    long Database::generate_new_id() {
        Poco::Data::Session session = get().create_session();
        session.begin();
        try {
            long id = -1;
            Poco::Data::Statement select(session);
            select << "SELECT next value for main_id_sequence " << squence_sharding_hint(),
                into(id),
                range(0, 1);

            std::cout << "[DEBUG SQL] " << select.toString() << std::endl; 
            if (!select.done()){
                select.execute();
            }
            session.commit();
            
            std::cout << "Generated id:" << id << std::endl;
            return id;
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
}