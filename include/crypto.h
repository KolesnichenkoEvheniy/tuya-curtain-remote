#ifndef CRYPTO_H
#define CRYPTO_H

#include <string>
#include <vector>
#include <mbedtls/md.h>

class CryptoJS {
public:
    static std::string sha256(const std::string& input) {
        std::vector<unsigned char> hash(32);
        mbedtls_md_context_t ctx;
        mbedtls_md_type_t md_type = MBEDTLS_MD_SHA256;
        
        mbedtls_md_init(&ctx);
        mbedtls_md_setup(&ctx, mbedtls_md_info_from_type(md_type), 0);
        mbedtls_md_starts(&ctx);
        mbedtls_md_update(&ctx, (const unsigned char*)input.c_str(), input.length());
        mbedtls_md_finish(&ctx, hash.data());
        mbedtls_md_free(&ctx);
        
        return bytesToHex(hash);
    }

    static std::string hmacSha256(const std::string& key, const std::string& message) {
        std::vector<unsigned char> hmac(32);
        mbedtls_md_context_t ctx;
        mbedtls_md_type_t md_type = MBEDTLS_MD_SHA256;
        
        mbedtls_md_init(&ctx);
        mbedtls_md_setup(&ctx, mbedtls_md_info_from_type(md_type), 1);
        mbedtls_md_hmac_starts(&ctx, (const unsigned char*)key.c_str(), key.length());
        mbedtls_md_hmac_update(&ctx, (const unsigned char*)message.c_str(), message.length());
        mbedtls_md_hmac_finish(&ctx, hmac.data());
        mbedtls_md_free(&ctx);
        
        return bytesToHex(hmac);
    }

private:
    static std::string bytesToHex(const std::vector<unsigned char>& bytes) {
        std::string hex;
        for (unsigned char byte : bytes) {
            char buf[3];
            snprintf(buf, sizeof(buf), "%02x", byte);
            hex += buf;
        }
        return hex;
    }
};

#endif // CRYPTO_H
