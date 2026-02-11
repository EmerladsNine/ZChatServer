#pragma once
#include "networking_manager.h"
#include "database.h"
#include "security/argon_hash.h"

class Services
{
private:
public:
        Services();
        NetworkingManager *networkingManager;
        Database *db;
        ArgonHash *argonHash;
        ~Services();
};
