#include "fcm_handler.h"
#include "../protocol.h"
#include "../unit_type.h"

void FcmHandler::handleFcmToken(Client &client, expected_size expectedSize, Services &services)
{
        if (!client.inSession || !client.isSessionValid)
                return services.networkingManager.sendSessionStateResponseCode(client, SessionStateResponseCode::NotAuthenticated);
        const size_t FCM_TOKEN_OFFSET = HEADER_OFFSET + HEAD_SIZE;
        size_t fcmTokenLength = expectedSize - FCM_TOKEN_OFFSET;
        if (fcmTokenLength == 0)
                return;
        std::string fcmTok(
            reinterpret_cast<const char *>(&client.buf[FCM_TOKEN_OFFSET]),
            fcmTokenLength);
        services.db.removeFcmToken(fcmTok.c_str()); // Ensure fcm token is not used somewhere else.
        services.db.updateFcmToken(client.session.sessionId, fcmTok.c_str());
        client.session.fcmToken = fcmTok;

        std::vector<char> packet;
        packet.push_back(UnitType::syncFcmTokenResponse);
        services.networkingManager.secure_send(client, packet);
}