#include "google_auth.h"
#include "../protocol.h"
#include "curl/curl.h"

GoogleAuth::GoogleAuth()
{
}

GoogleAuth::~GoogleAuth()
{
}

bool verifyGoogleToken(std::string &token, Client &client, std::string &googleIdOut)
{
        NetworkingManager *networkManager = client.networkingManager;
        CURL *curl = curl_easy_init();
        if (!curl)
        {
                networkManager->sendResponseCode(client, ResponseCode::googleAuthFailed);
                return false;
        }
        std::string jsonBody = "{\"token\": \"" + token + "\"}";
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, +[](char *ptr, size_t size, size_t nmemb, void *userdata) -> size_t
                         {
              std::string *str = static_cast<std::string *>(userdata);
              str->append(ptr, size * nmemb);
              return size * nmemb; });
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &googleIdOut);
        curl_easy_setopt(curl, CURLOPT_URL, "http://localhost:8000/verify");
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, jsonBody.c_str());
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 5L);
        curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 5L);
        struct curl_slist *headers = nullptr;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        CURLcode res = curl_easy_perform(curl);
        long http_code = 0;
        if (res == CURLE_OK)
                curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
        if (http_code == 401)
        {
                networkManager->sendResponseCode(client, ResponseCode::googleAuthInvalidToken);
                return false;
        }
        if (http_code != 200)
        {
                networkManager->sendResponseCode(client, ResponseCode::googleAuthFailed);
                return false;
        }
        return true;
}

void GoogleAuth::SignIn(Client &client, size_t expectedSize)
{
        NetworkingManager *networkManager = client.networkingManager;
        size_t googleTokenLength = expectedSize - GOOGLE_TOKEN_SIGNIN_OFFSET;
        if (googleTokenLength == 0)
                return networkManager->sendResponseCode(client, ResponseCode::googleAuthInvalidToken);
        std::string googleToken(
            reinterpret_cast<const char *>(&client.buf[GOOGLE_TOKEN_SIGNIN_OFFSET]),
            googleTokenLength);
        std::string googleId;
        if (!verifyGoogleToken(googleToken, client, googleId))
                return;
        bool googleIdExists;
        bool status;
        client.db->objExists(client.db->check_google_id_exists_stmt, googleId.c_str(), googleIdExists, status);
        if (!status)
                return networkManager->sendResponseCode(client, ResponseCode::googleAuthFailed);
        if (!googleIdExists)
                return networkManager->sendResponseCode(client, ResponseCode::googleAuthRequireSignUp);
        // Todo send a session id.
        return networkManager->sendResponseCode(client, ResponseCode::googleAuthSuccessful);
}

void GoogleAuth::SignUp(Client &client, size_t expectedSize)
{
        NetworkingManager *networkManager = client.networkingManager;

        // Parse.
        uint8_t usernameLength = static_cast<uint8_t>(client.buf[USERNAME_LENGTH_OFFSET]);
        if (usernameLength == 0 || usernameLength > 12 || client.buf.size() < USERNAME_OFFSET + usernameLength)
                return networkManager->sendResponseCode(client, ResponseCode::googleSignUpInvalidUsernameLengthError);
        std::string username(
            reinterpret_cast<const char *>(&client.buf[USERNAME_OFFSET]),
            usernameLength);

        size_t googleTokenOffset = USERNAME_OFFSET + usernameLength;
        size_t googleTokenLength = expectedSize - googleTokenOffset;
        if (googleTokenLength == 0)
                return networkManager->sendResponseCode(client, ResponseCode::googleAuthInvalidToken);
        std::string googleToken(
            reinterpret_cast<const char *>(&client.buf[googleTokenOffset]),
            googleTokenLength);

        // Check.
        bool usernameExists;
        bool status;
        client.db->objExists(client.db->check_username_exists_stmt, username.c_str(), usernameExists, status);
        if (!status)
                return networkManager->sendResponseCode(client, ResponseCode::googleAuthFailed);
        if (usernameExists)
                return networkManager->sendResponseCode(client, ResponseCode::googleSignUpUsernameExistError);

        std::string googleId;
        if (!verifyGoogleToken(googleToken, client, googleId))
                return;
        bool googleIdExists;
        client.db->objExists(client.db->check_google_id_exists_stmt, googleId.c_str(), googleIdExists, status);
        if (!status)
                return networkManager->sendResponseCode(client, ResponseCode::googleAuthFailed);
        if (googleIdExists)
                return networkManager->sendResponseCode(client, ResponseCode::googleSignUpGoogleIdExistError);

        // Create.
        if (!client.db->insertGoogleAccount(username.c_str(), googleId.c_str()))
                return networkManager->sendResponseCode(client, ResponseCode::googleAuthFailed);

        return networkManager->sendResponseCode(client, ResponseCode::googleAuthSuccessful);
}