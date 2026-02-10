#include <stdint.h>

enum UnitType : uint8_t
{
        ping = 0,
        pong = 1,
        normalMessage = 2,
        emailSignIn = 3,
        emailSignUp = 4,
        responseCode = 5,
        googleSignIn = 6,
        googleSignUp = 7
};
