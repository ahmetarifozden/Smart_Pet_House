#include <SoftwareSerial.h>       // SoftwareSerial kütüphanesi
#include <DHT.h>                  // DHT11 sensör kütüphanesi

#define DHTPIN 2                  // DHT11 sensörünün bağlı olduğu pin
#define DHTTYPE DHT11             // DHT11 sensörü tanımlaması
DHT dht(DHTPIN, DHTTYPE);         // DHT sensör nesnesi

String ssid = "Robotistan";        // WiFi ağ adı
String password = "fortinet";      // WiFi şifresi
String thingspeakIP = "184.106.153.149"; // Thingspeak IP adresi
String apiKey = "2F55993RWVDCTSUS"; // Thingspeak API anahtarı

int rxPin = 10;                    // ESP8266 RX pini
int txPin = 11;                    // ESP8266 TX pini

SoftwareSerial esp(rxPin, txPin);   // ESP8266 ile seri haberleşme

void setup() {
  Serial.begin(9600);              // Seri haberleşme başlat
  esp.begin(115200);               // ESP8266 ile haberleşme başlat
  dht.begin();                     // DHT sensörünü başlat
  Serial.println("Başlatıldı");

  // ESP8266 modülünün çalıştığını doğrula
  if (!sendCommand("AT", "OK")) {
    Serial.println("ESP8266 bulunamadı.");
    return;
  }

  // ESP8266'yı client moduna al
  if (!sendCommand("AT+CWMODE=1", "OK")) {
    Serial.println("ESP8266 client moduna ayarlanamadı.");
    return;
  }

  // WiFi ağına bağlan
  String wifiConnectCmd = "AT+CWJAP=\"" + ssid + "\",\"" + password + "\"";
  if (!sendCommand(wifiConnectCmd, "OK")) {
    Serial.println("WiFi ağına bağlanılamadı.");
    return;
  }

  Serial.println("WiFi ağına başarıyla bağlandı.");
}

void loop() {
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

  // Eğer sıcaklık veya nem okumada hata varsa döngüyü atla
  if (isnan(temperature) || isnan(humidity)) {
    Serial.println("Sıcaklık veya nem sensörü okunamadı.");
    return;
  }

  // Thingspeak'e bağlan
  if (!sendCommand("AT+CIPSTART=\"TCP\",\"" + thingspeakIP + "\",80", "OK")) {
    Serial.println("Thingspeak'e bağlanılamadı.");
    return;
  }

  // HTTP isteğini oluştur
  String data = "GET /update?api_key=" + apiKey + "&field1=" + String(temperature) + "&field2=" + String(humidity) + "\r\n\r\n";
  String sendLength = "AT+CIPSEND=" + String(data.length());

  // Veri uzunluğunu gönder ve veriyi gönder
  if (sendCommand(sendLength, ">")) {
    esp.print(data);
    Serial.println("Veri gönderildi: " + data);
  }

  // Bağlantıyı kapat
  sendCommand("AT+CIPCLOSE", "OK");

  // 1 dakika bekle
  delay(60000);
}

// ESP8266'ya komut gönderip cevap bekler
bool sendCommand(String command, String expectedResponse) {
  esp.println(command);
  if (esp.find(expectedResponse.c_str())) {
    return true;
  }
  return false;
}
