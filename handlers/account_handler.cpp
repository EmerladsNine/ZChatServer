#include "account_handler.h"
#include <regex>
#include <string>

bool AccountHandler::isEmailValid(const std::string &email)
{
        const std::regex pattern(R"(^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,6}$)");
        return std::regex_match(email, pattern);
}

bool AccountHandler::isUsernameValid(const std::string &username)
{
        for (size_t i = 0; i < username.length(); i++)
        {
                char c = username[i];
                bool isDigit = (c >= 48 && c <= 57);
                bool isUpper = (c >= 65 && c <= 90);
                bool isLower = (c >= 97 && c <= 122);
                bool isSpecial = (c == 45 || c == 95);

                if ((i == 0 || i == username.length() - 1) && isSpecial)
                        return false;

                if (!(isDigit || isUpper || isLower || isSpecial))
                {
                        return false;
                }
        }
        return !username.empty();
}