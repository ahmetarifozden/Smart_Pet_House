#include <HX711.h>

HX711 scale;  // HX711 kütüphanesini başlatıyoruz.
int calibration_factor = -100000; // Kalibrasyon faktörü.

void setup() {
  Serial.begin(9600);  // Seri haberleşmeyi başlatıyoruz.

  // Kullanıcıya talimatlar veriyoruz.
  Serial.println("Tartı kalibrasyonu başlatılıyor.");
  Serial.println("Lütfen tartıdan tüm ağırlığı kaldırın.");
  Serial.println("Tartıya bilinen ağırlıklar yerleştirin.");
  Serial.println("'+' ile kalibrasyon faktörünü 1000 artırın.");
  Serial.println("'-' ile kalibrasyon faktörünü 1000 azaltın.");
  Serial.println("'C' ile sıfırlama yapın.");

  scale.begin(A1, A0);  // Tartı işlemi için kullanılan pinleri başlatıyoruz.
  scale.set_scale();    // Tartıyı başlatıyoruz.
  scale.tare();         // Tartıyı sıfırlıyoruz.
}

void loop() {
  // Kalibrasyon faktörünü uyguluyoruz.
  scale.set_scale(calibration_factor);

  // Tartım sonucunu ekrana yazdırıyoruz.
  Serial.print("Okunan değer: ");
  Serial.print(scale.get_units(), 2);  // Ağırlığı iki basamaklı olarak gösteriyoruz.
  Serial.println(" kg");

  // Mevcut kalibrasyon faktörünü ekrana yazdırıyoruz.
  Serial.print("Kalibrasyon faktörü: ");
  Serial.println(calibration_factor);

  // Eğer seri porttan bir veri gelirse kalibrasyon işlemini başlatıyoruz.
  if (Serial.available()) {
    char input = Serial.read();  // Kullanıcı girişini okuyoruz.
    handleCalibrationInput(input);  // Girişe göre kalibrasyon işlemi yapıyoruz.
  }

  // Tartıyı uyku moduna alıyoruz ve 5 saniye bekliyoruz.
  scale.power_down();
  delay(5000);
  scale.power_up();
}

// Kalibrasyon işlemlerini yöneten fonksiyon
void handleCalibrationInput(char input) {
  switch (input) {
    case '+':
      calibration_factor += 1000;  // '+' basıldığında kalibrasyon faktörünü 1000 artırıyoruz.
      break;
    case '-':
      calibration_factor -= 1000;  // '-' basıldığında kalibrasyon faktörünü 1000 azaltıyoruz.
      break;
    case 'c':
    case 'C':
      scale.tare();  // 'C' veya 'c' basıldığında tartıyı sıfırlıyoruz.
      Serial.println("Tartı sıfırlandı.");
      break;
    default:
      Serial.println("Geçersiz giriş.");
      break;
  }
}
