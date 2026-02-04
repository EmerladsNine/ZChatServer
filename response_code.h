#include <stdint.h>

enum ResponseCode : uint8_t
{
        emailAccountCreated = 0,
        emailAccountEmailExistError = 1,
        emailAccountUsernameExistError = 2,
        emailAccountInvalidEmailLengthError = 3,
        emailAccountInvalidPasswordLengthError = 4,
        emailAccountInvalidUsernameLengthError = 5,
        emailAccountCreationFailureError = 6,
};
