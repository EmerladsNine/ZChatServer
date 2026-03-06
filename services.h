#pragma once
#include "networking_manager.h"
#include "database_managment/database.h"
#include "security/argon_hash.h"
#include "security/token_generator.h"

class Services
{
private:
public:
        Services();
        NetworkingManager networkingManager;
        Database db;
        ArgonHash argonHash;
        TokenGenerator tokenGen;
        ~Services();
};
