#include <PicoMQTT.h>

#if __has_include("config.h")
    #include "config.h"
#endif

#include <ProxyWebSocketsServer.h>

TaskHandle_t wsProxyTask_handle;

PicoMQTT::Server mqtt(1883);
#define INTERVAL 1000

#define PROXY_DEST "127.0.0.1"
ProxyWebSocketsServer *webSocket;

void webserver_setup(void);
void webserver_loop(void);

void wsProxyTask(void* pvParameters) {
   log_i("wsProxyTask");
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
        log_i("Received message in topic '%s': %s", topic, payload);
    });
    mqtt.begin();
    xTaskCreate(wsProxyTask,"wsproxy", 16384, nullptr, 1, &wsProxyTask_handle);
}

uint32_t last;
void loop() {
    mqtt.loop();
    webserver_loop();
    // periodically publish CPU temperature
    if (millis() - last > INTERVAL) {
        float t = temperatureRead();
        String topic = "picomqtt/esp-" + WiFi.macAddress();
        String message = String(t);
        log_d("Publishing message in topic '%s': %s", topic.c_str(), message.c_str());
        mqtt.publish(topic, message);
        last = millis();
    }
}
