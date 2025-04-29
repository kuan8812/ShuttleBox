/*
  ShuttleBox.ino - 加強除錯版

  功能簡述：
  1) 使用固定 Wi-Fi ("Asus_71D5" / "12345678..")
  2) 透過 WebSocket 連線到 Python ws://192.168.50.90:5001
  3) 移除「光纖自動 Shock」, 只由 Python 指令 (shock_on/off) 控制
  4) 每秒回傳 fiber_A, fiber_B 給 Python
  5) OLED 顯示：
     - 默認顯示「ID: ...」
     - 若偵測魚側改變 => 2秒顯示「Fish => side X」
     - 收到 cmd="set_oled" => 顯示自訂文字 (覆蓋畫面)
  6) 主要在 `webSocketEvent()` 中解析 Python 發來的指令，對應執行Shock、NeoPixel、OLED操作
  7) ***加入大量 Serial.println() 除錯輸出，協助您判斷連線、斷線、指令有無進來、是否反覆重啟等***
*/

#include <WiFi.h>
#include <WebSocketsClient.h>
#include <ArduinoJson.h>
#include <Adafruit_NeoPixel.h>

// 假設您有自己的 OledModule、FiberSensorModule
#include "OledModule.h"
#include "FiberSensorModule.h"

// ============ Wi-Fi 設定 ============
const char* ssid     = "ASUS_E0_EBM68";
const char* password = "solution_4953";

// ============ WebSocket ============
String ws_host = "192.168.50.141"; // 請改成 Python電腦IP
//String ws_host = "192.168.50.105"; // 請改成 Python電腦IP
uint16_t ws_port = 5001;
String ws_path = "/";

WebSocketsClient webSocket;

// ============ GPIO 定義 ============
#define SHOCK_PIN   6   // 例: GPIO43(ESP32-S3)
#define FIBER_PIN_A 2   // 例: GPIO3
#define FIBER_PIN_B 3   // 例: GPIO4
#define PIN_STRIP0  0   // 例: GPIO1
#define PIN_STRIP1  1   // 例: GPIO2
#define NUMPIXELS   64  // 假設8x8 NeoPixel

// ============ NeoPixel 物件 ============
Adafruit_NeoPixel strip0(NUMPIXELS, PIN_STRIP0, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel strip1(NUMPIXELS, PIN_STRIP1, NEO_GRB + NEO_KHZ800);

// ============ OLED 與 FiberSensor ============
OledModule oled;
FiberSensorModule fiberSensor(FIBER_PIN_A, FIBER_PIN_B);

// ============ 全域變數 ============
// 由 Python "set_id" 給我們 ID, 預設 Unknown
String deviceIdString = "Unknown";

// 短暫訊息 (fish side變化時顯示2秒)
unsigned long ephemeralDisplayUntil = 0;
String ephemeralMessage = "";

// 上一次偵測到的魚側
char lastFishSide = 'U'; // 'A' or 'B' or 'U'

//------------------ 填滿strip的工具函式 ------------------//
void fillStrip(Adafruit_NeoPixel &strip, uint8_t r, uint8_t g, uint8_t b) {
  for(int i=0; i<strip.numPixels(); i++){
    strip.setPixelColor(i, r, g, b);
  }
  strip.show();
}

//------------------ 根據 fiberA/B 判斷魚側 ------------------//
char getFishSide(int valA, int valB) {
  // <100 => 該側有魚(示範)
  if(valA<100 && valB>100) return 'A';
  if(valB<100 && valA>100) return 'B';
  return 'U';
}

//------------------ 更新OLED顯示 ------------------//
void updateOledDisplay() {
  oled.clear();
  unsigned long now = millis();

  if(now < ephemeralDisplayUntil) {
    // 還在臨時訊息時間
    oled.drawText(ephemeralMessage, 0,0);
  } else {
    // 顯示裝置 ID
    oled.drawText("ID: " + deviceIdString, 0,0);
  }
  oled.display();
}

//------------------ WebSocket事件回呼 ------------------//
void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {
  // 先印出事件型別，方便除錯
  Serial.printf("[WS] Event type=%d\n", type);

  switch(type) {
    case WStype_DISCONNECTED:
      Serial.println("[WS] Disconnected!");
      break;

    case WStype_CONNECTED:
      Serial.println("[WS] Connected to server!");
      // 不帶 device_id => Python 會把我們當 unknown_xxx => auto rename
      webSocket.sendTXT("{\"msg\":\"Hello from unknown!\"}");
      break;

    case WStype_TEXT: {
      // 收到文字訊息
      String msg = String((char*)payload);
      Serial.printf("[WS] Got text: %s\n", msg.c_str());

      StaticJsonDocument<256> doc;
      DeserializationError err = deserializeJson(doc, msg);
      if(err){
        Serial.print("[WS] JSON parse error: ");
        Serial.println(err.c_str());
        return;
      }
      if(doc.containsKey("cmd")){
        String cmd = doc["cmd"].as<String>();
        Serial.print("[WS] cmd: "); 
        Serial.println(cmd);

        //================= set_id =================
        if(cmd=="set_id"){
          if(doc.containsKey("device_id")){
            deviceIdString = doc["device_id"].as<String>();
            Serial.println("[CMD] set_id => " + deviceIdString);
            updateOledDisplay();
          }
        }

        //================= light_on/off =================
        else if(cmd=="light_on"){
          Serial.println("[CMD] light_on => both=white");
          fillStrip(strip0, 255,255,255);
          fillStrip(strip1, 255,255,255);
        }
        else if(cmd=="light_off"){
          Serial.println("[CMD] light_off => both=off");
          fillStrip(strip0, 0,0,0);
          fillStrip(strip1, 0,0,0);
        }

        //================= shock_on/off =================
        else if(cmd=="shock_on"){
          Serial.println("[CMD] shock_on => SHOCK=HIGH");
          digitalWrite(SHOCK_PIN, HIGH);
        }
        else if(cmd=="shock_off"){
          Serial.println("[CMD] shock_off => SHOCK=LOW");
          digitalWrite(SHOCK_PIN, LOW);
        }

        //================= strip0_color / strip1_color =================
        else if(cmd=="strip0_color"){
          int r = doc["r"];
          int g = doc["g"];
          int b = doc["b"];
          Serial.printf("[CMD] strip0_color => (%d,%d,%d)\n", r,g,b);
          fillStrip(strip0, r,g,b);
        }
        else if(cmd=="strip1_color"){
          int r = doc["r"];
          int g = doc["g"];
          int b = doc["b"];
          Serial.printf("[CMD] strip1_color => (%d,%d,%d)\n", r,g,b);
          fillStrip(strip1, r,g,b);
        }

        //================= light_side_A/B/both_blue =================
        else if(cmd=="light_side_A"){
          Serial.println("[CMD] light_side_A => strip0=white, strip1=off");
          fillStrip(strip0,255,255,255);
          fillStrip(strip1,0,0,0);
        }
        else if(cmd=="light_side_B"){
          Serial.println("[CMD] light_side_B => strip0=off, strip1=white");
          fillStrip(strip0,0,0,0);
          fillStrip(strip1,255,255,255);
        }
        else if(cmd=="light_both_blue"){
          Serial.println("[CMD] light_both_blue => both=blue");
          fillStrip(strip0,0,0,255);
          fillStrip(strip1,0,0,255);
        }

        //================= set_oled =================
        else if(cmd=="set_oled"){
          if(doc.containsKey("msg")){
            String msgText = doc["msg"].as<String>();
            Serial.println("[CMD] set_oled => " + msgText);
            oled.clear();
            oled.drawText(msgText, 0,0);
            oled.display();
          }
        }
        //================= 其他未處理的cmd =================
        else {
          Serial.println("[WS] Unknown cmd: " + cmd);
        }
      }
      else {
        Serial.println("[WS] No cmd in message");
      }
    }
    break;

    default:
      break;
  }
}

//------------------ setup() ------------------//
void setup() {
  Serial.begin(115200);
  delay(300);

  Serial.println("\n===== Setup Start =====");

  // Wi-Fi
  Serial.println("[Setup] Connecting Wi-Fi...");
  WiFi.begin(ssid, password);
  while(WiFi.status()!=WL_CONNECTED) {
    delay(300);
    Serial.print(".");
  }
  Serial.print("\n[Setup] WiFi connected, IP: ");
  Serial.println(WiFi.localIP());

  // WebSocket
  Serial.println("[Setup] WebSocket init...");
  webSocket.begin(ws_host.c_str(), ws_port, ws_path.c_str());
  webSocket.onEvent(webSocketEvent);
  webSocket.setReconnectInterval(2000);

  // Shock pin
  pinMode(SHOCK_PIN, OUTPUT);
  digitalWrite(SHOCK_PIN, LOW);

  // NeoPixel
  strip0.begin();
  strip0.setBrightness(128);
  fillStrip(strip0, 0,0,0);

  strip1.begin();
  strip1.setBrightness(128);
  fillStrip(strip1, 0,0,0);

  // OLED
  if(!oled.begin(0x3C)) {
    Serial.println("[OLED] init failed!");
  } else {
    Serial.println("[OLED] init OK");
    oled.clear();
    oled.drawText("Starting up...", 0,0);
    oled.display();
  }

  // FiberSensor
  Serial.println("[Setup] FiberSensor begin");
  fiberSensor.begin();

  // 簡單開機測試 (可自行註解掉以免電流過大)
  fillStrip(strip0, 255,0,0);    
  fillStrip(strip1, 0,255,0);
  delay(1000);
  fillStrip(strip0, 0,0,255);    
  fillStrip(strip1, 255,255,255);
  delay(1000);
  fillStrip(strip0, 0,0,0);
  fillStrip(strip1, 0,0,0);

  // 顯示初始 ID
  updateOledDisplay();

  Serial.println("===== Setup Done =====");
}

//------------------ loop() ------------------//
void loop() {
  // 讓 WebSocket 持續運作
  webSocket.loop();

  // 為了觀察 loop 執行 (避免 blocking)
  static unsigned long debugLoopTime=0;
  unsigned long now=millis();
  if(now - debugLoopTime > 2000){
    debugLoopTime = now;
    Serial.println("loop() running...");
  }

  // 每秒上傳 fiber 狀態
  static unsigned long lastSend=0;
  if(now - lastSend>1000){
    lastSend=now;

    int valA = fiberSensor.readPinA();
    int valB = fiberSensor.readPinB();

    // 傳 JSON 給 Python, 不帶 device_id => auto rename
    String json = "{\"fiber_A\":"+String(valA)+",\"fiber_B\":"+String(valB)+"}";
    webSocket.sendTXT(json);

    // 偵測魚側
    char currentSide = getFishSide(valA, valB);
    if(currentSide!=lastFishSide && currentSide!='U'){
      ephemeralMessage = "Fish => side ";
      ephemeralMessage += currentSide;
      ephemeralDisplayUntil = now + 2000; //顯示2秒
      updateOledDisplay();
    }
    lastFishSide = currentSide;

    // 若臨時訊息到期 => 更新回顯示ID
    if(millis() > ephemeralDisplayUntil){
      static bool wasExpired=false;
      if(!wasExpired){
        wasExpired=true;
        updateOledDisplay();
      }
    } else {
      static bool wasExpired=true;
      if(wasExpired){
        wasExpired=false;
      }
    }
  }
}
