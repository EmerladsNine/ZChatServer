#pragma once
#include "curl/curl.h"

class CurlManager
{
private:
public:
        bool valid = false;
        CURL *googleAuthCurl;
        CURL *FcmCurl;
        CurlManager();
        ~CurlManager();
};