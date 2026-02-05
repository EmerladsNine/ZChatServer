#include "email_auth.h"
#include "../unit_type.h"

EmailAuth::EmailAuth()
{
}

EmailAuth::~EmailAuth()
{
}

void EmailAuth::SignIn(Client &client, size_t expectedSize)
{
        NetworkingManager *networkManager = client.networkingManager;
        uint8_t emailLength = static_cast<uint8_t>(client.buf[EMAIL_LENGTH_OFFSET]);
        if (emailLength < 3 || emailLength > 254 || client.buf.size() < EMAIL_OFFSET + emailLength)
                return networkManager->sendResponseCode(client, ResponseCode::emailAccountInvalidEmailLengthError);
        std::string email(
            reinterpret_cast<const char *>(&client.buf[EMAIL_OFFSET]),
            emailLength);
        std::cout << static_cast<int>(emailLength) << " : " << email << std::endl;

        size_t passwordOffset = EMAIL_OFFSET + emailLength;
        size_t passwordLength = expectedSize - passwordOffset;
        if (passwordLength < 8 || passwordLength > 254)
                return networkManager->sendResponseCode(client, ResponseCode::emailAccountInvalidPasswordLengthError);
        std::string password(
            reinterpret_cast<const char *>(&client.buf[passwordOffset]),
            passwordLength);
        std::cout << passwordLength << " : " << password << std::endl;

        // Email and Password Check
        bool emailExists;
        bool status;
        std::string passHash = client.db->getPasswordHash(email.c_str(), emailExists, status);
        if (!status)
                return networkManager->sendResponseCode(client, ResponseCode::emailSignInFailureError);
        if (!emailExists)
                return networkManager->sendResponseCode(client, ResponseCode::emailSignInEmailNotExistError);
        if (passHash != password)
                return networkManager->sendResponseCode(client, ResponseCode::emailSignInPasswordIncorrectError);

        // Todo send a session id
        networkManager->sendResponseCode(client, ResponseCode::emailSignInDone);
}

void EmailAuth::SignUp(Client &client, size_t expectedSize)
{
        NetworkingManager *networkManager = client.networkingManager;
        uint8_t emailLength = static_cast<uint8_t>(client.buf[EMAIL_LENGTH_OFFSET]);
        if (emailLength < 3 || emailLength > 254 || client.buf.size() < EMAIL_OFFSET + emailLength + PASSWORD_LENGTH_SIZE)
                return networkManager->sendResponseCode(client, ResponseCode::emailAccountInvalidEmailLengthError);

        std::string email(
            reinterpret_cast<const char *>(&client.buf[EMAIL_OFFSET]),
            emailLength);

        size_t passwordLengthOffset = EMAIL_OFFSET + emailLength;
        size_t passwordLength = static_cast<uint8_t>(client.buf[passwordLengthOffset]);
        size_t passwordOffset = passwordLengthOffset + PASSWORD_LENGTH_SIZE;

        if (passwordLength < 8 || passwordLength > 254 || client.buf.size() < passwordOffset + passwordLength)
                return networkManager->sendResponseCode(client, ResponseCode::emailAccountInvalidPasswordLengthError);
        std::string password(
            reinterpret_cast<const char *>(&client.buf[passwordOffset]),
            passwordLength);

        size_t usernameOffset = passwordOffset + passwordLength;
        size_t usernameLength = expectedSize - usernameOffset;
        if (usernameLength == 0 || usernameLength > 12)
                return networkManager->sendResponseCode(client, ResponseCode::emailAccountInvalidUsernameLengthError);
        std::string username(
            reinterpret_cast<const char *>(&client.buf[usernameOffset]),
            usernameLength);

        // Creating account.
        // Check Email.
        bool emailExists;
        bool status;
        client.db->objExists(client.db->check_email_exists_stmt, email.c_str(), emailExists, status);
        if (!status)
                return networkManager->sendResponseCode(client, ResponseCode::emailAccountCreationFailureError);
        if (emailExists)
                return networkManager->sendResponseCode(client, ResponseCode::emailAccountEmailExistError);
        // Check Username.
        bool usernameExists;
        client.db->objExists(client.db->check_username_exists_stmt, username.c_str(), usernameExists, status);
        if (!status)
                return networkManager->sendResponseCode(client, ResponseCode::emailAccountCreationFailureError);
        if (usernameExists)
                return networkManager->sendResponseCode(client, ResponseCode::emailAccountUsernameExistError);

        // Create.
        if (!client.db->insertEmailAccount(username.c_str(), email.c_str(), password.c_str()))
                return networkManager->sendResponseCode(client, ResponseCode::emailAccountCreationFailureError);

        // Todo send a session id
        return networkManager->sendResponseCode(client, ResponseCode::emailAccountCreated);
}
