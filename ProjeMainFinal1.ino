// Buton ve LED/Buzzer pin tanımları
const int buttonPin = 5;   // Butonun bağlı olduğu pin numarası
const int ledPin = 12;     // LED'in bağlı olduğu pin numarası
const int buzzerPin = 8;   // Buzzer'ın bağlı olduğu pin numarası

#define trigPin 7
#define echoPin 6
#define buzzerPin 8


// WiFi ve LED pin tanımları
#define ag_ismi "azim123"
#define ag_sifresi "12345678"

int alarm = 0;

int RedLED = 4;
int GreenLED = 3;
int YellowLED = 2;

int AlarmOnLED = 12;
int AlarmOffLED = 11;

// Değişkenler
int buttonState = 0;       // Butonun durumunu okuyan değişken
String name = "deneme";      // İsim değişkeni tanımlama

void setup() {
  // Buton, LED ve Buzzer pinlerini başlatma
  pinMode(ledPin, OUTPUT);
  pinMode(buzzerPin, OUTPUT);
  pinMode(buttonPin, INPUT);

  // LED pinlerini başlatma
  pinMode(RedLED, OUTPUT);
  pinMode(GreenLED, OUTPUT);
  pinMode(YellowLED, OUTPUT);

  pinMode(AlarmOnLED, OUTPUT);
  pinMode(AlarmOffLED, OUTPUT);

  // Seri iletişimi başlatma
  Serial.begin(115200); 
  delay(1000); // ESP ile iletişim kurmak için biraz bekle
  Serial.println("AT"); 

  if (Serial.find("OK")) { 
    Serial.println("AT+CWMODE=1"); // ESP modülünü STA moduna ayarla
    delay(2000);
    String baglantiKomutu = String("AT+CWJAP=\"") + ag_ismi + "\",\"" + ag_sifresi + "\"";
    Serial.println(baglantiKomutu);
    delay(5000);
  }

  Serial.println("AT+CIPMUX=1");
  delay(200);
  Serial.println("AT+CIPSERVER=1,80");
  delay(1000);

  pinMode(trigPin, OUTPUT); // Trig pinini çıkış olarak ayarla
  pinMode(echoPin, INPUT);  // Echo pinini giriş olarak ayarla
  pinMode(buzzerPin, OUTPUT); // Buzzer pinini çıkış olarak ayarla
}

void loop() {
  long duration;
  int distance;

  // Trig pinine 10us yüksek sinyal gönder
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  // Echo pininden yüksek sinyali bekle ve süresini ölç
  duration = pulseIn(echoPin, HIGH);

  // Mesafeyi hesapla (ses hızı 340 m/s veya 29.1 us/cm)
  distance = duration * 0.034 / 2;

  // Mesafe belirli bir değerin altına düşerse buzzer'ı çal
  if (distance < 10) { // 10 cm'den daha yakınsa
    alarm = 1;
    name = "active";
  } else {
    digitalWrite(buzzerPin, LOW); // Buzzer'ı kapat
  }

  if (alarm == 1) {
    digitalWrite(buzzerPin, HIGH); // Buzzer'ı aktif et
    digitalWrite(AlarmOffLED, LOW);
    digitalWrite(AlarmOnLED, HIGH);
  }else if (alarm == 0) {
    digitalWrite(buzzerPin, LOW); // Buzzer'ı kapat
    digitalWrite(AlarmOffLED, HIGH);
    digitalWrite(AlarmOnLED, LOW);
  }

  delay(500); // 500ms bekle

  // Butonun durumunu oku
  buttonState = digitalRead(buttonPin);

  // Butona basılıp basılmadığını kontrol et. Eğer basılıysa buttonState HIGH olur:
  if (buttonState == HIGH) {
    // LED'i aç
    digitalWrite(ledPin, HIGH);
    // Buzzer'ı aç
    digitalWrite(buzzerPin, HIGH);
  } else {
    // LED'i kapat
    digitalWrite(ledPin, LOW);
    // Buzzer'ı kapat
    digitalWrite(buzzerPin, LOW);
  }

  // Seri port üzerinden gelen veriyi kontrol et
  if (Serial.available() > 0) {
    if (Serial.find("+IPD,")) {
      String metin = "<html><body>";
      metin += "<form action=\"/submit\" method=\"GET\">";
      metin += "<label for=\"veri\">Veri:</label><br>";
      metin += "<input type=\"text\" id=\"veri\" name=\"veri\" value=\"" + name + "\"><br><br>";
      metin += "<input type=\"submit\" value=\"Gonder\">";
      metin += "</form>";
      metin += "<br><a href=\"?pin=red\"><button type='button'>Red LED</button></a>"; 
      metin += "<br><a href=\"?pin=green\"><button type='button'>Green LED</button></a>";
      metin += "<br><a href=\"?pin=yellow\"><button type='button'>Yellow LED</button></a>"; 
      metin += "<br><a href=\"?pin=alarmOff\"><button type='button'>Alarm Off</button></a>";      
      metin += "</body></html>";

      String cipsend = "AT+CIPSEND=0," + String(metin.length()) + "\r\n";
      Serial.print(cipsend);
      delay(500);
      Serial.print(metin);
      delay(100); // Gönderme işleminden sonra biraz bekle
      Serial.println("AT+CIPCLOSE=0");
      led_yakma();
    }
  }
}

void led_yakma() {
  String gelen = "";
  char serialdenokunan;
  while (Serial.available() > 0) {
    serialdenokunan = Serial.read();
    gelen += serialdenokunan;
  }
  Serial.println(gelen);

  // Form verilerini işleme
  if (gelen.indexOf("GET /submit?veri=") > 0) {
    int startIndex = gelen.indexOf("veri=") + 5;
    int endIndex = gelen.indexOf(" ", startIndex);
    String formVeri = gelen.substring(startIndex, endIndex);
    formVeri.replace("%20", " "); // Boşlukları değiştir
    Serial.print("Gelen veri: ");
    Serial.println(formVeri);

    // Gelen veriyi name değişkenine ata
    name = formVeri;
  }

  // LED kontrol
  if (gelen.indexOf(":GET /?pin=red") > 0) {
    digitalWrite(RedLED, HIGH);
    digitalWrite(GreenLED, LOW);
    digitalWrite(YellowLED, LOW);
  }
  if (gelen.indexOf(":GET /?pin=green") > 0) {
    digitalWrite(RedLED, LOW);
    digitalWrite(GreenLED, HIGH);
    digitalWrite(YellowLED, LOW);
  }
  if (gelen.indexOf(":GET /?pin=yellow") > 0) {
    digitalWrite(RedLED, LOW);
    digitalWrite(GreenLED, LOW);
    digitalWrite(YellowLED, HIGH);
  }

  if (gelen.indexOf(":GET /?pin=alarmOff") > 0) {
    alarm = 0;
    name = "active";
  }
}
