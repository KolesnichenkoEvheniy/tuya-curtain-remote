#ifndef TUYA_API_H
#define TUYA_API_H

#include <Arduino.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <crypto.h>

class TuyaAPI {
public:
    TuyaAPI(const char* clientId, const char* secret, const char* deviceId, const char* apiUrl);
    bool fetchAccessToken();
    bool setBlindsPosition(int percentage);

private:
    const char* m_clientId;
    const char* m_secret;
    const char* m_deviceId;
    const char* m_apiUrl;
    String m_accessToken;

    String generateSignature(const String& payload, const String& timestamp, const String& token = "");
    String getCurrentTimestamp();
};

#endif
