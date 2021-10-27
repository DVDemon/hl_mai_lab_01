#ifndef USERHANDLER_H
#define USERHANDLER_H

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
#include "Poco/JSON/Object.h"
#include "Poco/JSON/Parser.h"
#include <iostream>
#include <iostream>
#include <fstream>

using Poco::DateTimeFormat;
using Poco::DateTimeFormatter;
using Poco::ThreadPool;
using Poco::Timestamp;
using Poco::Net::HTMLForm;
using Poco::Net::HTTPRequestHandler;
using Poco::Net::HTTPRequestHandlerFactory;
using Poco::Net::HTTPServer;
using Poco::Net::HTTPServerParams;
using Poco::Net::HTTPServerRequest;
using Poco::Net::HTTPServerResponse;
using Poco::Net::NameValueCollection;
using Poco::Net::ServerSocket;
using Poco::Util::Application;
using Poco::Util::HelpFormatter;
using Poco::Util::Option;
using Poco::Util::OptionCallback;
using Poco::Util::OptionSet;
using Poco::Util::ServerApplication;
using Poco::JSON::Object;
using Poco::JSON::Parser;

#include "../../database/user.h"

class UserHandler : public HTTPRequestHandler
{
private:
    void GetByLogin(HTMLForm &form, std::ostream &ostr) const {
        try
        {
            std::optional<database::User> result = database::User::read_by_login(form.get("login").c_str());
            if (result) {
                Poco::JSON::Stringifier::stringify(result->toJSON(), ostr);
            } else {
                ostr << "{ \"result\": false , \"reason\": \"not found\" }";
            }
        }
        catch (std::exception& s)
        {
            ostr << "{ \"result\": false , \"reason\": \"not found\" }";
        }
    }

    void GetByNamePattern(HTMLForm &form, std::ostream &ostr) const {
        try
        {
            std::optional<std::string> fn = form.has("first_name") ? std::make_optional(form.get("first_name")) : std::nullopt;
            std::optional<std::string> ln = form.has("last_name") ? std::make_optional(form.get("last_name")) : std::nullopt;
            auto results = database::User::search(fn,ln);
            Poco::JSON::Array arr;
            for (auto s : results)
                arr.add(s.toJSON());
            Poco::JSON::Stringifier::stringify(arr, ostr);
        }
        catch (...)
        {
            ostr << "{ \"result\": false , \"reason\": \"not gound\" }";
        }
    }

    void CreateUser(HTTPServerRequest &request, std::ostream &ostr) const {
        std::istream& client_body = request.stream();
		int length_msg = request.getContentLength();
		if (length_msg > 1024) return;
		char buffer[1025];
		client_body.read(buffer, length_msg);
        Parser parser;
        auto result = parser.parse(buffer);

        Object::Ptr object = result.extract<Object::Ptr>();
        database::User user;
        try {
            user.login = object->get("login").toString();
            user.first_name = object->get("first_name").toString();
            user.last_name = object->get("last_name").toString();
            user.age = std::atoi(object->get("age").toString().c_str());
        } catch (...) {
            ostr << "{ \"result\": false , \"reason\": \"bad request\" }";
            return;
        }

        try {
            user.save_to_mysql();
        } catch (...) {
            ostr << "{ \"result\": false , \"reason\": \"mysql error\" }";
            return;
        }

        ostr << "{ \"result\": true }";
    }

public:
    UserHandler(const std::string &format) : _format(format)
    {
    }

    void handleRequest(HTTPServerRequest &request,
                       HTTPServerResponse &response)
    {
        HTMLForm form(request);

        response.setChunkedTransferEncoding(true);
        response.setContentType("application/json");
        std::ostream &ostr = response.send();

        if (request.getMethod() == "GET" && form.has("login")) {
            GetByLogin(form, ostr);
            return;
        } else if (
            request.getMethod() == "GET" &&
            (
                (form.has("first_name") && form.get("first_name").length() > 0) ||
                (form.has("last_name") && form.get("last_name").length() > 0)
            )
        ) {
            GetByNamePattern(form, ostr);
            return;
        } else if (
            request.getMethod() == "POST"
        ) {
            CreateUser(request, ostr);
            return;
        }
    }

private:
    std::string _format;
};
#endif // !USERHANDLER_H
