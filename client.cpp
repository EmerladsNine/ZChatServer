#include "client.h"
#include "utils.h"
#include <chrono>

using namespace std::chrono;

Client::Client(NetworkingManager *networkingManager)
    : networkingManager(networkingManager)
{
}

Client::~Client()
{
}

ssize_t Client::read()
{
        char temp[4096];
        ssize_t n = recv(fd, temp, sizeof(temp), 0);
        if (n < 0)
        {
                // Todo handle this
                perror("clientRead");
                exit(1);
        }
        else if (n == 0)
        {
                // Client Disconnected.
                close(fd);
                return n;
        }

        if (head == -1)
        {
                head = temp[0];
        }

        // bodyless units
        switch (head)
        {
        case 0:
        {
                char pong = 1;
                send(fd, &pong, 1, 0);
                head = -1;
                return n;
        }
        case 1:
        {
                head = -1;
                return n;
        }
        }

        buf.insert(buf.end(), temp, temp + n);
        if (expectedSize == -1)
        {
                if (buf.size() < 3)
                {
                        return n;
                }

                expectedSize = readUint16FromBuffer(buf) + TIME_STAMP_BYTES;
                std::vector<char> size = intToBigEndian<std::uint16_t>(expectedSize);
                std::copy(size.begin(), size.end(), buf.begin() + 1);
        }

        std::vector<char> timeStamp = intToBigEndian<std::int64_t>(duration_cast<microseconds>(system_clock::now().time_since_epoch()).count());
        buf.insert(buf.begin() + 3, timeStamp.begin(), timeStamp.end());

        if (buf.size() < expectedSize)
                return n;

        switch (head)
        {
        case 2:
        {
                for (Client &other : networkingManager->clientsConnected)
                {
                        if (other.fd != fd)
                        {
                                networkingManager->safe_send(other, buf);
                        }
                }
        }
        }
        buf.clear();
        expectedSize = -1;
        head = -1;
        return n;
}