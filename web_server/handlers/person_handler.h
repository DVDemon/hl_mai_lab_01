#ifndef PERSONHANDLER_H
#define PERSONHANDLER_H

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

#include "Poco/JSON/Parser.h"
#include "Poco/JSON/Object.h"

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

#include "../../database/person.h"

class PersonHandler : public HTTPRequestHandler
{
public:
    PersonHandler(const std::string &format) : _format(format)
    {
    }

    void getByLogin(const HTMLForm& form, bool no_cache, std::ostream &ostr) {
        const std::string login = form.get("login");
        if (!no_cache) {
            try
            {
                database::Person result = database::Person::read_by_login_from_cache(login);
                Poco::JSON::Stringifier::stringify(result.toJSON(), ostr);
                return;
            }
            catch (...)
            {
                std::cout << "cache missed for login:" << login << std::endl;
            }
        }
        database::Person result = database::Person::read_by_login(login);

        if (!no_cache) {
            result.save_to_cache();
        }
        Poco::JSON::Stringifier::stringify(result.toJSON(), ostr);
    }

    void getByName(const HTMLForm& form, std::ostream &ostr) {
        std::string  fn = form.get("first_name", "");
        std::string  ln = form.get("last_name", "");

        auto results = database::Person::search(fn,ln);
        if (results.empty()) {
            ostr << "{ \"result\": false , \"reason\": \"not found\" }";
            return;
        }

        Poco::JSON::Array arr;
        for (auto s : results)
            arr.add(s.toJSON());
        Poco::JSON::Stringifier::stringify(arr, ostr);
    }

    void createPerson(HTTPServerRequest &request, std::ostream &ostr) {
        try
        {
            Poco::JSON::Parser parser;
            Poco::JSON::Object::Ptr json = parser.parse(request.stream()).extract<Poco::JSON::Object::Ptr>();

            if (json->has("login") && json->has("age") && json->has("last_name") &&
                json->has("first_name"))
            {
                database::Person person(
                        json->getValue<std::string>("login"),
                        json->getValue<std::string>("first_name"),
                        json->getValue<std::string>("last_name"),
                        json->getValue<int>("age")
                                );

                person.save_to_mysql();
                person.save_to_cache();
                ostr << "{ \"result\": true }";

            } else {
                ostr << "{\"result\": \"Not all fields exist: [login, age, last_name, first_name]\"}";
            }
        } catch(Poco::JSON::JSONException& jsone)
        {
            ostr << "{\"result\": \"Error while parsing. "<< jsone.message() << "\"}";
        }
    }

    void handleRequest(HTTPServerRequest &request,
                       HTTPServerResponse &response)
    {
        response.setChunkedTransferEncoding(true);
        response.setContentType("application/json");
        std::ostream &ostr = response.send();


        if (request.getMethod() == "GET") { // Searching smth
            try {
                HTMLForm form(request, request.stream());
                bool no_cache = false;
                if (form.has("no_cache"))
                    no_cache = true;
                if (form.has("login")) {
                    getByLogin(form, no_cache, ostr);
                    return;
                } else if ((form.has("first_name")) || (form.has("last_name"))) {
                    getByName(form, ostr);
                    return;
                }
            }  catch (...)
            {
                ostr << "{ \"result\": false , \"reason\": \"not found\" }";
                return;
            }

            auto results = database::Person::read_all();
            Poco::JSON::Array arr;
            for (auto s : results)
                arr.add(s.toJSON());
            Poco::JSON::Stringifier::stringify(arr, ostr);
        } else { // adding new person
            try
            {
                createPerson(request, ostr);
            }
            catch (...)
            {
                ostr << "{ \"result\": false , \"reason\": \" database error\" }";
                return;
            }
        }
    }

private:
    std::string _format;
};
#endif // !PERSONHANDLER_H
