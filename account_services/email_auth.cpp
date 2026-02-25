#include "../handlers/account_handler.h"
#include "../protocol.h"
#include "../unit_type.h"


void AccountHandler::EmailSignIn(Client &client, size_t expectedSize, Services &services)
{
        const uint16_t EMAIN_LENGTH_SIZE = 1;
        const size_t EMAIL_LENGTH_OFFSET = HEADER_OFFSET + HEAD_SIZE;
        const size_t EMAIL_OFFSET = EMAIL_LENGTH_OFFSET + EMAIN_LENGTH_SIZE;
        NetworkingManager *networkManager = services.networkingManager;
        uint8_t emailLength = static_cast<uint8_t>(client.buf[EMAIL_LENGTH_OFFSET]);
        if (emailLength < EMAIL_LENGTH_MIN || emailLength > EMAIL_LENGTH_MAX || client.buf.size() < EMAIL_OFFSET + emailLength)
                return networkManager->sendResponseCode(client, ResponseCode::emailAccountInvalidEmailLengthError);
        std::string email(
            reinterpret_cast<const char *>(&client.buf[EMAIL_OFFSET]),
            emailLength);

        size_t passwordOffset = EMAIL_OFFSET + emailLength;
        size_t passwordLength = expectedSize - passwordOffset;
        if (passwordLength < PASSWORD_LENGTH_MIN || passwordLength > PASSWORD_LENGTH_MAX)
                return networkManager->sendResponseCode(client, ResponseCode::emailAccountInvalidPasswordLengthError);
        std::string password(
            reinterpret_cast<const char *>(&client.buf[passwordOffset]),
            passwordLength);

        // Email and Password Check
        bool emailExists;
        bool status;
        std::string passHash = services.db->getPasswordHash(email.c_str(), emailExists, status);
        if (!status)
                return networkManager->sendResponseCode(client, ResponseCode::emailSignInFailureError);
        if (!emailExists)
                return networkManager->sendResponseCode(client, ResponseCode::emailSignInEmailNotExistError);

        int isEqual;
        if (!services.argonHash->verifyPassword(password.c_str(), password.length(), passHash.c_str(), &isEqual))
                return networkManager->sendResponseCode(client, ResponseCode::emailSignInFailureError);
        if (!isEqual)
                return networkManager->sendResponseCode(client, ResponseCode::emailSignInPasswordIncorrectError);

        // Todo send a session id
        networkManager->sendResponseCode(client, ResponseCode::emailSignInDone);
}

void AccountHandler::EmailSignUp(Client &client, size_t expectedSize, Services &services)
{
        const uint16_t EMAIN_LENGTH_SIZE = 1;
        const uint16_t PASSWORD_LENGTH_SIZE = 1;
        const size_t EMAIL_LENGTH_OFFSET = HEADER_OFFSET + HEAD_SIZE;
        const size_t EMAIL_OFFSET = EMAIL_LENGTH_OFFSET + EMAIN_LENGTH_SIZE;
        NetworkingManager *networkManager = services.networkingManager;
        uint8_t emailLength = static_cast<uint8_t>(client.buf[EMAIL_LENGTH_OFFSET]);
        if (emailLength < EMAIL_LENGTH_MIN || emailLength > EMAIL_LENGTH_MAX || client.buf.size() < EMAIL_OFFSET + emailLength + PASSWORD_LENGTH_SIZE)
                return networkManager->sendResponseCode(client, ResponseCode::emailAccountInvalidEmailLengthError);

        std::string email(
            reinterpret_cast<const char *>(&client.buf[EMAIL_OFFSET]),
            emailLength);

        size_t passwordLengthOffset = EMAIL_OFFSET + emailLength;
        size_t passwordLength = static_cast<uint8_t>(client.buf[passwordLengthOffset]);
        size_t passwordOffset = passwordLengthOffset + PASSWORD_LENGTH_SIZE;

        if (passwordLength < PASSWORD_LENGTH_MIN || passwordLength > PASSWORD_LENGTH_MAX || client.buf.size() < passwordOffset + passwordLength)
                return networkManager->sendResponseCode(client, ResponseCode::emailAccountInvalidPasswordLengthError);
        std::string password(
            reinterpret_cast<const char *>(&client.buf[passwordOffset]),
            passwordLength);

        size_t usernameOffset = passwordOffset + passwordLength;
        size_t usernameLength = expectedSize - usernameOffset;
        if (usernameLength == 0 || usernameLength > USERNAME_LENGTH_MAX)
                return networkManager->sendResponseCode(client, ResponseCode::emailAccountInvalidUsernameLengthError);
        std::string username(
            reinterpret_cast<const char *>(&client.buf[usernameOffset]),
            usernameLength);

        // Creating account.
        // Check Email.
        bool emailExists;
        bool status;
        services.db->objExists(services.db->check_email_exists_stmt, email.c_str(), emailExists, status);
        if (!status)
                return networkManager->sendResponseCode(client, ResponseCode::emailAccountCreationFailureError);
        if (emailExists)
                return networkManager->sendResponseCode(client, ResponseCode::emailAccountEmailExistError);
        // Check Username.
        bool usernameExists;
        services.db->objExists(services.db->get_account_from_username_stmt, username.c_str(), usernameExists, status);
        if (!status)
                return networkManager->sendResponseCode(client, ResponseCode::emailAccountCreationFailureError);
        if (usernameExists)
                return networkManager->sendResponseCode(client, ResponseCode::emailAccountUsernameExistError);

        // Hash
        std::string hashedPassword;
        if (!services.argonHash->Hash(password, hashedPassword))
                return networkManager->sendResponseCode(client, ResponseCode::emailAccountCreationFailureError);

        // Create.
        if (!services.db->insertEmailAccount(username.c_str(), email.c_str(), hashedPassword.c_str()))
                return networkManager->sendResponseCode(client, ResponseCode::emailAccountCreationFailureError);

        // Todo send a session id
        return networkManager->sendResponseCode(client, ResponseCode::emailAccountCreated);
}
