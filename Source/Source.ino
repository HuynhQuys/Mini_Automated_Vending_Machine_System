// ==== KHAI BÁO THƯ VIỆN ====
#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>
#include <SPI.h>
#include <MFRC522.h>
#include <ESP32Servo.h>

// ==== WIFI ====
const char* ssid = "HuynhQuys";
const char* password = "244466666";

// ==== GOOGLE SCRIPT ====
const String scriptURL = "https://script.google.com/macros/s/AKfycbz6kQmtLExcuUelAeKr0sWueTosuHKTVhTfu-ygqH-k5GDo0uqm0nww5BEHbX5gKVov/exec";

// ==== CHÂN KẾT NỐI ====
#define RST_PIN  4
#define SS_PIN   5
#define BUZZER_PIN  15

// ==== LCD & KEYPAD ====
LiquidCrystal_I2C lcd(0x27, 20, 4);
const byte ROWS = 4;
const byte COLS = 4;
char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};
byte rowPins[ROWS] = {0, 13, 34, 35};
byte colPins[COLS] = {32, 33, 25, 26};
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// ==== RFID ====
MFRC522 rfid(SS_PIN, RST_PIN);
String currentUID = "";
bool rfidDetected = false;

// ==== SERVO ====
Servo servo1, servo2, servo3;

// ==== GIÁ SẢN PHẨM ====
int PRODUCT_PRICES[] = {0, 1, 2, 3};
int userBalance = 0;
String enteredCode = "";
String userName = "";

void setup() {
  Serial.begin(115200);

  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Dang ket noi WiFi...");
  
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  lcd.clear();
  lcd.setCursor(2, 1);
  lcd.print("WiFi da ket noi");
  lcd.setCursor(4, 2);
  lcd.print("thanh cong!");
  delay(1000);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("May ban hang tu dong");
  lcd.setCursor(1, 1);
  lcd.print("Xin chao Quy khach");
  lcd.setCursor(8, 2);
  lcd.print("****");
  lcd.setCursor(2, 3);
  lcd.print("Doi quet the...");
  delay(2000);

  SPI.begin(18, 19, 23, SS_PIN);
  rfid.PCD_Init();

  servo1.attach(27);
  servo2.attach(14);
  servo3.attach(12);

  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);
}

void loop() {
 if (!rfidDetected) {
  if (rfid.PICC_IsNewCardPresent() && rfid.PICC_ReadCardSerial()) {
    currentUID = "";
    for (byte i = 0; i < rfid.uid.size; i++) {
      if (rfid.uid.uidByte[i] < 0x10) currentUID += "0";
      currentUID += String(rfid.uid.uidByte[i], HEX);
      currentUID.toUpperCase();
    }
    longBeep();
    Serial.println("UID đọc được: " + currentUID);

    rfid.PICC_HaltA();

      // Gọi Google Script để lấy tên và số dư
      String userData = getUserInfo(currentUID);
      if (userData.startsWith("UNKNOWN")) {
       rfidDetected = false;
       return;
      }
      int separatorIndex = userData.indexOf('|');
      userName = userData.substring(0, separatorIndex);
      userBalance = userData.substring(separatorIndex + 1).toInt();

      rfidDetected = true;
     
      lcd.clear();
      lcd.setCursor(1, 1);
      lcd.print("Khach hang: " + userName);
      lcd.setCursor(1, 2);
      lcd.print("So du: $" + String(userBalance));
      delay(2500);

      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("So du: $" + String(userBalance));
      lcd.setCursor(0, 1);
      lcd.print("Chon san pham");
      lcd.setCursor(0, 2);
      lcd.print("bang phim 1 2 3");
      lcd.setCursor(0, 3);
      lcd.print("Nhan phim # de thoat");
      delay(1500);
      enteredCode = "";
    }
    return;
  }

  char key = keypad.getKey();
  if (key) {
    beep();

    if (key == '*') {
      int code = enteredCode.toInt();
      if (code >= 1 && code <= 3) {
        int price = PRODUCT_PRICES[code];
        if (userBalance >= price) {
          userBalance -= price;
          lcd.clear();
          lcd.setCursor(3, 1);
          lcd.print("Thanh toan OK");
          lcd.setCursor(6, 2);
          lcd.print("Gia: $" + String(price));
          beep();
          delay(1000);
          activateServo(code);
        } else {
          lcd.clear();
          lcd.setCursor(3, 0);
          lcd.print("Khong du tien");
          lcd.setCursor(6, 1);
          lcd.print("Gia: $" + String(price));
          lcd.setCursor(1, 2);
          lcd.print("Vui long nap tien!");
          longBeep();
        }
      } else {
        lcd.clear();
        lcd.setCursor(2, 1);
        lcd.print("Ma khong hop le");
        lcd.setCursor(1, 2);
        lcd.print("Vui long nhap lai!");
        longBeep();
      }
      delay(2000);
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("So du: $" + String(userBalance));
      lcd.setCursor(0, 1);
      lcd.print("Chon san pham:");
      lcd.setCursor(0, 2);
      lcd.print("Nhan * de xac nhan");
      lcd.setCursor(0, 3);
      lcd.print("Nhan # de thoat");
      enteredCode = "";

    } else if (key == '#') {
      updateBalance(currentUID, userBalance);  // Cập nhật số dư mới
      lcd.clear();
      lcd.setCursor(1, 0);
      lcd.print("Ket thuc giao dich");
      lcd.setCursor(5, 1);
      lcd.print("Xin cam on");
      lcd.setCursor(9, 2);
      lcd.print("va");
      lcd.setCursor(4, 3);
      lcd.print("Hen gap lai!");
      delay(2000);

      rfidDetected = false;
      currentUID = "";
      userBalance = 0;
      enteredCode = "";
      rfid.PCD_Init();

      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("May ban hang tu dong");
      lcd.setCursor(1, 1);
      lcd.print("Xin chao Quy khach");
      lcd.setCursor(8, 2);
      lcd.print("****");
      lcd.setCursor(2, 3);
      lcd.print("Doi quet the...");
    }

    else if (isDigit(key)) {
      if (enteredCode.length() < 2) {
        enteredCode += key;
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Ma SP: " + enteredCode);
        lcd.setCursor(0, 1);
        lcd.print("Nhan * de xac nhan");
        lcd.setCursor(0, 2);
        lcd.print("Nhan # de thoat");
      }
    }
  }
}

// ==== HÀM PHỤ ====
void beep() {
  digitalWrite(BUZZER_PIN, HIGH);
  delay(100);
  digitalWrite(BUZZER_PIN, LOW);
}

void longBeep() {
  digitalWrite(BUZZER_PIN, HIGH);
  delay(500);
  digitalWrite(BUZZER_PIN, LOW);
}

void activateServo(int productCode) {
  switch (productCode) {
    case 1:
      servo1.write(0); delay(1120); servo1.write(90);
      break;
    case 2:
      servo2.write(0); delay(1070); servo2.write(90);
      break;
    case 3:
      servo3.write(0); delay(1210); servo3.write(90);
      break;
  }
}

// ==== HÀM GỌI API GOOGLE SCRIPT ====
String getUserInfo(String uid) {
  HTTPClient http;
  WiFiClientSecure client;
  client.setInsecure();  // Bỏ kiểm tra SSL

  String url = scriptURL + "?uid=" + uid;
  http.begin(client, url);
  http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);

  int httpCode = http.GET();
  if (httpCode == HTTP_CODE_OK) {
    String payload = http.getString();
    Serial.println("Phản hồi từ server: " + payload);
    http.end();  // Dọn tài nguyên

    // Kiểm tra nếu phản hồi là UNKNOWN
    if (payload.startsWith("UNKNOWN")) {
      lcd.clear();
      lcd.setCursor(1, 1);
      lcd.print("Khong tim thay UID");
      lcd.setCursor(8, 2);
      lcd.print("!!!!");
      delay(5000);
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("May ban hang tu dong");
      lcd.setCursor(1, 1);
      lcd.print("Xin chao Quy khach");
      lcd.setCursor(8, 2);
      lcd.print("****");
      lcd.setCursor(2, 3);
      lcd.print("Doi quet the...");
    }

    return payload;
  } else {
    Serial.println("Lỗi HTTP: " + String(httpCode));
    http.end();  // Dọn tài nguyên
    return "UNKNOWN";
  }
}



void updateBalance(String uid, int newBalance) {
  HTTPClient http;
  http.begin("https://script.google.com/macros/s/AKfycbz6kQmtLExcuUelAeKr0sWueTosuHKTVhTfu-ygqH-k5GDo0uqm0nww5BEHbX5gKVov/exec");
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");

  String postData = "action=update&uid=" + uid + "&sodu=" + String(newBalance);
  int httpCode = http.POST(postData);

  if (httpCode > 0) {
    String response = http.getString();
    Serial.println("Cập nhật số dư: " + response);
  } else {
    Serial.println("Lỗi cập nhật số dư");
  }

  http.end();
}
