#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <FS.h>

IPAddress apIP(192, 168, 1, 1);
IPAddress apGateway(192, 168, 1, 1);
IPAddress apSubnet(255, 255, 0, 0);

const String AP_NETWORK_NAME = "Config";
const int CONFIG_PIN = 16;
const int RELAY_PIN = 5;

const String NETWORK_CREDENTIALS_FILE = "network-credentials.txt";

struct networkCredentials {
  String networkName;
  String password;
};

ESP8266WebServer server(80);

void setupConfigPin() {
  pinMode(CONFIG_PIN, INPUT);
}

void setupRelayPin() {
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);
}

boolean isConfigSetup() {
  return digitalRead(CONFIG_PIN) == 1;
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

void saveCredentials(String networkName, String password) {
  File file = SPIFFS.open(NETWORK_CREDENTIALS_FILE, "w");
  file.println(networkName);
  file.println(password);
  file.close();
}

struct networkCredentials readCredentials() {
  Serial.println("Reading network credentials");
  File file = SPIFFS.open(NETWORK_CREDENTIALS_FILE, "r");

  if (!file) {
    Serial.print(NETWORK_CREDENTIALS_FILE);
    Serial.println(" not found!");
  }

  int line = 0;
  struct networkCredentials credentials;

  while (file.available()) {
    if (line == 0) {
      credentials.networkName = file.readStringUntil('\n');
    }

    if (line == 1) {
      credentials.password  = file.readStringUntil('\n');
    }

    line++;
  }

  file.close();

  return credentials;
}

void handleSaveConfig() {
  String networkName = server.arg("networkName");
  String password = server.arg("password");

  Serial.println("Setting a new network credentials");
  Serial.print("networkName: ");
  Serial.println(networkName);
  Serial.print("password: ");
  Serial.println(password);

  saveCredentials(networkName, password);
  server.send(200);
}

void setupConfigHttpServer() {
  server.on("/", HTTP_GET, handleRoot);
  server.on("/config", HTTP_POST, handleSaveConfig);
  server.begin();
  Serial.println("Config server listening on port 80");
}

void setupHardware() {
  setupConfigPin();
  setupRelayPin();
}

void setup() {
  Serial.begin(115200);
  Serial.println();
  setupHardware();
  setupFileSystem();

  struct networkCredentials credentials = readCredentials();

  Serial.println(credentials.networkName);
  Serial.println(credentials.password);

  if (isConfigSetup()) {
    Serial.println("Config setup mode on");
    setupConfigAccessPoint();
    setupConfigHttpServer();
  } else {
    Serial.println("Operation mode on");

  }
}

void loop() {
  server.handleClient();
}
