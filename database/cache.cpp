#include "cache.h"
#include "../config/config.h"


#include <ignite/ignite.h>
#include <ignite/ignition.h>
#include <exception>

namespace database
{
    Cache::Cache(ignite::Ignite &client) : _client(client),_cache(_client.GetCache<std::string, std::string>("persons"))
    {

    }

    Cache& Cache::get()
    {
        static ignite::IgniteConfiguration cfg;
        cfg.springCfgPath = "config/config.xml";
        static  ignite::Ignite client  = ignite::Ignition::Start(cfg);
        static Cache instance(client);
        return instance;
    }

    void Cache::put(const std::string& login, const std::string& val){
        _cache.Put(login, val);

    }

    void Cache::remove(std::string& login){
        _cache.Remove(login);
    }

    size_t Cache::size(){
        return 0;//cache.GetSize(ignite::thin::cache::CachePeekMode::ALL);
    }

    void Cache::remove_all(){
        _cache.RemoveAll();;
    }

    bool Cache::get(const std::string& login, std::string& val){
        try{
            val = _cache.Get(login);
            return true;
        }catch(...){
            throw std::logic_error("key not found in cache");
        }
    }

    Cache::~Cache(){
        std::cout << " ----------------------------------------" << std::endl;
        ignite::Ignition::StopAll(false);
    }
}