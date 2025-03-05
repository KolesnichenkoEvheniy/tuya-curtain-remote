#include "TuyaAPI.h"
#include "crypto.h"
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <chrono>

TuyaAPI::TuyaAPI(const char* clientId, const char* secret, const char* deviceId, const char* apiUrl)
    : m_clientId(clientId), m_secret(secret), m_deviceId(deviceId), m_apiUrl(apiUrl), 
      m_tokenExpirationTime(0) {}

bool TuyaAPI::isTokenValid() {
    return !m_accessToken.isEmpty() && (millis() < m_tokenExpirationTime);
}

bool TuyaAPI::fetchAccessToken() {
    WiFiClientSecure client;
    client.setInsecure();

    HTTPClient http;
    String url = String(m_apiUrl) + "/v1.0/token?grant_type=1";
    
    String timestamp = getCurrentTimestamp();
    Serial.print("Using timestamp for token request: ");
    Serial.println(timestamp);

    String method = "GET";
    String bodyStr = "";
    String sha256Body = String(CryptoJS::sha256("").c_str());
    String headersStr = "";
    String urlStr = "/v1.0/token?grant_type=1";

    String signUrl = method + "\n" + sha256Body + "\n" + headersStr + "\n" + urlStr;

    String nonce = ""; // Nonce is empty for this request
    std::string stdSignStr = std::string((m_clientId + timestamp + nonce + signUrl).c_str());
    std::string stdSecret = std::string(m_secret);
    String signature = String(CryptoJS::hmacSha256(stdSecret, stdSignStr).c_str());
    signature.toUpperCase();

    http.begin(client, url);
    http.addHeader("client_id", m_clientId);
    http.addHeader("sign", signature);
    http.addHeader("t", timestamp);
    http.addHeader("sign_method", "HMAC-SHA256");
    http.addHeader("nonce", nonce);

    int httpCode = http.GET();
    String payload = http.getString();

    Serial.print("Token request HTTP Response code: ");
    Serial.println(httpCode);
    Serial.print("Response payload: ");
    Serial.println(payload);

    if (httpCode == 200) {
        DynamicJsonDocument doc(1024);
        deserializeJson(doc, payload);

        if (doc["result"]["access_token"]) {
            m_accessToken = doc["result"]["access_token"].as<String>();
            m_tokenExpirationTime = millis() + TOKEN_VALIDITY_PERIOD;
            http.end();
            return true;
        }
    }

    http.end();
    return false;
}

bool TuyaAPI::setBlindsPosition(int percentage) {
    if (!isTokenValid() && !fetchAccessToken()) {
        return false;
    }

    WiFiClientSecure client;
    client.setInsecure();

    HTTPClient http;
    String url = String(m_apiUrl) + "/v1.0/devices/" + m_deviceId + "/commands";

    String payload = "{\"commands\":[{\"code\":\"percent_control\",\"value\":" + String(percentage) + "}]}";

    String timestamp = getCurrentTimestamp();
    Serial.print("Using timestamp for setBlindsPosition: ");
    Serial.println(timestamp);

    String method = "POST";
    
	std::string stdPayload(payload.c_str(), payload.length());
	String sha256Payload = String(CryptoJS::sha256(stdPayload).c_str());
	String headersStr = "";
	String urlStr = "/v1.0/devices/" + String(m_deviceId) + "/commands";

	String signUrl = method + "\n" + sha256Payload + "\n" + headersStr + "\n" + urlStr;

	String nonce = ""; // Nonce is empty for this request
	std::string stdSignStr = std::string((m_clientId + m_accessToken + timestamp + nonce + signUrl).c_str());
	std::string stdSecret = std::string(m_secret);
	String signature = String(CryptoJS::hmacSha256(stdSecret, stdSignStr).c_str());
	signature.toUpperCase();

	http.begin(client, url);
	http.addHeader("Content-Type", "application/json");
	http.addHeader("client_id", m_clientId);
	http.addHeader("access_token", m_accessToken);
	http.addHeader("sign", signature);
	http.addHeader("t", timestamp);
	http.addHeader("sign_method", "HMAC-SHA256");

	int httpCode = http.POST(payload);
	String response = http.getString();

	Serial.print("HTTP Response code: ");
	Serial.println(httpCode);
	Serial.print("Response: ");
	Serial.println(response);

	if (httpCode == 200) {
		DynamicJsonDocument doc(1024);
		deserializeJson(doc, response);

		if (doc["success"].as<bool>()) {
			http.end();
			return true;
		} else if (doc["code"].as<int>() == 1010 || 
				   (doc["msg"].as<String>() == "token invalid" && !doc["success"].as<bool>())) {
			Serial.println("Token invalid. Fetching new token and retrying...");
			throw TokenExpiredException(); // Throw an exception for the expired token
		}
	}

	http.end();
	return false;
}

bool TuyaAPI::setBlindsPositionWithRetries(int percentage, int maxRetries) {
	for (int attempt = 0; attempt < maxRetries; ++attempt) {
		try {
			return setBlindsPosition(percentage); // Try to set the blinds position
		} catch (const TokenExpiredException&) {
			Serial.println("Caught TokenExpiredException. Fetching new token...");
			if (!fetchAccessToken()) {
				break; // Exit if unable to fetch a new token
			}
		}
	}
	return false; // Return false if all attempts fail
}

String TuyaAPI::getCurrentTimestamp() {
	return String(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count());
}
