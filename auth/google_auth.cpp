#include "google_auth.h"
#include "../protocol.h"
#include "curl/curl.h"

GoogleAuth::GoogleAuth()
{
}

GoogleAuth::~GoogleAuth()
{
}

void GoogleAuth::StartAuthentication(Client &client, size_t expectedSize)
{
        NetworkingManager *networkManager = client.networkingManager;

        size_t googleTokenLength = expectedSize - GOOGLE_TOKEN_OFFSET;

        if (googleTokenLength == 0)
                return networkManager->sendResponseCode(client, ResponseCode::googleAuthInvalidToken);

        std::string googleToken(
            reinterpret_cast<const char *>(&client.buf[GOOGLE_TOKEN_OFFSET]),
            googleTokenLength);

        CURL *curl = curl_easy_init();
        if (!curl)
                return networkManager->sendResponseCode(client, ResponseCode::googleAuthFailed);

        std::string jsonBody = "{\"token\": \"" + googleToken + "\"}";

        // Prepare response storage
        std::string response;
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, +[](char *ptr, size_t size, size_t nmemb, void *userdata) -> size_t
                         {
              std::string *str = static_cast<std::string *>(userdata);
              str->append(ptr, size * nmemb);
              return size * nmemb; });
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

        curl_easy_setopt(curl, CURLOPT_URL, "http://localhost:8000/verify");
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, jsonBody.c_str());
        curl_easy_setopt(curl, CURLOPT_POST, 1L);

        // Set proper header
        struct curl_slist *headers = nullptr;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        CURLcode res = curl_easy_perform(curl);
        long http_code = 0;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);

        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);

        if (http_code == 401)
                return networkManager->sendResponseCode(client, ResponseCode::googleAuthInvalidToken);
        if (res != CURLE_OK || http_code != 200)
                return networkManager->sendResponseCode(client, ResponseCode::googleAuthFailed);

        // Todo send a session id
        std::cout << response << std::endl;
        return networkManager->sendResponseCode(client, ResponseCode::googleAuthSuccessful);
}
