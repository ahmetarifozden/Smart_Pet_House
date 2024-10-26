#define SSID "WiFi_Ismi"            // WiFi ağ ismi
#define PASSWORD "WiFi Sifresi"     // WiFi şifresi
#define THINGSPEAK_IP "184.106.153.149"    // Thingspeak IP adresi
#define API_KEY "64T0OS3R1OEAYUML"  // Thingspeak API key

float temperature;  // Sıcaklık değeri

void setup() {
  Serial.begin(115200);  // ESP modülünün baud hızı 115200 olduğundan seri haberleşme de bu hızda başlatılıyor.
  delay(3000);           // ESP ile bağlantı kurulabilmesi için 3 saniye bekleniyor.

  if (sendATCommand("AT", "OK")) {  // ESP ile bağlantı kontrolü
    Serial.println("ESP bağlantısı başarılı");

    // WiFi modunu STA (station) olarak ayarla
    if (sendATCommand("AT+CWMODE=1", "OK")) {
      Serial.println("ESP STA moduna alındı.");

      // WiFi ağına bağlan
      String connectCommand = String("AT+CWJAP=\"") + SSID + "\",\"" + PASSWORD + "\"";
      if (sendATCommand(connectCommand, "OK", 5000)) {
        Serial.println("WiFi ağına bağlanıldı.");
      } else {
        Serial.println("WiFi ağına bağlanılamadı.");
      }
    } else {
      Serial.println("ESP STA moduna ayarlanamadı.");
    }
  } else {
    Serial.println("ESP bağlantısı başarısız.");
  }
}

void loop() {
  // Sıcaklık verisini oku (örnek: analog değer 9.31'e bölünüyor)
  temperature = analogRead(A0) / 9.31;
  Serial.println(temperature);

  // Sıcaklığı Thingspeak'e gönder
  sendTemperatureToThingSpeak(temperature);

  // Dakikada bir güncelleme yapılması için 1 dakika bekle
  delay(60000);
}

// Thingspeak'e sıcaklık verisini gönderme fonksiyonu
void sendTemperatureToThingSpeak(float temp) {
  // Thingspeak'e bağlan
  String connectCommand = "AT+CIPSTART=\"TCP\",\"" + String(THINGSPEAK_IP) + "\",80";
  if (!sendATCommand(connectCommand, "OK", 1000)) {
    Serial.println("Thingspeak'e bağlanılamadı.");
    return;
  }

  // Thingspeak'e sıcaklık verisini gönderme komutu
  String dataCommand = "GET /update?key=" + String(API_KEY) + "&field1=" + String((int)temp) + "\r\n\r\n";
  int dataLength = dataCommand.length() + 2;

  // Gönderilecek veri uzunluğunu belirt ve veri gönder
  String lengthCommand = "AT+CIPSEND=" + String(dataLength);
  if (sendATCommand(lengthCommand, ">", 1000)) {
    Serial.print(dataCommand);  // Veriyi gönder
    Serial.print("\r\n\r\n");
    Serial.println("Sıcaklık verisi gönderildi.");
  } else {
    Serial.println("Veri gönderilemedi, bağlantı kapatılıyor.");
    sendATCommand("AT+CIPCLOSE", "OK");
  }
}

// ESP8266'ya AT komutu gönderme fonksiyonu
bool sendATCommand(String command, String expectedResponse, int timeout = 2000) {
  Serial.println(command);  // AT komutunu ESP modülüne gönder
  unsigned long startTime = millis();
  while (millis() - startTime < timeout) {
    if (Serial.find(expectedResponse.c_str())) {  // Beklenen cevap alındı mı?
      return true;
    }
  }
  return false;
}
