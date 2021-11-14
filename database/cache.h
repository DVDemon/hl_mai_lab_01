#ifndef CACHE_H
#define CACHE_H

#include <string>
#include <memory>
#include <ignite/ignite.h>
#include <ignite/ignition.h>

namespace database
{
    class Cache
    {
    private:
        ignite::Ignite &_client;
        ignite::cache::Cache<std::string, std::string> _cache;
        Cache(ignite::Ignite &client);

    public:
        static Cache& get();
        void put(const std::string& login, const std::string& val);
        bool get(const std::string& login, std::string& val);
        size_t size();
        void remove(std::string& login);
        void remove_all();
        ~Cache();
    };
}

#endif