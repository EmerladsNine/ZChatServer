cmake_minimum_required(VERSION 3.25)

project(ZChatServer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(.)

find_package(SQLite3 REQUIRED)
find_package(CURL REQUIRED)
find_package(PkgConfig REQUIRED)

pkg_check_modules(SODIUM REQUIRED libsodium)

add_executable(ZChatServer
        ./main.cpp
        ./client.cpp 
        ./utils.cpp 
        ./networking_manager.cpp 
        ./protocol.cpp 
        ./services.cpp
        ./account_services/email_auth.cpp 
        ./account_services/google_auth.cpp 
        ./account_services/search_account.cpp 
        ./security/hashing/argon_hash.cpp 
        ./security/hashing/sha256_hash.cpp
        ./security/hashing/hash_manager.cpp 
        ./security/token_generator.cpp
        ./database_managment/database.cpp 
        ./database_managment/data/account.cpp
        ./database_managment/data/session.cpp
        ./database_managment/repositories/account_repository.cpp
        ./database_managment/repositories/session_repository.cpp
        ./database_managment/repositories/messages_repository.cpp
        ./handlers/account_handler.cpp
        ./handlers/token_handler.cpp
        ./handlers/session_list_handler.cpp
)

target_link_directories(ZChatServer PRIVATE ${SODIUM_INCLUDE_DIRS})
target_link_libraries(ZChatServer PRIVATE SQLite::SQLite3 CURL::libcurl ${SODIUM_LIBRARIES})
target_compile_options(ZChatServer PRIVATE ${SODIUM_CFLAGS_OTHER})