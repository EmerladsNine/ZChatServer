#pragma once
#include "networking_manager.h"
#include "database_managment/database.h"
#include "security/hashing/hash_manager.h"
#include "security/token_generator.h"
#include "curl_manager/curl_manager.h"

class Services
{
private:
public:
        Services();
        NetworkingManager networkingManager;
        Database db;
        HashManager hashManager;
        TokenGenerator tokenGen;
        CurlManager curlManager;
        ~Services();
};
