#include "curl_manager.h"
#include <string>

CurlManager::CurlManager()
{
        curl_global_init(CURL_GLOBAL_ALL);
        googleAuthCurl = curl_easy_init();
        if (!googleAuthCurl)
                return;
        curl_easy_setopt(googleAuthCurl, CURLOPT_WRITEFUNCTION, +[](char *ptr, size_t size, size_t nmemb, void *userdata) -> size_t
                         {
              std::string *str = static_cast<std::string *>(userdata);
              str->append(ptr, size * nmemb);
              return size * nmemb; });
        curl_easy_setopt(googleAuthCurl, CURLOPT_URL, "http://localhost:8000/verify");
        curl_easy_setopt(googleAuthCurl, CURLOPT_POST, 1L);
        curl_easy_setopt(googleAuthCurl, CURLOPT_TIMEOUT, 5L);
        curl_easy_setopt(googleAuthCurl, CURLOPT_CONNECTTIMEOUT, 5L);
        struct curl_slist *headers = nullptr;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        curl_easy_setopt(googleAuthCurl, CURLOPT_HTTPHEADER, headers);
        curl_slist_free_all(headers);

        FcmCurl = curl_easy_init();
        if (!FcmCurl)
                return;
        curl_easy_setopt(FcmCurl, CURLOPT_WRITEFUNCTION, +[](char *ptr, size_t size, size_t nmemb, void *userdata) -> size_t
                         {
              std::string *str = static_cast<std::string *>(userdata);
              str->append(ptr, size * nmemb);
              return size * nmemb; });
        curl_easy_setopt(FcmCurl, CURLOPT_URL, "http://localhost:8000/send-notification");
        curl_easy_setopt(FcmCurl, CURLOPT_POST, 1L);
        curl_easy_setopt(FcmCurl, CURLOPT_TIMEOUT, 5L);
        curl_easy_setopt(FcmCurl, CURLOPT_CONNECTTIMEOUT, 5L);
        struct curl_slist *headers = nullptr;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        curl_easy_setopt(FcmCurl, CURLOPT_HTTPHEADER, headers);
        curl_slist_free_all(headers);
        valid = true;
}

CurlManager::~CurlManager()
{
}
