#pragma once
#include <vector>
#include "account.h"

struct Message
{
        userIdType senderUserId;
        sessionIdType receiverSessionId;
        std::vector<char> message;
        int64_t timestamp;

        Message()
            : senderUserId(0), receiverSessionId(0), timestamp(0) {}

        Message(userIdType sender, sessionIdType receiver, std::vector<char> msg, int64_t ts)
            : senderUserId(sender), receiverSessionId(receiver), message(std::move(msg)), timestamp(ts) {}
};