#include <stdint.h>
#include <vector>
#include "session.h"

class SessionList
{
private:
public:
        SessionList(sessionListVersionType version);
        sessionListVersionType version;
        std::vector<sessionIdType> sessions;
        ~SessionList();
};