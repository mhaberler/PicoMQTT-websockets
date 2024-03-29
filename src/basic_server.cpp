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

void webserver_setup(void);
void webserver_loop(void);

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
#ifdef ARDUINO_USB_CDC_ON_BOOT
    delay(3000);
#endif
    Serial.begin(115200);

    webserver_setup();
    mqtt.subscribe("#", [](const char * topic, const char * payload) {
        Serial.printf("Received message in topic '%s': %s\n", topic, payload);
    });
    mqtt.begin();
    xTaskCreate(wsProxyTask,"wsproxy", 16384, nullptr, 1, &wsProxyTask_handle);
}

void loop() {
    mqtt.loop();
    webserver_loop();
}
