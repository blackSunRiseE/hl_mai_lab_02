#ifndef AUTH_REST_H
#define AUTH_REST_H


#include "Poco/Net/HTTPServer.h"
#include "Poco/Net/HTTPRequestHandler.h"
#include "Poco/Net/HTTPRequestHandlerFactory.h"
#include "Poco/Net/HTTPServerParams.h"
#include "Poco/Net/HTTPServerRequest.h"
#include "Poco/Net/HTTPServerResponse.h"
#include "Poco/Net/HTTPServerParams.h"
#include "Poco/Net/ServerSocket.h"
#include "Poco/Timestamp.h"
#include "Poco/DateTimeFormatter.h"
#include "Poco/DateTimeFormat.h"
#include "Poco/Exception.h"
#include "Poco/ThreadPool.h"
#include "Poco/Util/ServerApplication.h"
#include "Poco/Util/Option.h"
#include "Poco/Util/OptionSet.h"
#include "Poco/Util/HelpFormatter.h"
#include "Poco/JSON/Object.h"
#include "Poco/Net/HTTPServer.h"
#include "Poco/Net/HTTPRequestHandler.h"
#include "Poco/Net/HTTPRequestHandlerFactory.h"
#include "Poco/Net/HTTPServerParams.h"
#include "Poco/Net/HTTPServerRequest.h"
#include "Poco/Net/HTTPServerResponse.h"
#include "Poco/Net/HTTPServerParams.h"
#include "Poco/Net/HTMLForm.h"
#include "Poco/Net/ServerSocket.h"
#include "Poco/Timestamp.h"
#include "Poco/DateTimeFormatter.h"
#include "Poco/DateTimeFormat.h"
#include "Poco/Exception.h"
#include "Poco/ThreadPool.h"
#include "Poco/Util/ServerApplication.h"
#include "Poco/Util/Option.h"
#include "Poco/Util/OptionSet.h"
#include "Poco/Util/HelpFormatter.h"
#include "Poco/StreamCopier.h"
#include "Poco/URI.h"
#include "Poco/Exception.h"
#include "Poco/SharedPtr.h"
#include "Poco/Net/SSLManager.h"
#include "Poco/Net/KeyConsoleHandler.h"
#include "Poco/Net/ConsoleCertificateHandler.h"
#include "Poco/Net/AcceptCertificateHandler.h"
#include "Poco/Net/HTTPSClientSession.h"
#include "Poco/Net/HTTPRequest.h"
#include "Poco/Net/HTTPResponse.h"
#include <Poco/JSON/Parser.h>
#include <Poco/Dynamic/Var.h>

#include <Poco/URIStreamFactory.h>
#include <Poco/URIStreamOpener.h>
#include <Poco/Net/HTTPSStreamFactory.h>
#include <Poco/Net/HTTPStreamFactory.h>
#include <Poco/Base64Encoder.h>
#include <Poco/Base64Decoder.h>

using Poco::DateTimeFormat;
using Poco::DateTimeFormatter;
using Poco::ThreadPool;
using Poco::Timestamp;
using Poco::Net::HTTPRequestHandler;
using Poco::Net::HTTPRequestHandlerFactory;
using Poco::Net::HTTPServer;
using Poco::Net::HTTPServerParams;
using Poco::Net::HTTPServerRequest;
using Poco::Net::HTTPServerResponse;
using Poco::Net::ServerSocket;
using Poco::Util::Application;
using Poco::Util::HelpFormatter;
using Poco::Util::Option;
using Poco::Util::OptionCallback;
using Poco::Util::OptionSet;
using Poco::Util::ServerApplication;
using Poco::DateTimeFormat;
using Poco::DateTimeFormatter;
using Poco::ThreadPool;
using Poco::Timestamp;
using Poco::Net::HTMLForm;
using Poco::Net::NameValueCollection;
using Poco::Util::HelpFormatter;
using Poco::Util::Option;
using Poco::Util::OptionCallback;
using Poco::Util::OptionSet;
using Poco::Util::ServerApplication;

#include <iostream>
#include <string>
#include <fstream>
#include "../database/user.h"
#include "../utils/request_utils.h"
#include "./auth_service.h"
#include "../service_excpetion/service_exceptions.h"


class AuthRequestHandler : public HTTPRequestHandler {
    public:
        AuthRequestHandler(const std::string &format): _format(format){};

        void handleRequest(HTTPServerRequest &request, HTTPServerResponse &response) {
            response.add("Access-Control-Allow-Origin", "*");
            response.add("Access-Control-Allow-Methods", "GET, POST, DELETE, PUT, OPTIONS");
            response.add("Access-Control-Allow-Headers", "Content-Type, api_key, Authorization");
            if (request.getMethod() == Poco::Net::HTTPRequest::HTTP_OPTIONS) {
                response.setContentType("application/json");
                response.setKeepAlive(true);
                response.setStatusAndReason(Poco::Net::HTTPResponse::HTTP_OK);
                response.send();
                return;
            }

            try {
                if (request.getMethod() == Poco::Net::HTTPRequest::HTTP_GET) {
                    if (hasSubstr(request.getURI(), "/sign/in")) {
                        std::string scheme;
                        std::string info;
                        request.getCredentials(scheme, info);
                        std::cout << "scheme " << scheme << " :: info " << info << std::endl;

                        if (scheme == "Basic") {
                            std::string token = auth_user(info);
                            response.setStatusAndReason(Poco::Net::HTTPResponse::HTTP_OK);
                            response.setChunkedTransferEncoding(true);
                            response.setContentType("application/json");
                            Poco::JSON::Object::Ptr root = new Poco::JSON::Object();
                            root->set("token", token);
                            std::ostream &ostr = response.send();
                            Poco::JSON::Stringifier::stringify(root, ostr);
                        } else {
                            throw validation_exception("Only basic auth accepted");
                        }
                    } else if (hasSubstr(request.getURI(), "/validate")) {
                        std::string scheme;
                        std::string token;

                        request.getCredentials(scheme, token);

                        std::cout << "Scheme/token: " << scheme << "/" << token << std::endl;
                        if (scheme == "Bearer") {
                            database::User user = validate(token);
                            response.setStatus(Poco::Net::HTTPResponse::HTTPStatus::HTTP_ACCEPTED);
                            response.setChunkedTransferEncoding(true);
                            response.setContentType("application/json");
                            Poco::JSON::Object::Ptr root = new Poco::JSON::Object();
                            root->set("login", user.get_login());
                            root->set("id", user.get_id());
                            std::ostream &ostr = response.send();
                            Poco::JSON::Stringifier::stringify(root, ostr);
                        } else {
                            throw access_denied_exception("No bearer token found");
                        }
                    }
                }
                if (request.getMethod() == Poco::Net::HTTPRequest::HTTP_POST) {
                    if (hasSubstr(request.getURI(), "/sign/up")) {
                        std::string body = extractBody(request.stream(), request.getContentLength());
                        long id = create_user(body);
                        response.setStatus(Poco::Net::HTTPResponse::HTTPStatus::HTTP_CREATED);
                        response.setChunkedTransferEncoding(true);
                        response.setContentType("application/json");
                        Poco::JSON::Object::Ptr root = new Poco::JSON::Object();
                        root->set("id", id);
                        std::ostream &ostr = response.send();
                        Poco::JSON::Stringifier::stringify(root, ostr);
                    }
                }
            } catch (validation_exception &ex) {
                std::cout << "Validation exception: " << ex.what() << std::endl;
                response.setStatusAndReason(Poco::Net::HTTPResponse::HTTPStatus::HTTP_BAD_REQUEST);
                response.setChunkedTransferEncoding(true);
                response.setContentType("application/json");
                Poco::JSON::Object::Ptr root = new Poco::JSON::Object();
                root->set("type", "/errors/validation");
                root->set("detail", ex.what());
                root->set("status", Poco::Net::HTTPResponse::HTTP_REASON_BAD_REQUEST);
                std::ostream &ostr = response.send();
                Poco::JSON::Stringifier::stringify(root, ostr);
            } catch (not_found_exception &ex) {
                std::cout << "Not found exception: " << ex.what() << std::endl;
                response.setStatusAndReason(Poco::Net::HTTPResponse::HTTPStatus::HTTP_NOT_FOUND);
                response.setChunkedTransferEncoding(true);
                response.setContentType("application/json");
                Poco::JSON::Object::Ptr root = new Poco::JSON::Object();
                root->set("type", "/errors/not_found");
                root->set("detail", ex.what());
                root->set("status", Poco::Net::HTTPResponse::HTTP_REASON_NOT_FOUND);
                std::ostream &ostr = response.send();
                Poco::JSON::Stringifier::stringify(root, ostr);
            } catch (access_denied_exception &ex) {   
                std::cout << "Access denied exception: " << ex.what() << std::endl;
                response.setStatusAndReason(Poco::Net::HTTPResponse::HTTPStatus::HTTP_UNAUTHORIZED);
                response.setChunkedTransferEncoding(true);
                response.setContentType("application/json");
                Poco::JSON::Object::Ptr root = new Poco::JSON::Object();
                root->set("type", "/errors/anauthorized");
                root->set("detail", ex.what());
                root->set("status", Poco::Net::HTTPResponse::HTTP_REASON_UNAUTHORIZED);
                std::ostream &ostr = response.send();
                Poco::JSON::Stringifier::stringify(root, ostr);
            } catch (std::exception &ex) {
                std::cout << "Server exception: " << ex.what() << std::endl;
                response.setStatusAndReason(Poco::Net::HTTPResponse::HTTPStatus::HTTP_INTERNAL_SERVER_ERROR);
                response.setChunkedTransferEncoding(true);
                response.setContentType("application/json");
                Poco::JSON::Object::Ptr root = new Poco::JSON::Object();
                root->set("type", "/errors/server_error");
                root->set("title", "Internal exception");
                root->set("status", Poco::Net::HTTPResponse::HTTP_REASON_INTERNAL_SERVER_ERROR);
                root->set("detail", ex.what());
                std::ostream &ostr = response.send();
                Poco::JSON::Stringifier::stringify(root, ostr);
            }
            if (!response.sent()) {
                response.setStatusAndReason(Poco::Net::HTTPResponse::HTTPStatus::HTTP_NOT_FOUND);
                response.setChunkedTransferEncoding(true);
                response.setContentType("application/json");
                Poco::JSON::Object::Ptr root = new Poco::JSON::Object();
                root->set("type", "/errors/not_found");
                root->set("detail", "Requested page is not available");
                root->set("status", Poco::Net::HTTPResponse::HTTP_REASON_NOT_FOUND);
                std::ostream &ostr = response.send();
                Poco::JSON::Stringifier::stringify(root, ostr);
            }
        }
    private:
        std::string _format;
};


class HTTPAuthRequestFactory : public HTTPRequestHandlerFactory {
    public:
        HTTPAuthRequestFactory(const std::string &format) : _format(format){}
        HTTPRequestHandler *createRequestHandler([[maybe_unused]] const HTTPServerRequest &request){
            std::cout << "request [" << request.getMethod() << "] " << request.getURI()<< std::endl;

            if (request.getURI().rfind("/auth") == 0 || 
                (request.getURI().rfind("http") == 0 && hasSubstr(request.getURI(), "/auth"))) {
                return new AuthRequestHandler(_format);
            }
            return 0;
        }
    private:
        std::string _format;
};

class HTTPAuthWebServer : public Poco::Util::ServerApplication {
    public:
        HTTPAuthWebServer() : _helpRequested(false){}
        ~HTTPAuthWebServer() {}
    protected:
        void initialize(Application &self) {
            loadConfiguration();
            ServerApplication::initialize(self);
        }
        void uninitialize() {
            ServerApplication::uninitialize();
        }
        int main([[maybe_unused]] const std::vector<std::string> &args) {
            const char * portValue = "8080";
            if (std::getenv("AUTH_SERVICE_PORT") != nullptr) {
                portValue = std::getenv("AUTH_SERVICE_PORT");
            }
            if (strlen(portValue) == 0) {
                std::cout << "Port value is missing" << std::endl;
                return Application::EXIT_DATAERR;
            }

            if (std::getenv("CREATE_TEST_USERS") != nullptr) {
                database::User::create_test_users();
            }

            if (!_helpRequested) {
                ServerSocket svs(Poco::Net::SocketAddress("0.0.0.0", atoi(portValue)));
                HTTPServer srv(new HTTPAuthRequestFactory(DateTimeFormat::SORTABLE_FORMAT), svs, new HTTPServerParams);
                srv.start();
                std::cout << "auth server started on port " << portValue << std::endl;
                waitForTerminationRequest();
                srv.stop();
                std::cout << "auth server stoped" << std::endl;
            }
            return Application::EXIT_OK;
        }
    private:
        bool _helpRequested;
};

#endif
