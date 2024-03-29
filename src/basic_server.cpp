#include <PicoMQTT.h>

#if __has_include("config.h")
    #include "config.h"
#endif

#include <ProxyWebSocketsServer.h>

TaskHandle_t wsProxyTask_handle;

PicoMQTT::Server mqtt(1883);
#define INTERVAL 2000

#define PROXY_DEST "127.0.0.1"
ProxyWebSocketsServer *webSocket;

void wsProxyTask(void* pvParameters) {
    Serial.printf("wsProxyTask\n");
    webSocket = new ProxyWebSocketsServer(8883, PROXY_DEST, 1883, 2000);

    for (;;) {
        webSocket->loop();
        yield();
        delay(1);
    }
}

void setup() {
    delay(3000);
    Serial.begin(115200);

    Serial.printf("Connecting to WiFi %s\n", WIFI_SSID);
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
    xTaskCreate(wsProxyTask,"wsproxy", 16384, nullptr, 1, &wsProxyTask_handle);
}

void loop() {
    mqtt.loop();
}
