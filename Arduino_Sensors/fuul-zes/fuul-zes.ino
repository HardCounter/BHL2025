#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "DHT.h"
#include <Servo.h>
#include <SoftwareSerial.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
SoftwareSerial hc06(2, 3); 

// --- KONFIGURACJA PINÓW ---
// Bezpośrednie
#define PIN_DHT     7
#define PIN_SERVO   5   // Serwo sterujące ZAWOREM WODY
#define PIN_GAZ     A1  // MQ-2 (Dym) 
#define PIN_OGIEN   A2  // Flame Sensor 
#define PIN_GLEBA   A3  // Soil Moisture

// --- MULTIPLEKSER ---
#define MUX_S0 8
#define MUX_S1 9
#define MUX_S2 10
#define MUX_S3 11
#define MUX_SIG A0      

// --- MAPA KANAŁÓW MUX ---
#define CH_TEMP_NTC 2   // C2 (Temperatura Muru)
#define CH_WODA     3   // C3 (Rezerwuar)
#define CH_MIC1     6   // C6 (Hałas Zewnętrzny/Ulica)
#define CH_MIC2     7   // C7 (Hałas Wewnętrzny/Za Mchem)

// Obiekty
DHT dht(PIN_DHT, DHT11);
Servo valveServo;

// Konfiguracja Zaworu
const int VALVE_CLOSED = 0;  
const int VALVE_OPEN   = 90; 

void setup() {
  Serial.begin(4800); 
  Serial.println(F("--- SEXY-HEXY ACOUSTIC SYSTEM ---"));

  hc06.begin(9600);
  
  pinMode(MUX_S0, OUTPUT);
  pinMode(MUX_S1, OUTPUT);
  pinMode(MUX_S2, OUTPUT);
  pinMode(MUX_S3, OUTPUT);

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
  display.println(F("BRYO-HIVE"));
  display.setTextSize(1);
  display.setCursor(10,35);
  display.println(F("Acoustic & Safety"));
  display.display();
  delay(2000);
}

// Funkcja MUX
int readMux(int channel) {
  digitalWrite(MUX_S0, (channel & 1));      
  digitalWrite(MUX_S1, (channel & 2) >> 1); 
  digitalWrite(MUX_S2, (channel & 4) >> 2); 
  digitalWrite(MUX_S3, (channel & 8) >> 3); 
  delay(10); 
  return analogRead(MUX_SIG); 
}

// Funkcja NTC
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
  return steinhart + 19.5; 
}

void loop() {
  // --- 1. ODCZYT SENSORÓW ---

  float hum_air = dht.readHumidity();
  float temp_air = dht.readTemperature(); // Temp powietrza
  
  // MUX
  float temp_wall = getNTCTemp(readMux(CH_TEMP_NTC)); // Temp muru
  int tank_raw = readMux(CH_WODA);
  
  // AKUSTYKA (NOWOŚĆ)
  int mic_street = readMux(CH_MIC1); // Ulica
  int mic_wall   = readMux(CH_MIC2); // Ściana
  // Obliczamy różnicę (Tłumienie)
  int insulation_diff = abs(mic_street - mic_wall); 

  // Bezpośrednie
  int soil_raw = analogRead(PIN_GLEBA); 
  int smoke = analogRead(PIN_GAZ);  
  int fire = analogRead(PIN_OGIEN); 

  // --- 2. OBLICZENIA ---
  int tank_percent = map(tank_raw, 0, 500, 0, 100);
  if(tank_percent > 100) tank_percent = 100; if(tank_percent < 0) tank_percent = 0;

  int soil_percent = map(soil_raw, 1023, 300, 0, 100); 
  if(soil_percent < 0) soil_percent = 0; if(soil_percent > 100) soil_percent = 100;

  // --- 3. LOGIKA STEROWANIA ---
  bool valve_state = false; 
  String status_msg = "OK";
  bool low_water_warning = (tank_percent < 10); 

  // PPOŻ
  if (smoke > 300 || fire < 500) { 
    valve_state = true; 
    status_msg = "! POZAR !";
  }
  // Nawadnianie
  else if (soil_percent < 30) { 
    if (hum_air < 80) {         
      if (!low_water_warning) {
        valve_state = true;      
        status_msg = "PODLEWANIE";
      } else {
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

  // Wykonawczy
  if (valve_state) valveServo.write(VALVE_OPEN);
  else valveServo.write(VALVE_CLOSED);

  // --- 4. TERMINAL (Dane dla sędziów) ---
  Serial.println(F("========================================"));
  Serial.print(F("AKUSTYKA (C6 vs C7): ")); 
  Serial.print(mic_street); Serial.print(F(" vs ")); Serial.print(mic_wall);
  Serial.print(F(" -> Izolacja: ")); Serial.println(insulation_diff);
  
  Serial.print(F("TERMIKA (Air vs Mur):")); 
  Serial.print(temp_air); Serial.print(F("C vs ")); Serial.print(temp_wall); Serial.println(F("C"));
  
  Serial.print(F("SYSTEM WODY:         ")); Serial.print(tank_percent); Serial.println(F("% (Zbiornik)"));
  if(status_msg == "! POZAR !") Serial.println(F("!!! ALARM POZAROWY - ZAWORY OTWARTE !!!"));
  Serial.println();

  // --- 5. EKRAN OLED ---
  display.clearDisplay();
  
  if (status_msg == "! POZAR !") {
    display.setCursor(10, 10); display.setTextSize(3); display.print(F("POZAR!"));
    display.setTextSize(1); display.setCursor(10, 50); display.print(F("ZAWOR OTWARTY!"));
  } else {
    display.setTextSize(1);
    
    // LINIA 1: Mech i Zbiornik
    display.setCursor(0,0);
    display.print(F("M:")); display.print(soil_percent); display.print(F("% "));
    display.print(F("Woda:")); display.print(tank_percent); display.print(F("%"));
    display.drawLine(0, 10, 128, 10, SSD1306_WHITE);

    // LINIA 2: AKUSTYKA (Kluczowe!)
    display.setCursor(0, 15);
    display.print(F("Tlumienie: ")); display.print(insulation_diff); display.print(F(" dB*"));

    // LINIA 3: Termika (Różnica)
    display.setCursor(0, 27);
    display.print(F("Air:")); display.print((int)temp_air); display.print(F("C "));
    display.print(F("Mur:")); display.print((int)temp_wall); display.print(F("C"));

    // LINIA 4: Status
    display.setCursor(0, 40);
    display.print(F("Stat: ")); display.println(status_msg);

    // LINIA 5: Bezpieczeństwo
    display.setCursor(0, 53);
    if(smoke > 100) { display.print(F("Dym: ")); display.print(smoke); }
    else { 
       display.print(F("Hałas Zew: ")); display.print(mic_street); // Pokazujemy hałas ulicy
    }
  }

  display.display();


  hc06.println("[");

  hc06.print("\"temp_air\":\""); hc06.print(temp_air); hc06.println("\",");
  hc06.print("\"temp_wall\":\""); hc06.print(temp_wall); hc06.println("\",");
  hc06.print("\"wilg_pow\":\""); hc06.print(hum_air); hc06.println("\",");
  hc06.print("\"mech\":\""); hc06.print(soil_percent); hc06.println("\",");
  hc06.print("\"woda\":\""); hc06.print(tank_percent); hc06.println("\",");
  hc06.print("\"dym\":\""); hc06.print(smoke); hc06.println("\",");
  hc06.print("\"ogien\":\""); hc06.print(fire); hc06.println("\",");
  // hc06.print("\"tlumienie\":\""); hc06.print(insulation_diff); hc06.println("\",");
  // hc06.print("\"zawor\":\""); hc06.print(valve_state ? "1" : "0"); hc06.println("\""); // Ostatni element bez przecinka


  hc06.println("]");
  
  delay(1500); 
}