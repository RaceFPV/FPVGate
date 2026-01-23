#include <WiFi.h>
#include <WebServer.h>

const char* ssid = "TestAP_SuperMini";
const char* password = "test1234";

WebServer server(80);

void handleRoot() {
  String html = "<html><head><title>ESP32-S3 Test</title></head>";
  html += "<body><h1>WiFi AP Test Success!</h1>";
  html += "<p>If you can see this page, WiFi AP and web server are working.</p>";
  html += "<p>Board: ESP32-S3 Super Mini</p>";
  html += "<p>Free Heap: " + String(ESP.getFreeHeap()) + " bytes</p>";
  html += "<p>Chip Model: " + String(ESP.getChipModel()) + "</p>";
  html += "<p>Flash Size: " + String(ESP.getFlashChipSize() / (1024 * 1024)) + " MB</p>";
  html += "</body></html>";
  server.send(200, "text/html", html);
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("\n=== ESP32-S3 Super Mini WiFi Test ===");
  Serial.println("Starting WiFi AP...");
  
  // Configure WiFi AP
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid, password);
  
  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);
  Serial.print("SSID: ");
  Serial.println(ssid);
  Serial.print("Password: ");
  Serial.println(password);
  
  // Setup web server
  server.on("/", handleRoot);
  server.begin();
  Serial.println("HTTP server started");
  Serial.println("Connect to WiFi and open http://192.168.4.1");
  Serial.println("=====================================\n");
}

void loop() {
  server.handleClient();
  
  // Print connection status every 5 seconds
  static unsigned long lastPrint = 0;
  if (millis() - lastPrint > 5000) {
    Serial.print("Connected clients: ");
    Serial.println(WiFi.softAPgetStationNum());
    lastPrint = millis();
  }
}
