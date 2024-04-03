#include "WebSocketsServer.h"
#include <WiFi.h>

#define TOWS_SIZE 1536


void _proxyWebSocketEvent(WebSocketsServerCore *server, uint8_t num, WStype_t type, uint8_t * payload, size_t length);
bool validateHttpHeader(String headerName, String headerValue);

class ProxyWebSocketsServer : public WebSocketsServer {

    friend void _proxyWebSocketEvent(WebSocketsServerCore *server, uint8_t num, WStype_t type, uint8_t * payload, size_t length);

  public:

    ProxyWebSocketsServer(uint16_t port,
                          const String &destHost,
                          uint16_t destPort,
                          int32_t timeout_ms = 500,
                          const String & origin = "",
                          const String & protocol = "mqtt") :
        _destHost(destHost),
        _destPort(destPort),
        _timeout_ms(timeout_ms),
        WebSocketsServer(port, origin, protocol) {
        _buffer = (uint8_t *)malloc(TOWS_SIZE);
        onEvent(_proxyWebSocketEvent);
        begin();
    }

    ~ProxyWebSocketsServer() {
        free(_buffer);
    }

    void loop(void) {
        WebSocketsServer::loop();
        _loop();
    }

  private:
    void _loop(void);
    String _destHost;
    uint16_t _destPort;
    int32_t _timeout_ms;
    uint8_t *_buffer;
    WiFiClient *_dest[WEBSOCKETS_SERVER_CLIENT_MAX] = {nullptr};
};

