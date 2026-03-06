#include "../handlers/account_handler.h"
#include "../protocol.h"
#include "../unit_type.h"

void AccountHandler::EmailSignIn(Client &client, size_t expectedSize, Services &services)
{
        const uint16_t EMAIN_LENGTH_SIZE = 1;
        const size_t EMAIL_LENGTH_OFFSET = HEADER_OFFSET + HEAD_SIZE;
        const size_t EMAIL_OFFSET = EMAIL_LENGTH_OFFSET + EMAIN_LENGTH_SIZE;
        NetworkingManager *networkManager = &services.networkingManager;
        uint8_t emailLength = static_cast<uint8_t>(client.buf[EMAIL_LENGTH_OFFSET]);
        if (emailLength < EMAIL_LENGTH_MIN || emailLength > EMAIL_LENGTH_MAX || client.buf.size() < EMAIL_OFFSET + emailLength)
                return networkManager->sendAuthResponseCode(client, AuthResponseCode::emailAccountInvalidEmailLengthError);
        std::string email(
            reinterpret_cast<const char *>(&client.buf[EMAIL_OFFSET]),
            emailLength);

        size_t passwordOffset = EMAIL_OFFSET + emailLength;
        size_t passwordLength = expectedSize - passwordOffset;
        if (passwordLength < PASSWORD_LENGTH_MIN || passwordLength > PASSWORD_LENGTH_MAX)
                return networkManager->sendAuthResponseCode(client, AuthResponseCode::emailAccountInvalidPasswordLengthError);
        std::string password(
            reinterpret_cast<const char *>(&client.buf[passwordOffset]),
            passwordLength);

        // Email and Password Check
        bool emailExists;
        bool status;
        std::string passHash = services.db.getPasswordHash(email.c_str(), emailExists, status);
        if (!status)
                return networkManager->sendAuthResponseCode(client, AuthResponseCode::emailSignInFailureError);
        if (!emailExists)
                return networkManager->sendAuthResponseCode(client, AuthResponseCode::emailSignInEmailNotExistError);

        int isEqual;
        if (!services.argonHash.verifyPassword(password.c_str(), password.length(), passHash.c_str(), &isEqual))
                return networkManager->sendAuthResponseCode(client, AuthResponseCode::emailSignInFailureError);
        if (!isEqual)
                return networkManager->sendAuthResponseCode(client, AuthResponseCode::emailSignInPasswordIncorrectError);

        std::vector<char> packet;
        packet.push_back(AuthResponseCode::emailSignInDone);
        std::vector<uint8_t> accessToken = services.tokenGen.generateAccessToken();
        std::vector<uint8_t> refreshToken = services.tokenGen.generateRefreshToken();

        //Todo store session tokens
        packet.insert(packet.end(), accessToken.begin(), accessToken.end());
        packet.insert(packet.end(), refreshToken.begin(), refreshToken.end());
        networkManager->secure_send(client, packet);
}

void AccountHandler::EmailSignUp(Client &client, size_t expectedSize, Services &services)
{
        const uint16_t EMAIN_LENGTH_SIZE = 1;
        const uint16_t PASSWORD_LENGTH_SIZE = 1;
        const size_t EMAIL_LENGTH_OFFSET = HEADER_OFFSET + HEAD_SIZE;
        const size_t EMAIL_OFFSET = EMAIL_LENGTH_OFFSET + EMAIN_LENGTH_SIZE;
        NetworkingManager *networkManager = &services.networkingManager;
        uint8_t emailLength = static_cast<uint8_t>(client.buf[EMAIL_LENGTH_OFFSET]);
        if (emailLength < EMAIL_LENGTH_MIN || emailLength > EMAIL_LENGTH_MAX || client.buf.size() < EMAIL_OFFSET + emailLength + PASSWORD_LENGTH_SIZE)
                return networkManager->sendAuthResponseCode(client, AuthResponseCode::emailAccountInvalidEmailLengthError);

        std::string email(
            reinterpret_cast<const char *>(&client.buf[EMAIL_OFFSET]),
            emailLength);

        size_t passwordLengthOffset = EMAIL_OFFSET + emailLength;
        size_t passwordLength = static_cast<uint8_t>(client.buf[passwordLengthOffset]);
        size_t passwordOffset = passwordLengthOffset + PASSWORD_LENGTH_SIZE;

        if (passwordLength < PASSWORD_LENGTH_MIN || passwordLength > PASSWORD_LENGTH_MAX || client.buf.size() < passwordOffset + passwordLength)
                return networkManager->sendAuthResponseCode(client, AuthResponseCode::emailAccountInvalidPasswordLengthError);
        std::string password(
            reinterpret_cast<const char *>(&client.buf[passwordOffset]),
            passwordLength);

        size_t usernameOffset = passwordOffset + passwordLength;
        size_t usernameLength = expectedSize - usernameOffset;
        if (usernameLength < USERNAME_LENGTH_MIN || usernameLength > USERNAME_LENGTH_MAX)
                return networkManager->sendAuthResponseCode(client, AuthResponseCode::emailAccountInvalidUsernameLengthError);
        std::string username(
            reinterpret_cast<const char *>(&client.buf[usernameOffset]),
            usernameLength);

        // Creating account.
        // Check Email.
        bool emailExists;
        bool status;
        services.db.objExists(services.db.check_email_exists_stmt, email.c_str(), emailExists, status);
        if (!status)
                return networkManager->sendAuthResponseCode(client, AuthResponseCode::emailAccountCreationFailureError);
        if (emailExists)
                return networkManager->sendAuthResponseCode(client, AuthResponseCode::emailAccountEmailExistError);
        // Check Username.
        bool usernameExists;
        services.db.objExists(services.db.get_account_from_username_stmt, username.c_str(), usernameExists, status);
        if (!status)
                return networkManager->sendAuthResponseCode(client, AuthResponseCode::emailAccountCreationFailureError);
        if (usernameExists)
                return networkManager->sendAuthResponseCode(client, AuthResponseCode::emailAccountUsernameExistError);

        // Hash
        std::string hashedPassword;
        if (!services.argonHash.Hash(password, hashedPassword))
                return networkManager->sendAuthResponseCode(client, AuthResponseCode::emailAccountCreationFailureError);

        // Create.
        if (!services.db.insertEmailAccount(username.c_str(), email.c_str(), hashedPassword.c_str()))
                return networkManager->sendAuthResponseCode(client, AuthResponseCode::emailAccountCreationFailureError);

        // Todo send a session id
        return networkManager->sendAuthResponseCode(client, AuthResponseCode::emailAccountCreated);
}
