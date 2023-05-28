#ifndef REQUEST_UTILS_H
#define REQUEST_UTILS_H

#include <string>
#include <iostream>
#include "Poco/JSON/Object.h"
#include "Poco/Net/HTTPServerResponse.h"
#include <Poco/JSON/Parser.h>
#include "ctime"

static bool hasSubstr(const std::string &str, const std::string &substr) {
    if (str.size() < substr.size())
        return false;
    for (size_t i = 0; i <= str.size() - substr.size(); ++i) {
        bool ok{true};
        for (size_t j = 0; ok && (j < substr.size()); ++j)
            ok = (str[i + j] == substr[j]);
        if (ok)
            return true;
    }
    return false;
}

std::string extractBody(std::istream &reqstream, int len) {
    char* buffer = new char[len];
    reqstream.read(buffer, len);
    return buffer;
} 

Poco::JSON::Object::Ptr parseJson(std::string response_body) {
    Poco::JSON::Parser parser;
    Poco::Dynamic::Var result = parser.parse(response_body);
    return result.extract<Poco::JSON::Object::Ptr>();
}

bool validateToken(std::string scheme, std::string token, long &id, std::string &login) {
    if (scheme.length() == 0 || token.length() == 0) {
        return false;
    }

    std::string host = "localhost";
    std::string auth_port = "8081";
    if (std::getenv("AUTH_SERVICE_HOST") != nullptr)
        host = std::getenv("AUTH_SERVICE_HOST");
    if (std::getenv("AUTH_SERVICE_PORT") != nullptr) {
        auth_port = std::getenv("AUTH_SERVICE_PORT");
    }   
    std::string url = "http://" + host + ":" + auth_port + "/auth/validate";

    try {
        Poco::URI uri(url);
        Poco::Net::HTTPClientSession session(uri.getHost(), uri.getPort());
        Poco::Net::HTTPRequest request(Poco::Net::HTTPRequest::HTTP_GET, uri.toString());
        request.setVersion(Poco::Net::HTTPMessage::HTTP_1_1);
        request.setKeepAlive(true);
        request.setCredentials(scheme, token);

        session.sendRequest(request);

        Poco::Net::HTTPResponse response;
        std::istream &rs = session.receiveResponse(response);
        std::string response_body;

        while (rs) {
            char c{};
            rs.read(&c, 1);
            if (rs)
                response_body += c;
        }

        if (response.getStatus() != Poco::Net::HTTPResponse::HTTPStatus::HTTP_ACCEPTED) {
            std::cout << "Failed to validate token [" << response.getStatus() << "] " << response_body << std::endl;
            return false;
        }

        Poco::JSON::Object::Ptr object = parseJson(response_body);
        if (object->has("login") && object->has("id")) {
            login = object->getValue<std::string>("login");
            id = object->getValue<long>("id");
            return true;
        }
        std::cout << "Not enough fields in auth response" << std::endl;
    } catch (Poco::Exception &ex) {
        std::cout << "Failed to validate token " << ex.what() << " :: " << ex.message() << std::endl;
    }
    return false;
}

#endif