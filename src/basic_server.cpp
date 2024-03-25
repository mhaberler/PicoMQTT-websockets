#include <PicoMQTT.h>

#if __has_include("config.h")
    #include "config.h"
#endif

#include <ProxyWebSocketsServer.h>

PicoMQTT::Server mqtt(1883);


ProxyWebSocketsServer *webSocket;

void setup() {
    delay(3000);
    Serial.begin(115200);

    Serial.printf("broker - Connecting to WiFi %s\n", WIFI_SSID);
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
    }
    Serial.printf("WiFi connected, IP: %s\n", WiFi.localIP().toString().c_str());
    mqtt.subscribe("#", [](const char * topic, const char * payload) {
        Serial.printf("Received message in topic '%s': %s\n", topic, payload);
    });
    mqtt.begin();
    webSocket = new ProxyWebSocketsServer(8883, PROXY_DEST, 1883, 2000);
}

void loop() {
    mqtt.loop();
    if (webSocket)
        webSocket->loop();
    yield();
}
