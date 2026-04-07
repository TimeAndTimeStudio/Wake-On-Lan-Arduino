#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <EEPROM.h>
#include <WiFiUdp.h>
#include <ArduinoJson.h>

#define EEPROM_SIZE      512
#define SSID_ADDR        0
#define PASSWORD_ADDR    64
#define API_KEY_ADDR     128
#define MAC_ADDR         192  // ที่อยู่ MAC ใน EEPROM (เก็บข้อมูล 6 ไบต์)

ESP8266WebServer server(5550);
WiFiUDP udp;  // สำหรับส่ง WOL Packet

// ======================= ฟังก์ชั่นช่วยเหลือ =======================

// อ่านข้อมูลจาก EEPROM
String readEEPROM(int startAddress, int length) {
    String result = "";
    for (int i = startAddress; i < startAddress + length; i++) {
        char c = EEPROM.read(i);
        if (c == '\0') break;  // หยุดอ่านเมื่อเจอ null terminator
        result += c;
    }
    return result;
}
// ======================= ฟังก์ชั่นช่วยเหลือ =======================

// อ่านและแสดงข้อมูลทั้งหมดจาก EEPROM
void printEEPROMData() {
    Serial.println("===== EEPROM DATA =====");

    String ssid = readEEPROM(SSID_ADDR, 32);
    String password = readEEPROM(PASSWORD_ADDR, 32);
    String apiKey = readEEPROM(API_KEY_ADDR, 64);

    byte mac[6];
    for (int i = 0; i < 6; i++) {
        mac[i] = EEPROM.read(MAC_ADDR + i);
    }

    Serial.println("SSID: " + ssid);
    Serial.println("Password: " + password);
    Serial.println("API Key: " + apiKey);

    char macStr[18];
    sprintf(macStr, "%02X:%02X:%02X:%02X:%02X:%02X",
            mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    Serial.print("MAC: ");
    Serial.println(macStr);

    Serial.println("======================");
}

// เขียนข้อมูลลง EEPROM
void writeEEPROM(int startAddress, String data) {
    for (int i = 0; i < data.length(); i++) {
        EEPROM.write(startAddress + i, data[i]);
    }
    EEPROM.write(startAddress + data.length(), '\0'); // null terminator
    EEPROM.commit();
}

// ล้างข้อมูลทั้งหมดใน EEPROM
void clearEEPROM() {
    for (int i = 0; i < EEPROM_SIZE; i++) {
        EEPROM.write(i, 0);
    }
    EEPROM.commit();
    Serial.println("EEPROM cleared successfully!");
}

// --- ฟังก์ชั่นอ่าน MAC จาก EEPROM เป็น string ---
String readMACfromEEPROM() {
    char macStr[18]; // "AA:BB:CC:DD:EE:FF"
    byte macBytes[6];
    for (int i = 0; i < 6; i++) {
        macBytes[i] = EEPROM.read(MAC_ADDR + i);
    }
    sprintf(macStr, "%02X:%02X:%02X:%02X:%02X:%02X",
            macBytes[0], macBytes[1], macBytes[2],
            macBytes[3], macBytes[4], macBytes[5]);
    return String(macStr);
}

// --- แปลง MAC จาก string "AA:BB:CC:DD:EE:FF" เป็น byte[6] ---
bool parseMAC(String macStr, byte mac[6]) {
    if (macStr.length() != 17) return false;
    for (int i = 0, j = 0; i < 6; i++, j += 3) {
        mac[i] = strtol(macStr.substring(j, j + 2).c_str(), NULL, 16);
    }
    return true;
}

// --- ส่ง WOL จาก MAC EEPROM ---
void sendWOLfromEEPROM() {
    String macString = readMACfromEEPROM();
    byte mac[6];
    if (!parseMAC(macString, mac)) {
        Serial.println("Invalid MAC in EEPROM!");
        return;
    }

    byte packet[102];
    memset(packet, 0xFF, 6);  // broadcast
    for (int i = 0; i < 16; i++) {
        memcpy(&packet[6 + i * 6], mac, 6);
    }

    udp.begin(9);                 // port 9
    udp.beginPacket("164.1.1.255", 9);
    udp.write(packet, sizeof(packet));
    udp.endPacket();

    Serial.print("WOL sent to MAC: ");
    for (int i = 0; i < 6; i++) {
        Serial.print(mac[i], HEX);
        if (i < 5) Serial.print(":");
    }
    Serial.println();
}


// อัพเดทข้อมูล EEPROM ผ่าน Serial
void updateEEPROM() {
    if (Serial.available() > 0) {
        String input = Serial.readStringUntil('\n');
        input.trim();

        if (input.startsWith("SSID=")) {
            String ssid = input.substring(5);
            writeEEPROM(SSID_ADDR, ssid);
            Serial.println("SSID updated successfully!");

        } else if (input.startsWith("PASSWORD=")) {
            String password = input.substring(9);
            writeEEPROM(PASSWORD_ADDR, password);
            Serial.println("Password updated successfully!");

        } else if (input.startsWith("API_KEY=")) {
            String apiKey = input.substring(8);
            apiKey = apiKey.substring(0, 64);
            writeEEPROM(API_KEY_ADDR, apiKey);
            Serial.println("API Key updated successfully!");

        } else if (input.startsWith("MAC=")) {
            String macAddress = input.substring(4);
            if (macAddress.length() == 17) {  // xx:xx:xx:xx:xx:xx
                byte mac[6];
                int i = 0;
                for (int j = 0; j < macAddress.length(); j += 3) {
                    String byteStr = macAddress.substring(j, j + 2);
                    mac[i++] = strtol(byteStr.c_str(), NULL, 16);
                }
                for (int i = 0; i < 6; i++) {
                    EEPROM.write(MAC_ADDR + i, mac[i]);
                }
                EEPROM.commit();
                Serial.println("MAC Address updated successfully!");
            } else {
                Serial.println("Invalid MAC Address format. Use xx:xx:xx:xx:xx:xx");
            }

        } else if (input == "CLEAR") {
            clearEEPROM();

        } else {
            Serial.println("Invalid command. Use SSID=your_ssid, PASSWORD=your_password, API_KEY=your_api_key, MAC=your_mac_address, or CLEAR");
        }
    }
}

// ======================= Setup =======================
void setup() {
    Serial.begin(115200);
    EEPROM.begin(EEPROM_SIZE);

    // อ่านค่าจาก EEPROM
    String ssid = readEEPROM(SSID_ADDR, 32);
    String password = readEEPROM(PASSWORD_ADDR, 32);
    String apiKey = readEEPROM(API_KEY_ADDR, 64);
    
    
    // เชื่อมต่อ Wi-Fi
    WiFi.begin(ssid.c_str(), password.c_str());
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.println("Connecting to WiFi...");
    }
    Serial.println("Connected to WiFi");

    printEEPROMData();
    
    // ตั้งค่า API POST สำหรับ WOL
    server.on("/api/api/api/2025/noksong/nok/nokcraft/pnc/kuy56/song/tat/game/wol/api/api/2024/wol", HTTP_POST, []() {
        String body = server.arg("plain");  // อ่าน raw body
        Serial.println(body);                // แสดง JSON ที่ส่งมา

        // แปลง JSON
        StaticJsonDocument<200> doc;
        DeserializationError error = deserializeJson(doc, body);

        if (!error) {
            String apiKeyReceived = doc["api_key"];
            String keyReceived = doc["key"];

            Serial.println(apiKeyReceived);
            Serial.println(keyReceived);

            if (apiKeyReceived == readEEPROM(API_KEY_ADDR, 64) && keyReceived == "wol") {
                sendWOLfromEEPROM();

                // สร้าง JSON ตอบกลับ
                StaticJsonDocument<200> resDoc;
                resDoc["success"] = true;
                resDoc["message"] = "WOL packet sent!";

                String jsonResponse;
                serializeJson(resDoc, jsonResponse);

                server.send(200, "application/json", jsonResponse);
            } else {
                // JSON สำหรับ error
                StaticJsonDocument<200> resDoc;
                resDoc["success"] = false;
                resDoc["message"] = "Forbidden: Invalid API Key or key";

                String jsonResponse;
                serializeJson(resDoc, jsonResponse);

                server.send(403, "application/json", jsonResponse);
            }
        } else {
            server.send(400, "application/json", "{\"success\":false,\"message\":\"Invalid JSON\"}");
        }
    });


    server.onNotFound([]() {});  // ไม่ตอบกลับ request ที่ไม่รู้จัก
    server.begin();
}

// ======================= Loop =======================
void loop() {
    server.handleClient();
    updateEEPROM();  // อัพเดทข้อมูลจาก Serial
}
