#include "DHT.h"

// --- KONFIGURACJA ---
#define DHTPIN 2        // Pin, do którego podpięty jest DHT11
#define DHTTYPE DHT11   // Twój typ czujnika
#define LED_PIN 8       // Pin, do którego podpięta jest dioda LED

DHT dht(DHTPIN, DHTTYPE);

void setup() {
  Serial.begin(9600); // Start komunikacji z komputerem
  dht.begin();        // Start czujnika
  
  // Ustawienie pinu diody jako wyjście
  pinMode(LED_PIN, OUTPUT);
  
  // Włączenie diody na stałe (wskaźnik, że urządzenie działa)
  digitalWrite(LED_PIN, HIGH); 
  
  Serial.println(F("Start pomiaru temperatury..."));
}

void loop() {
  // Czekamy 2 sekundy między pomiarami
  delay(2000);

  // Odczyt wilgotności i temperatury
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  // Sprawdzenie czy odczyt się udał
  if (isnan(h) || isnan(t)) {
    Serial.println(F("Błąd odczytu z czujnika DHT!"));
    return;
  }

  // Wyświetlenie wyników
  Serial.print(F("Wilgotność: "));
  Serial.print(h);
  Serial.print(F("%  Temperatura: "));
  Serial.print(t);
  Serial.println(F("°C "));
}