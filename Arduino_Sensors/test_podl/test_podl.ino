#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "DHT.h"
#include <Servo.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// --- KONFIGURACJA PINÓW ---
// Bezpośrednie
#define PIN_DHT     2
#define PIN_SERVO   5   // Serwo sterujące ZAWOREM WODY
#define PIN_GAZ     A1  // MQ-2 (Dym) 
#define PIN_OGIEN   A2  // Flame Sensor 
#define PIN_GLEBA   A3  // Soil Moisture

// --- 2. MULTIPLEKSER (Sterowanie i Sygnał) ---
#define MUX_S0 8
#define MUX_S1 9
#define MUX_S2 10
#define MUX_S3 11
#define MUX_SIG A0      

// --- MAPA KANAŁÓW W MUX ---
#define CH_TEMP_NTC 2   // C2 (Poprawiona nazwa dla spójności)
#define CH_WODA     3   // C3 (Czujnik w Rezerwuarze Górnym)
#define CH_MIC1     6   // C6
#define CH_MIC2     7   // C7

// Obiekty
DHT dht(PIN_DHT, DHT11);
Servo valveServo;

// Konfiguracja Zaworu
const int VALVE_CLOSED = 0;  
const int VALVE_OPEN   = 90; 

void setup() {
  Serial.begin(9600); 
  Serial.println(F("--- START SYSTEMU SEXY-HEXY ---"));
  
  pinMode(MUX_S0, OUTPUT);
  pinMode(MUX_S1, OUTPUT);
  pinMode(MUX_S2, OUTPUT);
  pinMode(MUX_S3, OUTPUT);

  // Serwo - na start zamykamy
  valveServo.attach(PIN_SERVO);
  valveServo.write(VALVE_CLOSED);

  dht.begin();

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
    Serial.println(F("Błąd OLED")); 
  }
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(2);
  display.setCursor(10,10);
  display.println(F("SEXY-HEXY"));
  display.setTextSize(1);
  display.setCursor(10,35);
  display.println(F("Tank Monitor v3.0"));
  display.display();
  delay(2000);
}

// Funkcja wyboru kanału w MUX
int readMux(int channel) {
  digitalWrite(MUX_S0, (channel & 1));      
  digitalWrite(MUX_S1, (channel & 2) >> 1); 
  digitalWrite(MUX_S2, (channel & 4) >> 2); 
  digitalWrite(MUX_S3, (channel & 8) >> 3); 
  delay(10); 
  return analogRead(MUX_SIG); 
}

// Funkcja dla modułu KY-013 (NTC)
float getNTCTemp(int raw) {
  if (raw == 0 || raw == 1023) return -999;
  const float R_SERIES = 10000; 
  const float NOMINAL_RESISTANCE = 10000; 
  float resistance = R_SERIES * raw / (1023.0 - raw);
  float steinhart;
  steinhart = resistance / NOMINAL_RESISTANCE;     
  steinhart = log(steinhart);                      
  steinhart /= 3950.0;                             
  steinhart += 1.0 / (25.0 + 273.15);              
  steinhart = 1.0 / steinhart;                     
  steinhart -= 273.15;                             
  return steinhart + 19.5; // Twoja kalibracja (Offset)
}

void loop() {
  // --- 1. ODCZYT SENSORÓW ---
  float hum_air = dht.readHumidity();
  
  // MUX
  float temp_wall = getNTCTemp(readMux(CH_TEMP_NTC));
  
  // ZBIORNIK WODY (REZERWUAR) [cite: 40]
  int tank_raw = readMux(CH_WODA);
  // Kalibracja zbiornika: 0=Pusto, 500=Pełno (typowa wartość dla wody kranowej)
  int tank_percent = map(tank_raw, 0, 500, 0, 100);
  if(tank_percent > 100) tank_percent = 100;
  if(tank_percent < 0) tank_percent = 0;

  // Mech (Gleba)
  int soil_raw = analogRead(PIN_GLEBA); 
  int soil_percent = map(soil_raw, 1023, 300, 0, 100); 
  if(soil_percent < 0) soil_percent = 0;
  if(soil_percent > 100) soil_percent = 100;

  // Bezpieczeństwo
  int smoke = analogRead(PIN_GAZ);  
  int fire = analogRead(PIN_OGIEN); 
  
  // Akustyka
  int noise = readMux(CH_MIC1);

  // --- 2. LOGIKA STEROWANIA (AI SYSTEMU) ---
  
  bool valve_state = false; 
  String status_msg = "OK";
  bool low_water_warning = (tank_percent < 10); // Ostrzeżenie poniżej 10%

  // A. Logika PPOŻ (Priorytet najwyższy - Flood Mode) [cite: 22]
  // Nawet jak jest mało wody, otwieramy, żeby spuścić resztki na ogień
  if (smoke > 300 || fire < 500) { 
    valve_state = true; 
    status_msg = "! POZAR !";
  }
  // B. Logika Nawadniania (Smart Irrigation)
  else if (soil_percent < 30) { 
    if (hum_air < 80) {         
      if (!low_water_warning) {
        // Podlewamy TYLKO gdy jest woda w zbiorniku
        valve_state = true;      
        status_msg = "PODLEWANIE";
      } else {
        // Mech suchy, ale brak wody w zbiorniku!
        valve_state = false;
        status_msg = "BRAK WODY!"; 
      }
    } else {
      status_msg = "WILGOTNO (STOP)"; 
    }
  }
  else {
    status_msg = "STAN OPTYMALNY"; 
  }

  // --- 3. ELEMENT WYKONAWCZY ---
  if (valve_state) {
    valveServo.write(VALVE_OPEN);
  } else {
    valveServo.write(VALVE_CLOSED);
  }

  // --- 4. WYŚWIETLANIE ---
  display.clearDisplay();
  
  if (status_msg == "! POZAR !") {
    // EKRAN ALARMOWY
    display.setCursor(10, 10);
    display.setTextSize(3);
    display.print(F("POZAR!"));
    display.setTextSize(1);
    display.setCursor(10, 45);
    display.print(F("ZBIORNIK: ")); display.print(tank_percent); display.print(F("%"));
    display.setCursor(10, 55);
    display.print(F("ZAWOR OTWARTY!"));
  } else {
    // EKRAN NORMALNY
    display.setTextSize(1);
    
    // LINIA 1: Mech i Zawór
    display.setCursor(0,0);
    display.print(F("MECH: ")); 
    display.print(soil_percent); display.print(F("% "));
    if(valve_state) display.print(F("[OTWARTY]"));
    else display.print(F("[ZAMKN]"));

    display.drawLine(0, 10, 128, 10, SSD1306_WHITE);

    // LINIA 2: Status Systemu
    display.setCursor(0, 15);
    display.print(F("Status: ")); display.println(status_msg);

    // LINIA 3: Parametry Fasady
    display.setCursor(0, 28);
    display.print(F("Mur:")); display.print((int)temp_wall); display.print(F("C "));
    display.print(F("Air:")); display.print((int)hum_air); display.print(F("%"));

    // LINIA 4: ZBIORNIK WODY (Kluczowe dla obsługi)
    display.setCursor(0, 38);
    display.print(F("Zbiornik: ")); 
    display.print(tank_percent); display.print(F("% "));
    if(low_water_warning) display.print(F("(!DOLEJ)"));

    // LINIA 5: Bezpieczeństwo/Hałas
    display.setCursor(0, 48);
    // Wyświetlamy dym, chyba że jest czysto, wtedy hałas
    if (smoke > 100) {
       display.print(F("Dym: ")); display.print(smoke);
    } else {
       display.print(F("Halas: ")); display.print(noise); display.print(F("dB"));
    }
  }

  display.display();
  delay(200); 
}