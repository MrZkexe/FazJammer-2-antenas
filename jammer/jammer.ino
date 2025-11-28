#include <SPI.h>
#include "RF24.h"
#include <ezButton.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <string>
#include "images.h"

// Radio1 -> CE = D4 (GPIO2), CSN = D2 (GPIO4)
RF24 radio1(2, 4);

// Radio2 -> CE = D3 (GPIO0), CSN = D1 (GPIO5)
RF24 radio2(0, 5);

byte i = 45;
ezButton button(3);
Adafruit_SSD1306 display = Adafruit_SSD1306(128, 64, &Wire);

const int wifiFrequencies[] = {
  2412, 2417, 2422, 2427, 2432, 2437,
  2442, 2447, 2452, 2457, 2462
};

void displayMessage(const char* line, uint8_t x = 55, uint8_t y = 22, const unsigned char* bitmap = helpy_menu_image) {
  
  radio1.powerDown();
  radio2.powerDown();
  SPI.end();
  delay(10);

  display.clearDisplay();
  if (bitmap != nullptr) {
    display.drawBitmap(0, 0, bitmap, 128, 64, WHITE);
  }

  display.setTextSize(1);
  String text = String(line);
  int16_t cursor_y = y;
  int16_t maxWidth = 128 - x;

  while (text.length() > 0) {
    int16_t charCount = 0;
    int16_t lineWidth = 0;

    while (charCount < text.length() && lineWidth < maxWidth) {
      charCount++;
      lineWidth = 6 * charCount;
    }

    if (charCount < text.length()) {
      int16_t lastSpace = text.substring(0, charCount).lastIndexOf(' ');
      if (lastSpace > 0) {
        charCount = lastSpace + 1;
      }
    }

    display.setCursor(x, cursor_y);
    display.println(text.substring(0, charCount));
    text = text.substring(charCount);
    cursor_y += 10;
    if (cursor_y > 64) break;
  }

  display.display();

  SPI.begin();
  radio1.powerUp();
  radio2.powerUp();
  delay(5);

  radio1.startConstCarrier(RF24_PA_MAX, i);
  radio2.startConstCarrier(RF24_PA_MAX, i);
}

void addvertising() {
  for (size_t t = 0; t < 3; t++) {
    displayMessage("", 60, 22, helpy_big_image);
    delay(310);
    displayMessage("", 60, 22, nullptr);
    delay(300);
  }
  displayMessage("v2 by Zk - duas antenas. Clique no botao!", 65, 6);
}

bool setupRadio(RF24 &r, const char* name) {
  if (r.begin()) {
    delay(200);

    r.setAutoAck(false); 
    r.stopListening();
    r.setRetries(0, 0);
    r.setPayloadSize(5);
    r.setAddressWidth(3);
    r.setPALevel(RF24_PA_MAX);
    r.setDataRate(RF24_2MBPS);
    r.setCRCLength(RF24_CRC_DISABLED);

    Serial.print(name);
    Serial.println(" iniciado com sucesso.");
    r.printPrettyDetails();
    return true;

  } else {
    Serial.print(name);
    Serial.println(" FALHOU!");
    return false;
  }
}

void setup() {
  Serial.begin(9600);
  button.setDebounceTime(100);
  pinMode(3, INPUT_PULLUP);

  Wire.begin(14, 12);  // SDA, SCL

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("OLED screen falhou!"));
    exit(0);
  }

  display.clearDisplay();
  display.setCursor(0, 0);
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.print(feragatname);
  display.display();
  delay(900);

  displayMessage("modificado por zk");
  delay(900);

  SPI.begin();

  displayMessage("Testando antena 1...");
  bool ok1 = setupRadio(radio1, "Radio1");

  displayMessage("Testando antena 2...");
  bool ok2 = setupRadio(radio2, "Radio2");

  if (!ok1 || !ok2) {
    displayMessage("ERRO: Uma antena falhou!");
    Serial.println("FATAL ERROR: Uma das antenas falhou!");
    while (true) { delay(1000); }
  }

  radio1.startConstCarrier(RF24_PA_MAX, i);
  radio2.startConstCarrier(RF24_PA_MAX, i);

  addvertising();
}

void fullAttack() {
  for (size_t ch = 0; ch < 80; ch++) {
    radio1.setChannel(ch);
    radio2.setChannel(ch);
  }
}

void wifiAttack() {
  for (int idx = 0; idx < sizeof(wifiFrequencies) / sizeof(wifiFrequencies[0]); idx++) {
    uint8_t ch = wifiFrequencies[idx] - 2400;
    radio1.setChannel(ch);
    radio2.setChannel(ch);
  }
}

const char* modes[] = {
  "BLE & tudo 2.4",
  "Modo Wi-Fi",
  "Esperando..."
};

uint8_t attack_type = 2;

void loop() {
  button.loop();

  if (button.isPressed()) {
    attack_type = (attack_type + 1) % 3;
    displayMessage((String(modes[attack_type])+" Mode").c_str());
  }

  switch (attack_type) {
    case 0: fullAttack(); break;
    case 1: wifiAttack(); break;
    case 2: break;
  }
}
