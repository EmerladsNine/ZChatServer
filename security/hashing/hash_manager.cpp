#include "hash_manager.h"
#include <sodium.h>
#include <iostream>

HashManager::HashManager()
{
        if (sodium_init() < 0)
        {
                std::cerr << "Failed To Initialize Sodium" << std::endl;
                return;
        }
        argonHash.valid = true;
        sha256Hash.valid = true;
}

HashManager::~HashManager()
{
}