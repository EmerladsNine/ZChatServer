#include "../handlers/account_handler.h"
#include "../protocol.h"
#include "../unit_type.h"
#include "../utils/endian_codec.h"

void AccountHandler::EmailSignIn(Client &client, expected_size expectedSize, Services &services)
{
        const uint16_t EMAIN_LENGTH_SIZE = 1;
        const size_t EMAIL_LENGTH_OFFSET = HEADER_OFFSET + HEAD_SIZE;
        const size_t EMAIL_OFFSET = EMAIL_LENGTH_OFFSET + EMAIN_LENGTH_SIZE;
        NetworkingManager *networkManager = &services.networkingManager;
        uint8_t emailLength = static_cast<uint8_t>(client.buf[EMAIL_LENGTH_OFFSET]);
        if (emailLength < EMAIL_LENGTH_MIN || emailLength > EMAIL_LENGTH_MAX || client.buf.size() < EMAIL_OFFSET + emailLength)
                return networkManager->sendAuthResponseCode(client, AuthResponseCode::emailAccountInvalidEmailError);
        std::string email(
            reinterpret_cast<const char *>(&client.buf[EMAIL_OFFSET]),
            emailLength);
        if (!isEmailValid(email))
                return networkManager->sendAuthResponseCode(client, AuthResponseCode::emailAccountInvalidEmailError);

        size_t passwordOffset = EMAIL_OFFSET + emailLength;
        size_t passwordLength = expectedSize - passwordOffset;
        if (passwordLength < PASSWORD_LENGTH_MIN || passwordLength > PASSWORD_LENGTH_MAX)
                return networkManager->sendAuthResponseCode(client, AuthResponseCode::emailAccountInvalidPasswordLengthError);
        std::string password(
            reinterpret_cast<const char *>(&client.buf[passwordOffset]),
            passwordLength);

        // Email and Password Check
        bool emailExists;
        Account account;
        bool status = services.db.getAccountFromEmail(email.c_str(), account, emailExists);
        if (!status)
                return networkManager->sendAuthResponseCode(client, AuthResponseCode::emailSignInFailureError);
        if (!emailExists)
                return networkManager->sendAuthResponseCode(client, AuthResponseCode::emailSignInEmailNotExistError);

        bool isEqual;
        if (!services.hashManager.argonHash.verifyPassword(password.c_str(), password.length(), account.passHash.c_str(), isEqual))
                return networkManager->sendAuthResponseCode(client, AuthResponseCode::emailSignInFailureError);
        if (!isEqual)
                return networkManager->sendAuthResponseCode(client, AuthResponseCode::emailSignInPasswordIncorrectError);

        // Generate Session.
        std::vector<char> accessToken;
        std::vector<char> refreshToken;
        std::string accessTokenHash;
        std::string refreshTokenHash;
        if (!services.tokenGen.generateSession(services, accessToken, refreshToken, accessTokenHash, refreshTokenHash))
                return networkManager->sendAuthResponseCode(client, AuthResponseCode::emailSignInFailureError);
        if (!services.db.insertSession(account.id, accessTokenHash, refreshTokenHash.data()))
                return networkManager->sendAuthResponseCode(client, AuthResponseCode::emailSignInFailureError);
        sessionIdType sessionId;
        if (!services.db.getLastInsertedId(sessionId))
                return networkManager->sendAuthResponseCode(client, AuthResponseCode::emailSignInFailureError);

        Session session;
        bool isFound;
        if (!services.db.getSessionFromId(sessionId, session, isFound))
                return networkManager->sendAuthResponseCode(client, AuthResponseCode::emailSignInFailureError);
        if (!isFound)
                return networkManager->sendAuthResponseCode(client, AuthResponseCode::emailSignInFailureError);

        // Authenticate client.
        client.authenticate(services, session);

        // Send Session.
        std::vector<char> packet;
        packet.push_back(UnitType::authResponseCode);
        packet.push_back(AuthResponseCode::emailSignInDone);
        std::vector<char> idVec = intToBigEndian<int>(account.id);
        std::vector<char> sessionIdVec = intToBigEndian<int>(sessionId);
        packet.insert(packet.end(), idVec.begin(), idVec.end());
        packet.insert(packet.end(), sessionIdVec.begin(), sessionIdVec.end());
        packet.insert(packet.end(), accessToken.begin(), accessToken.end());
        packet.insert(packet.end(), refreshToken.begin(), refreshToken.end());
        networkManager->secure_send(client, packet);
}

void AccountHandler::EmailSignUp(Client &client, expected_size expectedSize, Services &services)
{
        const uint16_t EMAIN_LENGTH_SIZE = 1;
        const uint16_t PASSWORD_LENGTH_SIZE = 1;
        const size_t EMAIL_LENGTH_OFFSET = HEADER_OFFSET + HEAD_SIZE;
        const size_t EMAIL_OFFSET = EMAIL_LENGTH_OFFSET + EMAIN_LENGTH_SIZE;
        NetworkingManager *networkManager = &services.networkingManager;
        uint8_t emailLength = static_cast<uint8_t>(client.buf[EMAIL_LENGTH_OFFSET]);
        if (emailLength < EMAIL_LENGTH_MIN || emailLength > EMAIL_LENGTH_MAX || client.buf.size() < EMAIL_OFFSET + emailLength + PASSWORD_LENGTH_SIZE)
                return networkManager->sendAuthResponseCode(client, AuthResponseCode::emailAccountInvalidEmailError);

        std::string email(
            reinterpret_cast<const char *>(&client.buf[EMAIL_OFFSET]),
            emailLength);
        if (!isEmailValid(email))
                return networkManager->sendAuthResponseCode(client, AuthResponseCode::emailAccountInvalidEmailError);

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
                return networkManager->sendAuthResponseCode(client, AuthResponseCode::emailAccountInvalidUsernameError);
        std::string username(
            reinterpret_cast<const char *>(&client.buf[usernameOffset]),
            usernameLength);
        if (!isUsernameValid(username))
                return networkManager->sendAuthResponseCode(client, AuthResponseCode::emailAccountInvalidUsernameError);

        // Check Email And Username.
        bool emailExists;
        bool status;
        services.db.objExists(services.db.check_email_exists_stmt, email.c_str(), emailExists, status);
        if (!status)
                return networkManager->sendAuthResponseCode(client, AuthResponseCode::emailAccountCreationFailureError);
        if (emailExists)
                return networkManager->sendAuthResponseCode(client, AuthResponseCode::emailAccountEmailExistError);
        bool usernameExists;
        services.db.objExists(services.db.get_account_from_username_stmt, username.c_str(), usernameExists, status);
        if (!status)
                return networkManager->sendAuthResponseCode(client, AuthResponseCode::emailAccountCreationFailureError);
        if (usernameExists)
                return networkManager->sendAuthResponseCode(client, AuthResponseCode::emailAccountUsernameExistError);

        // Hash Password
        std::string hashedPassword;
        if (!services.hashManager.argonHash.Hash(password, hashedPassword))
                return networkManager->sendAuthResponseCode(client, AuthResponseCode::emailAccountCreationFailureError);

        // Create Email Account.
        if (!services.db.insertEmailAccount(username.c_str(), email.c_str(), hashedPassword.c_str()))
                return networkManager->sendAuthResponseCode(client, AuthResponseCode::emailAccountCreationFailureError);
        userIdType id;
        if (!services.db.getLastInsertedId(id))
                return networkManager->sendAuthResponseCode(client, AuthResponseCode::emailAccountCreationFailureError);

        // Generate Session.
        std::vector<char> accessToken;
        std::vector<char> refreshToken;
        std::string accessTokenHash;
        std::string refreshTokenHash;
        if (!services.tokenGen.generateSession(services, accessToken, refreshToken, accessTokenHash, refreshTokenHash))
                return networkManager->sendAuthResponseCode(client, AuthResponseCode::emailAccountCreationFailureError);
        if (!services.db.insertSession(id, accessTokenHash, refreshTokenHash.data()))
                return networkManager->sendAuthResponseCode(client, AuthResponseCode::emailAccountCreationFailureError);
        sessionIdType sessionId;
        if (!services.db.getLastInsertedId(sessionId))
                return networkManager->sendAuthResponseCode(client, AuthResponseCode::emailAccountCreationFailureError);
        Session session;
        bool isFound;
        if (!services.db.getSessionFromId(sessionId, session, isFound))
                return networkManager->sendAuthResponseCode(client, AuthResponseCode::emailAccountCreationFailureError);
        if (!isFound)
                return networkManager->sendAuthResponseCode(client, AuthResponseCode::emailAccountCreationFailureError);

        // Authenticate client.
        client.authenticate(services, session);

        // Send Session.
        std::vector<char> packet;
        packet.push_back(UnitType::authResponseCode);
        packet.push_back(AuthResponseCode::emailAccountCreated);
        std::vector<char> idVec = intToBigEndian<int>(id);
        std::vector<char> sessionIdVec = intToBigEndian<int>(sessionId);
        packet.insert(packet.end(), idVec.begin(), idVec.end());
        packet.insert(packet.end(), sessionIdVec.begin(), sessionIdVec.end());
        packet.insert(packet.end(), accessToken.begin(), accessToken.end());
        packet.insert(packet.end(), refreshToken.begin(), refreshToken.end());
        networkManager->secure_send(client, packet);
}
