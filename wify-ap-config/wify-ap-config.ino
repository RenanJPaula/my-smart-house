#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <FS.h>

const String AP_NETWORK_NAME = "Config";
IPAddress apIP(192, 168, 1, 1);
IPAddress apGateway(192, 168, 1, 1);
IPAddress apSubnet(255, 255, 0, 0);

const int RELAY_PIN = 5; // D1
const int CONFIG_PIN = 4; // D2
const int WIFI_CONNECTED_PIN = 2; // D4

const String NETWORK_CREDENTIALS_FILE = "network-credentials.txt";
const String APPLICATION_FILE = "network-credentials.txt";
const String NETWORK_CONFIG_PAGE = "/config-network.html";
const String APPLICATION_PAGE = "/application.html";
const String WEBMANIFEST_FILE = "/manifest.webmanifest";
const String LAUNCHERICON_48_FILE = "/launchericon-48-48.png";
const String LAUNCHERICON_72_FILE = "/launchericon-72-72.png";
const String LAUNCHERICON_96_FILE = "/launchericon-96-96.png";
const String LAUNCHERICON_144_FILE = "/launchericon-144-144.png";
const String LAUNCHERICON_192_FILE = "/launchericon-192-192.png";

int relayStatus = 0;

struct NetworkCredentials {
  String networkName;
  String password;
};

ESP8266WebServer server(80);

/* SETUP HARDWARE */
void setupConfigPin() {
  pinMode(CONFIG_PIN, INPUT);
}

void setupRelayPin() {
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);
}

void setupIndicatorsPins() {
  pinMode(WIFI_CONNECTED_PIN, OUTPUT);
  digitalWrite(WIFI_CONNECTED_PIN, LOW);
}

void setupHardware() {
  setupConfigPin();
  setupRelayPin();
  setupIndicatorsPins();
}

void setRelayOn() {
  digitalWrite(RELAY_PIN, HIGH);
  relayStatus = 1;
}

void setRelayOff() {
  digitalWrite(RELAY_PIN, LOW);
  relayStatus = 0;
}

/* BUSINESS FUNCTIONS */
boolean isConfigSetup() {
  return digitalRead(CONFIG_PIN) == 1;
}

/* NETWORK CONFIG */
void setupConfigAccessPoint() {
  WiFi.mode(WIFI_AP);
  Serial.println(WiFi.softAPConfig(apIP, apGateway, apSubnet) ? "Network Configured!" : "Config Network Failed!");
  Serial.println(WiFi.softAP(AP_NETWORK_NAME) ? "Access Point Ready!" : "Access Point Failed!");
  Serial.print("MAC Address: ");
  Serial.println(WiFi.macAddress());
  Serial.print("Config server IP: ");
  Serial.println(WiFi.softAPIP());
}

void setupNetwork() {
  struct NetworkCredentials credentials = readCredentials();

  Serial.print("Connection to: ");
  Serial.println(credentials.networkName);
  Serial.print("With password: ");
  Serial.println(credentials.password);

  digitalWrite(WIFI_CONNECTED_PIN, LOW);
  
  WiFi.begin(credentials.networkName, credentials.password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Waiting to connect...");
  }

  digitalWrite(WIFI_CONNECTED_PIN, HIGH);
  Serial.print("MAC Address: ");
  Serial.println(WiFi.macAddress());
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void autoReconnectNetwork () {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Reconnecting WIFI Network");
    setupNetwork();
    delay(1000);
  }
}

/* FILE SYSTEM STORAGE */
void setupFileSystem() {
  if (!SPIFFS.begin()) {
    Serial.println("Error mounting the file system");
    return;
  }
  Serial.println("File System Ready!");
}

void saveCredentials(String networkName, String password) {
  Serial.println("Setting a new network credentials");
  Serial.print("networkName: ");
  Serial.println(networkName);
  Serial.print("password: ");
  Serial.println(password);

  File file = SPIFFS.open(NETWORK_CREDENTIALS_FILE, "w");
  file.println(networkName);
  file.println(password);
  file.close();
}

struct NetworkCredentials readCredentials() {
  Serial.println("Reading network credentials");
  File file = SPIFFS.open(NETWORK_CREDENTIALS_FILE, "r");

  if (!file) {
    Serial.print(NETWORK_CREDENTIALS_FILE);
    Serial.println(" not found!");
  }

  int line = 0;
  struct NetworkCredentials credentials;

  while (file.available()) {
    if (line == 0) {
      credentials.networkName = file.readStringUntil('\n');
      credentials.networkName.trim();
    }

    if (line == 1) {
      credentials.password = file.readStringUntil('\n');
      credentials.password.trim();
    }

    line++;
  }

  file.close();
  return credentials;
}

/* HTTP SERVER */
void handleConfigPage() {
  File file = SPIFFS.open(NETWORK_CONFIG_PAGE, "r");
  server.streamFile(file, "text/html");
  file.close();
}

void handleSaveConfig() {
  String networkName = server.arg("networkName");
  String password = server.arg("password");
  saveCredentials(networkName, password);
  server.send(200);
}

void handleApplicationPage() {
  File file = SPIFFS.open(APPLICATION_PAGE, "r");
  server.streamFile(file, "text/html");
  file.close();
}

void handleWebmanifest() {
  File file = SPIFFS.open(WEBMANIFEST_FILE, "r");
  server.streamFile(file, "application/manifest+json");
  file.close();
}

void handleLaunchericon48() {
  File file = SPIFFS.open(LAUNCHERICON_48_FILE, "r");
  server.streamFile(file, "image/png");
  file.close();
}

void handleLaunchericon72() {
  File file = SPIFFS.open(LAUNCHERICON_72_FILE, "r");
  server.streamFile(file, "image/png");
  file.close();
}

void handleLaunchericon96() {
  File file = SPIFFS.open(LAUNCHERICON_96_FILE, "r");
  server.streamFile(file, "image/png");
  file.close();
}

void handleLaunchericon144() {
  File file = SPIFFS.open(LAUNCHERICON_144_FILE, "r");
  server.streamFile(file, "image/png");
  file.close();
}

void handleLaunchericon192() {
  File file = SPIFFS.open(LAUNCHERICON_192_FILE, "r");
  server.streamFile(file, "image/png");
  file.close();
}

void handleStatusRelay() {
  String response = "{ \"relayStatus\" : " + String(relayStatus) + " }";
  Serial.println("response:" + response);
  server.send(200, "text/json", response);
}

void handleChangeStatusRelay() {
  String relayStatus = server.arg("relayStatus");
  if (relayStatus == "0") {
    setRelayOff();
  } else {
    setRelayOn();
  }
  String response = "{ \"relayStatus\" : " + String(relayStatus) + " }";
  Serial.println("response:" + response);
  server.send(200, "text/json", response);
}

void setupConfigHttpServer() {
  server.on("/", HTTP_GET, handleConfigPage);
  server.on("/config", HTTP_POST, handleSaveConfig);
  server.begin();
  Serial.println("Config server listening on port 80");
}

void setupApplicationHttpServer() {
  // PWA APPLICATION
  server.on("/", HTTP_GET, handleApplicationPage);
  server.on("/manifest.webmanifest", HTTP_GET, handleWebmanifest);
  server.on("/launchericon-48-48.png", HTTP_GET, handleLaunchericon48);
  server.on("/launchericon-72-72.png", HTTP_GET, handleLaunchericon72);
  server.on("/launchericon-96-96.png", HTTP_GET, handleLaunchericon96);
  server.on("/launchericon-144-144.png", HTTP_GET, handleLaunchericon144);
  server.on("/launchericon-192-192.png", HTTP_GET, handleLaunchericon192);
    
  // API
  server.on("/relay", HTTP_GET, handleStatusRelay);
  server.on("/relay", HTTP_POST, handleChangeStatusRelay);
  
  server.begin();
  Serial.println("Config server listening on port 80");
}

/* MAIN */
void setup() {
  Serial.begin(115200);
  Serial.println();
  setupHardware();
  setupFileSystem();

  if (isConfigSetup()) {
    Serial.println("Config setup mode on");
    setupConfigAccessPoint();
    setupConfigHttpServer();
  } else {
    Serial.println("Operation mode on");
    setupNetwork();
    setupApplicationHttpServer();
  }
}

void loop() {
  server.handleClient();
  if (!isConfigSetup()) {
    autoReconnectNetwork();  
  }
}
