#include <ArduinoSTL.h> //Các thư viện để sử dụng module RFID
#include <map>
#include <SPI.h>
#include <MFRC522.h>

#include <Wire.h> //Gọi thư viện I2C để sử dụng các thư viện I2C
#include <LiquidCrystal_I2C.h> //Thư viện LCD I2C

#define SS_PIN 10
#define RST_PIN 9

String THISROOM = "Room 602";
const int outPin = 8;
LiquidCrystal_I2C lcd(0x27, 16, 2); //Khai báo địa chỉ I2C (0x27 or 0x3F) và LCD 16x02
std::map<String, String> roomNumbers = {
  {"162 191 249 33", "Room 602" },
  {"10 223 136 23", "Room 603"}
};

MFRC522 mfrc522(SS_PIN, RST_PIN); // Khởi tạo đối tượng MFRC522

void setup() {
  Serial.begin(9600); // Mở cổng Serial
  SPI.begin();  // Thiết lập các chân và thanh ghi theo SPI example
  mfrc522.PCD_Init(); // Khởi tạo module giao tiếp với RFID

  pinMode(outPin, OUTPUT); // Khai báo chân gửi tín hiệu ra

  lcd.init(); //Khởi tạo màn hình LCD
  lcd.backlight(); //Bật đèn màn hình lCD
}

uint8_t buf[10] = {};
MFRC522::Uid id;
MFRC522::Uid id2;
bool is_card_present = false;
uint8_t control = 0x00;
String HexToString(uint8_t *data, uint8_t length) // converts 8-bit data to a string in decimal with leading zeroes
{
  String result = "";
  char tmp[16];
  for (int i = 0; i < length; i++) {
    sprintf(tmp, "%u", data[i]);
    result += tmp;
    if (i != length - 1) {
      result += " ";
    }
  }
  return result;
}
//*****************************************************************************************//
void cpid(MFRC522::Uid *id) {
  memset(id, 0, sizeof(MFRC522::Uid));
  memcpy(id->uidByte, mfrc522.uid.uidByte, mfrc522.uid.size);
  id->size = mfrc522.uid.size;
  id->sak = mfrc522.uid.sak;
}

void loop() {
  MFRC522::MIFARE_Key key;
  for (byte i = 0; i < 6; i++) key.keyByte[i] = 0xFF;
  MFRC522::StatusCode status;

  // Look for new cards
  if ( !mfrc522.PICC_IsNewCardPresent()) {
    return;
  }
  if ( !mfrc522.PICC_ReadCardSerial()) {
    return;
  }

  bool result = true;
  uint8_t buf_len = 4;
  cpid(&id);
  String card_uid = HexToString(id.uidByte, id.size);
  Serial.print("Card UID: ");
  Serial.print(card_uid);
  Serial.println("");
  Serial.println(roomNumbers[card_uid]);


  //TODO: CHECK IF THE CARD UID IS VALID
  if (roomNumbers[card_uid] == THISROOM) {
    Serial.println("Access granted");
//    digitalWrite(outPin, HIGH);
    lcd.setCursor(0, 0);
    lcd.print("Access granted");
    Serial.println(roomNumbers[card_uid]);
    lcd.setCursor(0, 1);
    lcd.print(roomNumbers[card_uid]);
  } else {
    Serial.println("Access denied");
//    digitalWrite(outPin, LOW); 
    lcd.setCursor(0, 0);
    lcd.print("Access denied");
    delay(2000);
    lcd.clear();
    return;
  }
  while (true) {
    Serial.println("Card is still here");
    control = 0;
    for (int i = 0; i < 3; i++) {
      if (!mfrc522.PICC_IsNewCardPresent()) {
        if (mfrc522.PICC_ReadCardSerial()) {
          control |= 0x16;
        }
        if (mfrc522.PICC_ReadCardSerial()) {
          control |= 0x16;
        }
        control += 0x1;
      }
      control += 0x4;
    }

    if (control == 13 || control == 14) {
      // thẻ nằm trong phạm vi của đầu đọc
      digitalWrite(outPin, HIGH);
    } else {
      digitalWrite(outPin, LOW);
      break;
    }
  }
  Serial.println("CardRemoved");
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("CardRemoved");
  delay(1000);
  lcd.clear();
  mfrc522.PICC_HaltA(); // dừng giao tiếp với thẻ RFID hiện tại nằm trong phạm vi của đầu đọc
  mfrc522.PCD_StopCrypto1(); // dừng mọi quy trình xác thực đang hoạt động cho thẻ hiện tại
}

