#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ArduinoJson.h>
#include <WiFiClientSecure.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

#define BTN_PIN D5
#define BUZZER D6

int f = 0;
int prevF = -1;  // ค่าก่อนหน้า
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

const char* ssid = "TAT";
const char* password = "TATAI2025NokSong";

const char* baseUrl = "http://ngsgame.thddns.net:5550";
const char* baseUrl1 = "http://ngsgame.thddns.net:5555";

void setup() {
  Wire.begin(D2, D1);
  pinMode(BTN_PIN, INPUT_PULLUP);
  pinMode(BUZZER, OUTPUT);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    while (true); // ถ้าไม่เจอจอ
  }

  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.println("Wifi");
  display.display();

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }

  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.println("OK");
  display.display();
}
String callAPI_JSON_POST1(String path, String body, String action) {
  WiFiClient client;
  HTTPClient http;

  String url = String(baseUrl1) + path;
  http.begin(client, url);
  http.addHeader("Content-Type", "application/json"); // บอก server ว่าส่ง JSON

  int httpCode = http.POST(body); // ส่ง body ไปเป็น POST
  String payload;
  if (httpCode > 0) {
    payload = http.getString();

    // แปลง JSON
    StaticJsonDocument<500> doc;
    DeserializationError error = deserializeJson(doc, payload);
    if (!error) {
      if (action == "result") {
        if (doc.containsKey("result")) {
          payload = doc["result"].as<String>();
          // ===== ตัวอย่างเพิ่ม key อื่น =====
          // if (doc.containsKey("success")) {
          //     bool ok = doc["success"];

          // }
          //
          // if (doc.containsKey("time")) {
          //     String t = doc["time"].as<String>();

          // }
          // ===============================
        }
      } else if (doc.containsKey("uuid")) {
        payload = doc["uuid"].as<String>();
        if (action == "start1") {
          display.clearDisplay();
          display.setTextSize(2);
          display.setTextColor(WHITE);
          display.setCursor(0,0);
          display.println("1 Start");
          display.display();
        } else if (action == "stop1") {
          display.clearDisplay();
          display.setTextSize(2);
          display.setTextColor(WHITE);
          display.setCursor(0,0);
          display.println("1 Stop");
          display.display();
        }
        // ===== ตัวอย่างเพิ่ม key อื่น =====
        // if (doc.containsKey("success")) {
        //     bool ok = doc["success"];

        // }
        //
        // if (doc.containsKey("time")) {
        //     String t = doc["time"].as<String>();

        // }
        // ===============================
      }
    } else {
      payload = "JSON parse error";
    }
  } else {
    payload = "HTTP error";
  }

  http.end();
  return payload;
}
String callAPI_JSON_POST(String path, String body) {
  WiFiClient client;
  HTTPClient http;

  String url = String(baseUrl) + path;
  http.begin(client, url);
  http.addHeader("Content-Type", "application/json"); // บอก server ว่าส่ง JSON

  int httpCode = http.POST(body); // ส่ง body ไปเป็น POST
  String payload;

  if (httpCode > 0) {
    payload = http.getString();

    // แปลง JSON
    StaticJsonDocument<200> doc;
    DeserializationError error = deserializeJson(doc, payload);
    if (!error) {
      if (doc.containsKey("message")) {
        payload = doc["message"].as<String>();
        
        // ===== ตัวอย่างเพิ่ม key อื่น =====
        // if (doc.containsKey("success")) {
        //     bool ok = doc["success"];

        // }
        //
        // if (doc.containsKey("time")) {
        //     String t = doc["time"].as<String>();

        // }
        // ===============================
      }
    } else {
      payload = "JSON parse error";
    }
  } else {
    payload = "HTTP error";
  }

  http.end();
  return payload;
}


void loop() {
  int val = analogRead(A0); // อ่านทุกครั้งที่ loop วิ่ง
  if (val >= 300) {
    f = 3;
  } else if (val >= 200) {
    f = 2;
  } else if (val >= 100) {
    f = 1;
  } else {
    f = 0;
  }

  // วาด OLED เฉพาะเมื่อ f เปลี่ยน
  if (f != prevF) {
    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(WHITE);
    display.setCursor(0,0);

    if (f == 0) display.println("WOL");
    else if (f == 1) display.println("Start");
    else if (f == 3) display.println("Status");
    else if (f == 2) display.println("Stop");

    display.display();

    prevF = f; // อัปเดตค่าเดิม
  }
  if (digitalRead(BTN_PIN) == LOW) {
    if (f == 0) {
      display.clearDisplay();
      display.setTextSize(2);
      display.setTextColor(WHITE);
      display.setCursor(0,0);
      display.println("WOL PC .");
      display.display();
      String jsonBody = "{\"key\":\"wol\",\"api_key\":\"TATAPI\"}";
      String res = callAPI_JSON_POST("/api/api/api/2025/noksong/nok/nokcraft/pnc/kuy56/song/tat/game/wol/api/api/2024/wol", jsonBody);
      display.clearDisplay();
      display.setTextSize(2);
      display.setTextColor(WHITE);
      display.setCursor(0,0);
      display.println(res);
      display.display();
      delay(2500); // กันเด้งเบื้องต้น
      radio();
      prevF = -1;
    } else if (f == 1) {
      display.clearDisplay();
      display.setTextSize(2);
      display.setTextColor(WHITE);
      display.setCursor(0,0);
      display.println("Start Start");
      display.display();
      String jsonBody = "{\"action\":\"start\",\"uuid\":\"1a814ab83b0343d6b9b8ee8b8d37f8be\",\"api_key\":\"1234567890abcdef\"}";
      String res = callAPI_JSON_POST1("/api/api/api/2025/noksong/nok/nokcraft/pnc/kuy56/song/tat/game/nok", jsonBody,"start1");

      delay(300); // กันเด้งเบื้องต้น
      radio();
      prevF = -1;
    } else if (f == 2) {
      display.clearDisplay();
      display.setTextSize(2);
      display.setTextColor(WHITE);
      display.setCursor(0,0);
      display.println("Start Stop");
      display.display();
      String jsonBody = "{\"action\":\"stop\",\"uuid\":\"1a814ab83b0343d6b9b8ee8b8d37f8be\",\"api_key\":\"1234567890abcdef\"}";
      String res = callAPI_JSON_POST1("/api/api/api/2025/noksong/nok/nokcraft/pnc/kuy56/song/tat/game/nok", jsonBody,"stop1");


      delay(300); // กันเด้งเบื้องต้น
      radio();
      prevF = -1;
    } else if (f == 3) {
      display.clearDisplay();
      display.setTextSize(2);
      display.setTextColor(WHITE);
      display.setCursor(0,0);
      display.println("Start Status");
      display.display();
      String jsonBody = "{\"action\":\"status\",\"uuid\":\"1a814ab83b0343d6b9b8ee8b8d37f8be\",\"api_key\":\"1234567890abcdef\"}";
      String res = callAPI_JSON_POST1("/api/api/api/2025/noksong/nok/nokcraft/pnc/kuy56/song/tat/game/nok", jsonBody,"result");
      if (res == "0") {
        display.clearDisplay();
        display.setTextSize(2);
        display.setTextColor(WHITE);
        display.setCursor(0,0);
        display.println("Stop");
        display.display();
      } else if (res == "3") {
        display.clearDisplay();
        display.setTextSize(2);
        display.setTextColor(WHITE);
        display.setCursor(0,0);
        display.println("Run");
        display.display();
      }
      
      delay(1500);
      radio();
      prevF = -1;
      
    } else {

      delay(300); // กันเด้งเบื้องต้น
    }
    
  }

}

void radio() {
  if (f == 0) {
    tone(BUZZER, 262, 200); delay(250); // Do
    tone(BUZZER, 294, 200); delay(250); // Re
  } else if (f == 1) {
    tone(BUZZER, 330, 200); delay(250); // Mi
    tone(BUZZER, 392, 00); delay(500); // Sol
    noTone(BUZZER);
    delay(10000);
    tone(BUZZER, 294, 200); delay(250); // Re
    tone(BUZZER, 330, 200); delay(250); // Mi
    tone(BUZZER, 294, 200); delay(250); // Re
    tone(BUZZER, 330, 200); delay(250); // Mi
  } else {
    tone(BUZZER, 294, 200); delay(250); // Re
    tone(BUZZER, 330, 200); delay(250); // Mi
  }

  noTone(BUZZER);
  delay(300);
}