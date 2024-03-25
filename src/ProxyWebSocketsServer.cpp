#include <ProxyWebSocketsServer.h>

void ProxyWebSocketsServer::_loop(void) {
    for (int i = 0; i < WEBSOCKETS_SERVER_CLIENT_MAX; i++) {
        WiFiClient *d = _dest[i];
        if (d && d->connected()) {
            while (d->available()) {
                uint8_t buffer[TOWS_SIZE];
                int res = d->read(buffer, sizeof(buffer));
                if ((res > 0) && (_clients[i].status == WSC_CONNECTED)) {
                    Serial.printf("[%u]-----> toClient %u\n", i, res);
                    this->sendBIN(i, buffer, res);
                }
            }
        }
    }
}


void _proxyWebSocketEvent(WebSocketsServerCore *server, uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
    ProxyWebSocketsServer *pws = (ProxyWebSocketsServer*)server;
    WSclient_t *wsc = &pws->_clients[num];
    WiFiClient *wc = pws->_dest[num];

    switch(type) {
        case WStype_DISCONNECTED:
            Serial.printf("[%u] Disconnected\n", num);
            if (wc && wc->connected()) {
                wc->stop();
                delete wc;
                pws->_dest[num] = nullptr;
            }
            break;
        case WStype_CONNECTED: {

                IPAddress ip = wsc->tcp->remoteIP() ;
                uint16_t port = wsc->tcp->remotePort() ;
                Serial.printf("[%u] Connected from %d.%d.%d.%d port %u url: %s len=%u\n", num, ip[0], ip[1], ip[2], ip[3], port, payload, length);

                pws->_dest[num] = new WiFiClient;
                WiFiClient *cp =  pws->_dest[num];

                if (!cp->connect(pws->_destHost.c_str(), pws->_destPort, pws->_timeout_ms)) {
                    delete pws->_dest[num] ;
                    pws->_dest[num] = nullptr;
                    Serial.printf("[%u] proxy connect failed\n", num);
                    break;
                }
                Serial.printf("[%u] proxy connect success connected=%d\n", num, cp->connected());
            }
            break;
        case WStype_TEXT:
            Serial.printf("[%u] get Text: %s\n", num, payload);
            if (wc && wc->connected()) {
                wc->write(payload, length);
            }
            break;
        case WStype_BIN:
            Serial.printf("[%u] get binary length: %u wc->connected=%d payload=%s\n", num, length, wc && wc->connected(), payload);
            if (wc && wc->connected()) {
                wc->write(payload, length);
            }
            break;
        case WStype_ERROR:
        case WStype_FRAGMENT_TEXT_START:
        case WStype_FRAGMENT_BIN_START:
        case WStype_FRAGMENT:
        case WStype_FRAGMENT_FIN:
            Serial.printf("wsmessage %u\n", type);
            break;
    }
}