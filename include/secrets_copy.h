#include <pgmspace.h>

#define SECRET
#define THINGNAME "ESP32-BLE"

const char WIFI_SSID[]        = "XXXXXXX";
const char WIFI_PASSWORD[]    = "XXXXXXX";
const char AWS_IOT_ENDPOINT[] = "XXXXXXX.iot.us-east-1.amazonaws.com";

// Amazon Root CA 1
static const char AWS_CERT_CA[] PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
XXXXXXX
-----END CERTIFICATE-----
)EOF";

// Device Certificate
static const char AWS_CERT_CRT[] PROGMEM = R"KEY(
-----BEGIN CERTIFICATE-----
XXXXXXX
-----END CERTIFICATE-----
)KEY";

// Device Private Key
static const char AWS_CERT_PRIVATE[] PROGMEM = R"KEY(
-----BEGIN RSA PRIVATE KEY-----
XXXXXXX
-----END RSA PRIVATE KEY-----
)KEY";
