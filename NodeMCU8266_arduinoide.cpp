#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <math.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET     -1
#define SCREEN_ADDRESS 0x3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define LED_PIN 2 // Wemos D1 mini: D4 (GPIO2), NodeMCU: D4 (GPIO2)
bool ledState = false;

// Ustaw dane swojej sieci WiFi
const char* ssid = "UNITRONIX";
const char* password = "Unitronix221!";

// Adres serwera Flask
const char* serverUrl = "http://77.87.3.228:8080/last_command";

struct ServerCommandResult {
    String command;
    String result;
    bool success;
};

// Funkcja pobierająca dane z serwera Flask
ServerCommandResult readServerCommand() {
    ServerCommandResult result = {"", "", true};
    if (WiFi.status() == WL_CONNECTED) {
        WiFiClient client;
        HTTPClient http;
        http.begin(client, serverUrl);
        int httpCode = http.GET();
        if (httpCode == HTTP_CODE_OK) {
            String payload = http.getString();
            int c1 = payload.indexOf("\"command\":\"") + 11;
            int c2 = payload.indexOf("\"", c1);
            int r1 = payload.indexOf("\"result\":\"") + 10;
            int r2 = payload.indexOf("\"", r1);
            int s1 = payload.indexOf("\"success\":") + 10;
            int s2 = payload.indexOf("}", s1);
            if (c1 > 10 && c2 > c1) result.command = payload.substring(c1, c2);
            if (r1 > 9 && r2 > r1) result.result = payload.substring(r1, r2);
            if (s1 > 9 && s2 > s1) result.success = payload.substring(s1, s2).indexOf("true") >= 0;
        }
        http.end();
    }
    return result;
}

// Plansza powitalna z informacją o połączeniu z WiFi i serwerem
void showConnectionStatus(bool wifiOk, bool serverOk) {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println("Status polaczenia:");
    display.setCursor(0, 16);
    display.print("WiFi: ");
    display.println(wifiOk ? "OK" : "BLAD");
    display.setCursor(0, 28);
    display.print("Serwer: ");
    display.println(serverOk ? "OK" : "BLAD");
    display.setCursor(0, 44);
    display.print("IP: ");
    display.println(WiFi.localIP());
    display.display();
}

// Wyświetlanie komendy i wyniku na bieżąco (bez animacji)
void showCommandAndResult(const ServerCommandResult& cmdResult) {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);

    display.setCursor(0, 0);
    display.print("Komenda:");
    display.setCursor(0, 12);
    display.print(cmdResult.command);

    display.setCursor(0, 32);
    display.print("Wynik:");
    display.setCursor(0, 44);
    display.print(cmdResult.result);

    display.display();
}

void setup() {
    Wire.begin(14, 12); // Jeśli używasz innych pinów, zmień na swoje
    pinMode(LED_PIN, OUTPUT);
    display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS);
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);

    display.setCursor(0, 0);
    display.print("Laczenie z WiFi...");
    display.display();

    WiFi.begin(ssid, password);
    int tries = 0;
    while (WiFi.status() != WL_CONNECTED && tries < 40) {
        delay(300);
        display.print(".");
        display.display();
        tries++;
    }

    bool wifiOk = WiFi.status() == WL_CONNECTED;
    bool serverOk = false;

    if (wifiOk) {
        // Sprawdź połączenie z serwerem
        ServerCommandResult test = readServerCommand();
        serverOk = (test.command.length() > 0 || test.result.length() > 0);
    }

    showConnectionStatus(wifiOk, serverOk);
    delay(2500);
}

void loop() {
    ServerCommandResult cmd = readServerCommand();
    showCommandAndResult(cmd);
    delay(1500); // Odświeżaj co 1,5 sekundy
}
