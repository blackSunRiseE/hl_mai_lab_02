#ifndef SERVICE_EXCEPTIONS_H
#define SERVICE_EXCEPTIONS_H

#include <exception>
#include <string>

class validation_exception: public std::exception {
    public:
        validation_exception(const std::string& message): _message(message){};
        const char* what() const noexcept override {
            return _message.c_str();
        }
    private:
        std::string _message;
};

class access_denied_exception: public std::exception {
    public:
        access_denied_exception(const std::string& message): _message(message){};
        const char* what() const noexcept override {
            return _message.c_str();
        }
    private:
        std::string _message;
};

class not_found_exception: public std::exception {
    public:
        not_found_exception(const std::string& message): _message(message){};
        const char* what() const noexcept override {
            return _message.c_str();
        }
    private:
        std::string _message;
};

#endif