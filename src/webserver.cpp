#include <ESPmDNS.h>
#include <WebServer.h>
#include <WiFiMulti.h>
#include <LittleFS.h>

#include <FS.h>
#include <LittleFS.h>

static WiFiMulti wifiMulti;
static WebServer *http_server;
static MDNSResponder *mdns_responder;

static void drawGraph(void);
static void handleRoot(void);
static void handleNotFound(void);

void webserver_loop(void) {
    if (http_server)
        http_server->handleClient();
}

String getContentType(String filename) {
    if (http_server->hasArg("download")) {
        return "application/octet-stream";
    } else if (filename.endsWith(".htm")) {
        return "text/html";
    } else if (filename.endsWith(".html")) {
        return "text/html";
    } else if (filename.endsWith(".css")) {
        return "text/css";
    } else if (filename.endsWith(".js")) {
        return "application/javascript";
    } else if (filename.endsWith(".png")) {
        return "image/png";
    } else if (filename.endsWith(".gif")) {
        return "image/gif";
    } else if (filename.endsWith(".jpg")) {
        return "image/jpeg";
    } else if (filename.endsWith(".ico")) {
        return "image/x-icon";
    } else if (filename.endsWith(".xml")) {
        return "text/xml";
    } else if (filename.endsWith(".pdf")) {
        return "application/x-pdf";
    } else if (filename.endsWith(".zip")) {
        return "application/x-zip";
    } else if (filename.endsWith(".gz")) {
        return "application/x-gzip";
    }
    return "text/plain";
}

bool exists(String path) {
    bool yes = false;
    File file = LittleFS.open(path, "r");
    if(!file.isDirectory()) {
        yes = true;
    }
    file.close();
    return yes;
}


bool handleFileRead(String path) {

    if (path.endsWith("/")) {
        path += "index.htm";
    }
    String contentType = getContentType(path);
    log_i("try %s", path.c_str());
    File file = LittleFS.open(path, "r");
    if(file.isDirectory()) {
        file.close();
        return false;
    }
    http_server->streamFile(file, contentType);
    file.close();
    return true;
}

void webserver_setup(void) {

    log_i("mounting LittleFS");
    // LittleFS.begin(false); // ) "/", 20);
LittleFS.begin(false,"/littlefs", 10);


    log_i("Connecting to WiFi");
    WiFi.mode(WIFI_STA);
    WiFi.enableProv(true);

    wifiMulti.addAP(WIFI_SSID, WIFI_PASSWORD);
#if defined(WIFI_SSID11)
    wifiMulti.addAP(WIFI_SSID1, WIFI_PASSWORD1);
#endif
#if defined(WIFI_SSID2)
    wifiMulti.addAP(WIFI_SSID2, WIFI_PASSWORD2);
#endif
#if defined(WIFI_SSID3)
    wifiMulti.addAP(WIFI_SSID3, WIFI_PASSWORD3);
#endif
#if defined(WIFI_SSID4)
    wifiMulti.addAP(WIFI_SSID4, WIFI_PASSWORD4);
#endif
    wifiMulti.run();
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
    }
    log_i("WiFi connected, IP: %s", WiFi.localIP().toString().c_str());


    http_server = new WebServer(HTTP_PORT);
    http_server->onNotFound([]() {
        if (!handleFileRead( // String("/littlefs") +
                    http_server->uri())) {
            http_server->send(404, "text/plain", "FileNotFound");
        }
    });
    http_server->begin();
    log_i("HTTP server started\n");

    mdns_responder = new MDNSResponder();
    if (mdns_responder->begin("broker")) {
        log_i("MDNS responder started");
    }
    mdns_responder->addService("mqtt", "tcp", MQTT_TCP);
    mdns_responder->addService("mqtt-ws", "tcp", MQTT_WS);
    mdns_responder->addService("http", "tcp", HTTP_PORT);
}

static void drawGraph(void) {


    String out = "";
    char temp[100];
    out += "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\" "
           "width=\"400\" height=\"150\">\n";
    out += "<rect width=\"400\" height=\"150\" fill=\"rgb(250, 230, 210)\" "
           "stroke-width=\"1\" stroke=\"rgb(0, 0, 0)\" />\n";
    out += "<g stroke=\"black\">\n";
    int y = rand() % 130;
    for (int x = 10; x < 390; x += 10) {
        int y2 = rand() % 130;
        sprintf(
            temp,
            "<line x1=\"%d\" y1=\"%d\" x2=\"%d\" y2=\"%d\" stroke-width=\"1\" />\n",
            x, 140 - y, x + 10, 140 - y2);
        out += temp;
        y = y2;
    }
    out += "</g>\n</svg>\n";

    http_server->send(200, "image/svg+xml", out);

}

// static void handleRoot(void) {

//     char temp[400];
//     int sec = millis() / 1000;
//     int min = sec / 60;
//     int hr = min / 60;

//     snprintf(temp, 400,
//              "<html>\
//   <head>\
//     <meta http-equiv='refresh' content='5'/>\
//     <title>ESP32 Demo</title>\
//     <style>\
//       body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000088; }\
//     </style>\
//   </head>\
//   <body>\
//     <h1>Hello from ESP32!</h1>\
//     <p>Uptime: %02d:%02d:%02d</p>\
//     <img src=\"/test.svg\" />\
//   </body>\
// </html>",

//              hr, min % 60, sec % 60);
//     http_server->send(200, "text/html", temp);
// }

static void handleNotFound(void) {
    String message = "File Not Found\n\n";
    message += "URI: ";
    message += http_server->uri();
    message += "\nMethod: ";
    message += (http_server->method() == HTTP_GET) ? "GET" : "POST";
    message += "\nArguments: ";
    message += http_server->args();
    message += "\n";

    for (uint8_t i = 0; i < http_server->args(); i++) {
        message += " " + http_server->argName(i) + ": " + http_server->arg(i) + "\n";
    }

    http_server->send(404, "text/plain", message);
}
