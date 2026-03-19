#pragma once
#include <stdint.h>

enum AuthResponseCode : uint8_t
{
        emailAccountCreated = 0,
        emailAccountEmailExistError = 1,
        emailAccountUsernameExistError = 2,
        emailAccountInvalidEmailError = 3,
        emailAccountInvalidPasswordLengthError = 4,
        emailAccountInvalidUsernameError = 5,
        emailAccountCreationFailureError = 6,
        emailSignInEmailNotExistError = 7,
        emailSignInFailureError = 8,
        emailSignInPasswordIncorrectError = 9,
        emailSignInDone = 10,
        googleAuthInvalidToken = 11,
        googleAuthSuccessful = 12,
        googleAuthFailed = 13,
        googleAuthRequireSignUp = 14,
        googleSignUpInvalidUsernameError = 15,
        googleSignUpUsernameExistError = 16,
        googleSignUpGoogleIdExistError = 17
};
