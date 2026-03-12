#include <stdint.h>

enum UnitType : uint8_t
{
        ping = 0,
        pong = 1,
        normalMessage = 2,
        emailSignIn = 3,
        emailSignUp = 4,
        authResponseCode = 5,
        googleSignIn = 6,
        googleSignUp = 7,
        searchWithUsername = 8,
        searchWithId = 9,
        searchResponseCode = 10,
        normalMessageResponseCode = 11,
        notAuthenticated = 12
};
