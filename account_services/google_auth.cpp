#include "../handlers/account_handler.h"
#include "../protocol.h"
#include "curl/curl.h"
#include "../unit_type.h"
#include "../utils.h"

bool verifyGoogleToken(std::string &token, Client &client, std::string &googleIdOut, Services &services)
{
        NetworkingManager *networkManager = &services.networkingManager;
        CURL *curl = curl_easy_init();
        if (!curl)
        {
                networkManager->sendAuthResponseCode(client, AuthResponseCode::googleAuthFailed);
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
                networkManager->sendAuthResponseCode(client, AuthResponseCode::googleAuthInvalidToken);
                return false;
        }
        if (http_code != 200)
        {
                networkManager->sendAuthResponseCode(client, AuthResponseCode::googleAuthFailed);
                return false;
        }
        return true;
}

void AccountHandler::GoogleSignIn(Client &client, size_t expectedSize, Services &services)
{
        const size_t GOOGLE_TOKEN_OFFSET = HEADER_OFFSET + HEAD_SIZE;
        NetworkingManager *networkManager = &services.networkingManager;
        size_t googleTokenLength = expectedSize - GOOGLE_TOKEN_OFFSET;
        if (googleTokenLength == 0)
                return networkManager->sendAuthResponseCode(client, AuthResponseCode::googleAuthInvalidToken);
        std::string googleToken(
            reinterpret_cast<const char *>(&client.buf[GOOGLE_TOKEN_OFFSET]),
            googleTokenLength);
        std::string googleId;
        if (!verifyGoogleToken(googleToken, client, googleId, services))
                return;
        bool googleIdExists;
        Account account;
        bool status = services.db.getAccountFromGoogleId(googleId.c_str(), account, googleIdExists);
        if (!status)
                return networkManager->sendAuthResponseCode(client, AuthResponseCode::googleAuthFailed);
        if (!googleIdExists)
                return networkManager->sendAuthResponseCode(client, AuthResponseCode::googleAuthRequireSignUp);

        // Generate Session.
        std::vector<char> accessToken;
        std::vector<char> refreshToken;
        std::string accessTokenHash;
        std::string refreshTokenHash;
        if (!services.tokenGen.generateSession(services, accessToken, refreshToken, accessTokenHash, refreshTokenHash))
                return networkManager->sendAuthResponseCode(client, AuthResponseCode::googleAuthFailed);
        if (!services.db.insertSession(account.id, accessTokenHash, refreshTokenHash.data()))
                return networkManager->sendAuthResponseCode(client, AuthResponseCode::googleAuthFailed);

        // Authenticate client.
        client.authenticate(services, account.id, accessTokenHash);

        // Send Session.
        std::vector<char> packet;
        packet.push_back(UnitType::authResponseCode);
        packet.push_back(AuthResponseCode::googleAuthSuccessful);
        std::vector<char> idVec = intToBigEndian<int>(account.id);
        packet.insert(packet.end(), idVec.begin(), idVec.end());
        packet.insert(packet.end(), accessToken.begin(), accessToken.end());
        packet.insert(packet.end(), refreshToken.begin(), refreshToken.end());
        networkManager->secure_send(client, packet);
}

void AccountHandler::GoogleSignUp(Client &client, size_t expectedSize, Services &services)
{
        NetworkingManager *networkManager = &services.networkingManager;
        const size_t USERNAME_LENGTH_SIZE = 1;
        const size_t USERNAME_LENGTH_OFFSET = HEADER_OFFSET + HEAD_SIZE;
        const size_t USERNAME_OFFSET = USERNAME_LENGTH_OFFSET + USERNAME_LENGTH_SIZE;
        // Parse.
        uint8_t usernameLength = static_cast<uint8_t>(client.buf[USERNAME_LENGTH_OFFSET]);
        if (usernameLength < USERNAME_LENGTH_MIN || usernameLength > USERNAME_LENGTH_MAX || client.buf.size() < USERNAME_OFFSET + usernameLength)
                return networkManager->sendAuthResponseCode(client, AuthResponseCode::googleSignUpInvalidUsernameLengthError);
        std::string username(
            reinterpret_cast<const char *>(&client.buf[USERNAME_OFFSET]),
            usernameLength);

        size_t googleTokenOffset = USERNAME_OFFSET + usernameLength;
        size_t googleTokenLength = expectedSize - googleTokenOffset;
        if (googleTokenLength == 0)
                return networkManager->sendAuthResponseCode(client, AuthResponseCode::googleAuthInvalidToken);
        std::string googleToken(
            reinterpret_cast<const char *>(&client.buf[googleTokenOffset]),
            googleTokenLength);

        // Check.
        bool usernameExists;
        bool status;
        services.db.objExists(services.db.get_account_from_username_stmt, username.c_str(), usernameExists, status);
        if (!status)
                return networkManager->sendAuthResponseCode(client, AuthResponseCode::googleAuthFailed);
        if (usernameExists)
                return networkManager->sendAuthResponseCode(client, AuthResponseCode::googleSignUpUsernameExistError);

        std::string googleId;
        if (!verifyGoogleToken(googleToken, client, googleId, services))
                return;
        bool googleIdExists;
        services.db.objExists(services.db.check_google_id_exists_stmt, googleId.c_str(), googleIdExists, status);
        if (!status)
                return networkManager->sendAuthResponseCode(client, AuthResponseCode::googleAuthFailed);
        if (googleIdExists)
                return networkManager->sendAuthResponseCode(client, AuthResponseCode::googleSignUpGoogleIdExistError);

        // Create.
        if (!services.db.insertGoogleAccount(username.c_str(), googleId.c_str()))
                return networkManager->sendAuthResponseCode(client, AuthResponseCode::googleAuthFailed);
        int id;
        if (!services.db.getLastInsertedAccountId(id))
                return networkManager->sendAuthResponseCode(client, AuthResponseCode::googleAuthFailed);

        // Generate Session.
        std::vector<char> accessToken;
        std::vector<char> refreshToken;
        std::string accessTokenHash;
        std::string refreshTokenHash;
        if (!services.tokenGen.generateSession(services, accessToken, refreshToken, accessTokenHash, refreshTokenHash))
                return networkManager->sendAuthResponseCode(client, AuthResponseCode::googleAuthFailed);
        if (!services.db.insertSession(id, accessTokenHash, refreshTokenHash.data()))
                return networkManager->sendAuthResponseCode(client, AuthResponseCode::googleAuthFailed);

        // Authenticate client.
        client.authenticate(services, id, accessTokenHash);

        // Send Session.
        std::vector<char> packet;
        packet.push_back(UnitType::authResponseCode);
        packet.push_back(AuthResponseCode::googleAuthSuccessful);
        std::vector<char> idVec = intToBigEndian<int>(id);
        packet.insert(packet.end(), idVec.begin(), idVec.end());
        packet.insert(packet.end(), accessToken.begin(), accessToken.end());
        packet.insert(packet.end(), refreshToken.begin(), refreshToken.end());
        networkManager->secure_send(client, packet);
}