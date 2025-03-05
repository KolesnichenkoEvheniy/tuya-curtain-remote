#ifndef TUYA_API_H
#define TUYA_API_H

#include <Arduino.h>
#include <WiFiClientSecure.h>

class TuyaAPI {
public:
    TuyaAPI(const char* clientId, const char* secret, const char* deviceId, const char* apiUrl);
    bool setBlindsPosition(int percentage);
    bool setBlindsPositionWithRetries(int percentage, int maxRetries = 3);

private:
    // Tuya API credentials
    const char* m_clientId;
    const char* m_secret;
    const char* m_deviceId;
    const char* m_apiUrl;

    // Token management
    String m_accessToken;             // Stores the current access token
    unsigned long m_tokenExpirationTime; // Stores the expiration time of the token in milliseconds
    const unsigned long TOKEN_VALIDITY_PERIOD = 2 * 60 * 60 * 1000; // 2 hours in milliseconds

    // Helper methods
    bool fetchAccessToken();          // Fetches a new access token from Tuya API
    bool isTokenValid();              // Checks if the current token is valid
    String getCurrentTimestamp();     // Gets the current timestamp in milliseconds
};

class TokenExpiredException : public std::exception {
public:
    const char* what() const throw() {
        return "Token has expired";
    }
};
    

#endif // TUYA_API_H
