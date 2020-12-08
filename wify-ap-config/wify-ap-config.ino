#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <FS.h>

IPAddress apIP(192, 168, 1, 1);
IPAddress apGateway(192, 168, 1, 1);
IPAddress apSubnet(255, 255, 0, 0);

const String AP_NETWORK_NAME = "Config";
const int CONFIG_PIN = 16;

ESP8266WebServer server(80);

void setupConfigPin() {
  pinMode(CONFIG_PIN, INPUT);
}

boolean isConfigSetup() {
  return digitalRead(CONFIG_PIN) == 0;
}

void setupConfigAccessPoint() {
  WiFi.mode(WIFI_AP);
  Serial.println(WiFi.softAPConfig(apIP, apGateway, apSubnet) ? "Network Configured!" : "Config Network Failed!");
  Serial.println(WiFi.softAP(AP_NETWORK_NAME) ? "Access Point Ready!" : "Access Point Failed!");
  Serial.print("MAC Address: ");
  Serial.println(WiFi.macAddress());
  Serial.print("Config server IP: ");
  Serial.println(WiFi.softAPIP());
}

void setupFileSystem() {
  if (!SPIFFS.begin()) {
    Serial.println("Error mounting the file system");
    return;
  }
  Serial.println("File System Ready!");
}

void handleRoot() {
  File file = SPIFFS.open("/config-network.html", "r");
  server.streamFile(file, "text/html");
  file.close();
}

void handleConfig() {
  Serial.print("networkName: ");
  Serial.println(server.arg("networkName"));
  Serial.print("password: ");
  Serial.println(server.arg("password"));
  server.send(200);
}

void setupConfigHttpServer() {
  server.on("/", HTTP_GET, handleRoot);
  server.on("/config", HTTP_POST, handleConfig);
  server.begin();
  Serial.println("Config server listening on port 80");
}

void setup() {
  Serial.begin(115200);
  Serial.println();
  setupConfigPin();

  if (isConfigSetup()) {
    Serial.println("Config setup mode on");
    setupFileSystem();
    setupConfigAccessPoint();
    setupConfigHttpServer();
  }
}

void loop() {
  server.handleClient();
}
