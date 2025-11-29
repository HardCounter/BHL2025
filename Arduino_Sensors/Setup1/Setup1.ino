#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "DHT.h"

// --- KONFIGURACJA EKRANU OLED ---
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// --- PINY CZUJNIKÓW ---
#define DHTPIN 2       // DHT11
#define DHTTYPE DHT11

// Piny analogowe
#define PIN_GAZ    A0  // MQ-9
#define PIN_OGIEN  A1  // Czujnik płomienia
#define PIN_HALAS  A2  // Mikrofon
#define PIN_GLEBA  A3  // Sygnał z gleby

DHT dht(DHTPIN, DHTTYPE);

void setup() {
  Serial.begin(9600);
  
  dht.begin();

  // Start Ekranu (Adres 0x3C lub 0x3D)
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
    Serial.println(F("Błąd ekranu OLED!"));
    for(;;); // Zatrzymaj program
  }
  
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0,0);
  display.println(F("System start..."));
  display.display();
  delay(2000); 
}

void loop() {
  // --- 1. ODCZYTY ---
  
  // DHT11
  float t = dht.readTemperature();
  float h = dht.readHumidity();

  // Czujniki analogowe
  int gleba = analogRead(PIN_GLEBA);
  int gaz = analogRead(PIN_GAZ);
  int ogien = analogRead(PIN_OGIEN);
  int halas = analogRead(PIN_HALAS);

  // --- 2. WYŚWIETLANIE NA EKRANIE ---
  display.clearDisplay();
  display.setCursor(0,0);
  
  // Nagłówek
  display.println(F("- STATUS DOMU -"));
  
  // Linia 1: Klimat
  display.print(F("Temp:")); display.print(t, 0); display.print(F("C "));
  display.print(F("Wilg:")); display.print(h, 0); display.println(F("%"));
  
  // Linia 2: Gleba i Hałas
  display.print(F("Gleba:")); display.print(gleba);
  display.print(F(" dB:")); display.println(halas);
  
  // Linia 3: Separator
  display.println(F("---------------------"));
  
  // Linia 4: STATUS ALARMOWY
  // Tutaj ekran wyświetli ostrzeżenie, jeśli coś jest nie tak
  
  if (ogien < 200) {
    // Alarm Pożarowy (priorytet)
    display.setTextSize(2); // Powiększ tekst
    display.setCursor(0, 45);
    display.print(F("! POZAR !"));
    display.setTextSize(1); // Wróć do małego
  } 
  else if (gaz > 400) {
    // Alarm Gazowy
    display.setTextSize(2); // Powiększ tekst
    display.setCursor(0, 45);
    display.print(F("! GAZ !"));
    display.setTextSize(1); // Wróć do małego
  } 
  else {
    // Stan normalny - wyświetlamy poziom gazu liczbowo
    display.print(F("Gaz (CO): ")); 
    display.println(gaz);
    display.println(F("Status: BEZPIECZNIE"));
  }

  display.display();
  
  // Odświeżanie co 2 sekundy
  delay(2000);
}