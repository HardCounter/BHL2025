#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "DHT.h"

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// --- PINY STERUJĄCE MULTIPLEKSEREM ---
#define MUX_S0 4
#define MUX_S1 5
#define MUX_S2 6
#define MUX_S3 7
#define MUX_SIG A0  // Wszystkie odczyty przyjdą tym jednym pinem!

// --- MAPA KANAŁÓW MULTIPLEKSERA (Gdzie co wpiąłeś) ---
#define CH_GAZ      0
#define CH_FOTO     1
#define CH_TEMP_AN  2
#define CH_WODA     3
#define CH_GLEBA    4
#define CH_OGIEN    5
#define CH_HALAS_1  6
#define CH_HALAS_2  7

// --- INNE PINY ---
#define DHTPIN 2
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

void setup() {
  Serial.begin(9600);
  
  // Konfiguracja pinów sterujących MUX
  pinMode(MUX_S0, OUTPUT);
  pinMode(MUX_S1, OUTPUT);
  pinMode(MUX_S2, OUTPUT);
  pinMode(MUX_S3, OUTPUT);
  
  dht.begin();

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
    Serial.println(F("Błąd OLED")); for(;;);
  }
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.println(F("System MUX Start..."));
  display.display();
  delay(1000);
}

// --- FUNKCJA CZYTAJĄCA Z MULTIPLEKSERA ---
int readMux(int channel) {
  // Ustawienie 4 pinów cyfrowych w układ binarny kanału (0-15)
  digitalWrite(MUX_S0, (channel & 1));      // Bit 0
  digitalWrite(MUX_S1, (channel & 2) >> 1); // Bit 1
  digitalWrite(MUX_S2, (channel & 4) >> 2); // Bit 2
  digitalWrite(MUX_S3, (channel & 8) >> 3); // Bit 3
  
  delay(5); // Krótki czas na ustabilizowanie styku
  return analogRead(MUX_SIG); // Odczyt z A0
}

void loop() {
  // --- 1. ODCZYTY ---
  // DHT (bezpośrednio)
  float t_dht = dht.readTemperature();
  float h_dht = dht.readHumidity();

  // MUX (przez funkcję)
  int gaz = readMux(CH_GAZ);
  int swiatlo = readMux(CH_FOTO);
  // Dla LM35:
  float t_analog = (readMux(CH_TEMP_AN) * 5.0 / 1024.0) * 100;
  int woda = readMux(CH_WODA);
  int gleba = readMux(CH_GLEBA);
  int ogien = readMux(CH_OGIEN);
  int halas1 = readMux(CH_HALAS_1);
  int halas2 = readMux(CH_HALAS_2);

  // --- 2. WYŚWIETLANIE (Podział na 3 ekrany, bo dużo danych!) ---
  
  // EKRAN 1: Klimat
  display.clearDisplay();
  display.setCursor(0,0);
  display.println(F("-- KLIMAT --"));
  display.print(F("DHT: ")); display.print((int)t_dht); display.print(F("C "));
  display.print((int)h_dht); display.println(F("%"));
  display.print(F("ANA: ")); display.print(t_analog, 1); display.println(F("C"));
  display.print(F("Swiatlo: ")); display.println(swiatlo);
  display.display();
  delay(2500);

  // EKRAN 2: Bezpieczeństwo
  display.clearDisplay();
  display.setCursor(0,0);
  display.println(F("-- BEZPIECZENSTWO --"));
  
  display.print(F("Gaz: ")); display.println(gaz);
  if(gaz > 400) { display.setCursor(60, 10); display.print(F("!ALARM!")); }

  display.print(F("Ogien: ")); display.println(ogien);
  // Ogień analogowy: zazwyczaj < 100 to ogień
  if(ogien < 100) { display.setCursor(60, 20); display.print(F("!POZAR!")); }
  
  display.print(F("Woda: ")); display.println(woda);
  display.display();
  delay(2500);

  // EKRAN 3: Hałas i Gleba
  display.clearDisplay();
  display.setCursor(0,0);
  display.println(F("-- INNE --"));
  display.print(F("Gleba: ")); display.println(gleba);
  display.print(F("Mic 1: ")); display.println(halas1);
  display.print(F("Mic 2: ")); display.println(halas2);
  display.display();
  delay(2500);
}