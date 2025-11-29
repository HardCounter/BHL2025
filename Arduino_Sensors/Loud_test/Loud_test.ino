#define MIC_PIN A0

// Wartość "ciszy" dla większości modułów to połowa zakresu (512).
// Jeśli u Ciebie wykres jest wyżej/niżej, zmień tę liczbę.
int poziomCiszy = 512; 

// Tu wpisz wartość, przy której chcesz, żeby działo się "KLASK"
// Będziesz widział tę linię na wykresie. Dostosuj ją po testach!
int progAlarmu = 530; 

void setup() {
  Serial.begin(9600);
}

void loop() {
  int odczyt = analogRead(MIC_PIN);

  // Wypisujemy dane w formacie, który Ploter rozumie jako 3 osobne linie
  // Format: "Zmienna1:Wartość1, Zmienna2:Wartość2, Zmienna3:Wartość3"
  
  Serial.print("Mikrofon:");
  Serial.print(odczyt);
  
  Serial.print(","); 
  
  Serial.print("Prog_Alarmu:");
  Serial.print(progAlarmu);
  
  Serial.print(",");
  
  Serial.print("Poziom_Ciszy:");
  Serial.println(poziomCiszy);

  // Bardzo małe opóźnienie, żeby wykres był płynny, ale nie uciekał
  delay(200);
}