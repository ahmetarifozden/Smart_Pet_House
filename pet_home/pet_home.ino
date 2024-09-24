#include <HX711.h>

// ====================
// KULLANICI AYARLARI
// ====================
#define IS_VERBOSE                    true
#define IS_WIFI_ACTIVE                false
#define IS_WEIGHT_CALIBRATION_ACTIVE  false
#define WORK_DELAY_MS                 1500
#define RETRY_CONNECTION_TIME         5

// Kalibrasyon tuşları
#if IS_WEIGHT_CALIBRATION_ACTIVE
  #define INCREASE_25_BUTTON          'q'
  #define DECREASE_25_BUTTON          'w'
  #define INCREASE_250_BUTTON         'e'
  #define DECREASE_250_BUTTON         'r'
  #define TARE_BUTTON                 't'
  #define DEFAULT_BUTTON              'y'
  #define EXIT_BUTTON                 'u'
  #define CALIBRATION_STEP            25
#endif

// ====================
// PINLER
// ====================
#define WATER_SENSOR_PIN              A3
#define WEIGHT_SENSOR_FIRST_PIN       A1
#define WEIGHT_SENSOR_SECOND_PIN      A0

// ====================
// WIFI VE API AYARLARI
// ====================
#define WIFI_SSID                     "iuc.misafir"
#define WIFI_PASSWORD                 "iuc.1453"
#define TEAM_SPEAK_SERVER_IP          "184.106.153.149"
#define TEAM_SPEAK_API_KEY            "M3PNVOVHXJFJ9H2A"

// ====================
// GLOBAL DEĞİŞKENLER
// ====================
double water_level = 0;
double food_level = 0;

HX711 scale;
int calibration_factor = -19750;
int default_calibration_factor = calibration_factor;
bool connection_status = false;
bool data_sent = false;
int retry_time = 0;

// ====================
// YARDIMCI FONKSİYONLAR
// ====================
void connect_to_wifi() {
  #if IS_WIFI_ACTIVE
    Serial.println("AT+CWJAP=\"" + String(WIFI_SSID) + "\",\"" + WIFI_PASSWORD + "\"");
    delay(5000);
  #endif
}

bool is_wifi_connected() {
  #if IS_WIFI_ACTIVE
    Serial.println("AT+CWJAP?");
    delay(1000);
    return Serial.find("OK");
  #else
    return false;
  #endif
}

double get_water_level(int pin) {
  int raw_value = analogRead(pin);
  raw_value = constrain(raw_value, 0, 650);
  return map(raw_value, 0, 650, 0, 100) / 100.0;
}

double get_food_level() {
  double raw_weight = -scale.get_units(5);
  return constrain(raw_weight, 0, 100) / 100.0;
}

bool send_data() {
  #if IS_WIFI_ACTIVE
    String data = "GET https://api.thingspeak.com/update?api_key=" + String(TEAM_SPEAK_API_KEY);
    data += "&field1=" + String(water_level);
    data += "&field2=" + String(food_level);
    Serial.println("AT+CIPSEND=" + String(data.length()));
    delay(2000);
    if (Serial.find(">")) {
      Serial.println(data);
      Serial.println("AT+CIPCLOSE=0");
      return true;
    } else {
      Serial.println("AT+CIPCLOSE=0");
      return false;
    }
  #else
    return false;
  #endif
}

void setup() {
  Serial.begin(9600);
  scale.begin(WEIGHT_SENSOR_FIRST_PIN, WEIGHT_SENSOR_SECOND_PIN);
  scale.set_scale(calibration_factor);
  scale.tare();
}

void loop() {
  water_level = get_water_level(WATER_SENSOR_PIN);
  food_level = get_food_level();

  if (is_wifi_connected()) {
    connection_status = true;
    data_sent = send_data();
  } else {
    connection_status = false;
    if (retry_time == 0) {
      connect_to_wifi();
      retry_time = RETRY_CONNECTION_TIME;
    } else {
      retry_time--;
    }
  }

  if (IS_VERBOSE) {
    Serial.print(connection_status ? "(ONLINE)" : "(OFFLINE)");
    Serial.print(" Water: " + String(water_level) + "L");
    Serial.print(" | Food: " + String(food_level) + "kg");
    Serial.println(data_sent ? " Data Sent" : " Data Not Sent");
  }

  scale.power_down();
  delay(WORK_DELAY_MS);
  scale.power_up();
}
