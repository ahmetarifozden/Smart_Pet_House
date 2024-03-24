/*
  Project GitLab Link
    - https://gitlab.com/msaidbilgehan/home-pet-project

  TeamSpeak Public Channel
    - https://thingspeak.com/channels/1067911
      - Field 1 for Water Level Information, Field 2 for Food Level Information
        GET https://api.thingspeak.com/update?api_key=M3PNVOVHXJFJ9H2A&field1=0&field2=0

  TeamSpeak API
    - https://community.thingspeak.com/forum/thingspeak-projects/esp8266-arduino-ide-and-thingspeak-with-webserver-for-debugging/
    - https://community.blynk.cc/t/solved-thingspeak-support/379/13
    - https://create.arduino.cc/projecthub/TechnoFabrique/diy-wifi-light-sensor-a80055

  Water Sensor:
    - https://create.arduino.cc/projecthub/omer-beden/basic-water-sensor-190c7a

  Wifi Sensor:
    - https://maker.robotistan.com/esp8266-ile-iot-dersleri-1-esp8266-modulunu-guncelleme/
    - https://maker.robotistan.com/esp8266-dersleri-2-thingspeake-sicaklik-yollama/
    - https://maker.robotistan.com/esp8266-dersleri-3-at-komutlarinin-kullanilmasi/
    - https://maker.robotistan.com/esp8266-dersleri-4-oda-sicakligini-tweet-atma/
    - http://www.pridopia.co.uk/pi-doc/ESP8266ATCommandsSet.pdf
    - https://maker.robotistan.com/arduino-esp8266-kullanimi/

  Weight Sensor
    - https://steemit.com/utopian-io/@drencolha/making-a-weight-scale-with-the-hx711-module-hx711-arduino-library-and-a-load-cell-tutorial
    - https://github.com/bogde/HX711
    - https://create.arduino.cc/projecthub/electropeak/digital-force-gauge-weight-scale-w-loadcell-arduino-7a7fd5

*/

// ====================
// IMPORTS
// ====================
#include <HX711.h>


// ====================
// DEVELOPER VARIABLES
// ====================
#define IS_VERBOSE                      true              // To get output to the Serial Monitor
#define IS_WIFI_ACTIVE                  false              // To activate wifi
#define IS_WEIGHT_CALIBRATIN_ACTIVATED  false              // To calibrate weight sensor
#define G_WORK_DELAY                    1500              // Global Delay Time for Get Environmental Information ans Send it to TeamSpeak Server
#define RETRY_CONNECTION_TIME 5

#if IS_WEIGHT_CALIBRATIN_ACTIVATED
  // Serial Activated (User Input Variable)
  #define MORE_25_BUTTON                'w'
  #define LESS_25_BUTTON                's'
  #define MORE_250_BUTTON               'e'
  #define LESS_250_BUTTON               'd'
  #define TARE_BUTTON                   't'
  #define DEFAULT_BUTTON                'd'
  #define EXIT_BUTTON                   'q'
  #define CALIBRATION_FACTOR_CHANGE     25
#endif


// ====================
// PIN CONFIGURATIONS
// ====================
#define WATER_SENSOR_PIN                A3                // Attach Water sensor to Arduino Analog pin 3 - A3
#define WEIGHT_SENSOR_FIRST_PIN         A1                // Attach Weight sensor to Arduino Analog pin 1 - A1
#define WEIGHT_SENSOR_SECOND_PIN        A0                // Attach Weight sensor to Arduino Analog pin 0 - A0


// ====================
// GLOBAL VARIABLES
// ====================
#define AP_NAME                         "KUTAY"           // Wifi Name which we will connect
#define AP_PASSWORD                     "kutayyaman123"   // Wifi Password which we will connect

#define TEAM_SPEAK_SERVER_IP            "184.106.153.149" // thingspeak.com server IP adress
#define TEAM_SPEAK_API_KEY              "M3PNVOVHXJFJ9H2A"         // API KEY taken from teamspeak

/*
// PreDefined Limits for Sending Information
#define WATER_LEVEL_LOWER_LIMIT         500
#define FOOD_LEVEL_LOWER_LIMIT          500
*/

// Food and Water Level Global (Shared) Variables
double g_water_level                    = 0;
double g_food_level                     = 0;


// Initializes HX711 library functions
// Optimum walue for 1kg to get -100 value is -19750
HX711 scale;
int calibration_factor                  = -19750; // Defines calibration factor we'll use for calibrating.
int default_calibration_factor          = calibration_factor;

// Wifi and Connection Status Global (Shared) Variables
bool connection_status                  = false;
bool is_sended                          = false;
int no_connection_work_time             = 0;

#if IS_WEIGHT_CALIBRATIN_ACTIVATED
  // Serial Activated (User Input Variable)
  char user_input                       = "";
#endif

// ====================
// CUSTOM FUNCTIONS
// ====================
void automated_scenario(int water_sensor_pin){
  while(true){
    bool is_sended = false;

    g_water_level = get_water_level(water_sensor_pin, false);
    g_food_level = get_food_level(5, false);

    if( is_connected_to_ap() ){
      connection_status = true;
      is_sended = send_information();
    }
    else{
      if(no_connection_work_time == 0){
        #if IS_WIFI_ACTIVE
          Serial.println("Access Point connection down, reconnecting...");
        #endif

        connect_to_ap();
        if( is_connected_to_ap() ){
          #if IS_WIFI_ACTIVE
            Serial.print("Access Point '");
            Serial.print(String(AP_NAME));
            Serial.println("' connected...");
          #endif

          connection_status = true;
          is_sended = send_information();
        }
        else{
          connection_status = false;
        }
        no_connection_work_time = RETRY_CONNECTION_TIME;
      }
      else{
        no_connection_work_time -= 1;
      }
    }

    #if IS_VERBOSE
      if(connection_status){
        Serial.print("(ONLINE  :: ");
      }
      else{
        Serial.print("(OFFLINE :: ");
      }
      if(is_sended){
        Serial.print("SERVER_ON)\t");
      }
      else{
        Serial.print("SERVER_OFF)\t");
      }

      Serial.print("Water Level: ");
      Serial.print(String(g_water_level / 100.0));
      Serial.print(" lt");
      Serial.print(" | Food Level: ");
      Serial.print(String(g_food_level / 100.0));
      Serial.println(" kg");
    #endif

    #if G_WORK_DELAY > 1000
      scale.power_down();			        // put the ADC in sleep mode
    #endif
    delay(G_WORK_DELAY);
    #if G_WORK_DELAY > 1000
      scale.power_up();
    #endif
  }
}


void calibrate_weight_sensor(){
  #if IS_WEIGHT_CALIBRATIN_ACTIVATED
    scale.set_scale();
    scale.tare();          // Resets the scale to 0.
    // Calibration process starts if there is a serial connection present.
    Serial.println("Weight Sensor Calibration started.");
    while(user_input != EXIT_BUTTON){
      // Reads users keyboard inputs.
      user_input = Serial.read();

      // Increases calibration factor by CALIBRATION_FACTOR_CHANGE if MORE_BUTTON key is pressed.
      if (user_input == MORE_25_BUTTON){
        calibration_factor += CALIBRATION_FACTOR_CHANGE;
        scale.set_scale(calibration_factor); //Adjust to this calibration factor
      }

      // Decreases calibration factor by CALIBRATION_FACTOR_CHANGE if LESS_BUTTON key is pressed.
      else if (user_input == LESS_25_BUTTON){
        calibration_factor -= CALIBRATION_FACTOR_CHANGE;
        scale.set_scale(calibration_factor); //Adjust to this calibration factor
      }

      // Increases calibration factor by CALIBRATION_FACTOR_CHANGE if MORE_BUTTON key is pressed.
      if (user_input == MORE_250_BUTTON){
        calibration_factor += CALIBRATION_FACTOR_CHANGE * 10;
        scale.set_scale(calibration_factor); //Adjust to this calibration factor
      }

      // Decreases calibration factor by CALIBRATION_FACTOR_CHANGE if LESS_BUTTON key is pressed.
      else if (user_input == LESS_250_BUTTON){
        calibration_factor -= CALIBRATION_FACTOR_CHANGE * 10;
        scale.set_scale(calibration_factor); //Adjust to this calibration factor
      }

      // Reset the scale to zero if TARE_BUTTON key is pressed.
      else if (user_input == TARE_BUTTON){
        scale.set_scale(calibration_factor); //Adjust to this calibration factor
        scale.tare();
      }

      // Reset the calibration factor to zero if ZERO_BUTTON key is pressed.
      else if (user_input == DEFAULT_BUTTON){
        calibration_factor = default_calibration_factor;
        scale.set_scale(calibration_factor); //Adjust to this calibration factor
      }

      g_food_level = get_raw_food_level(5);

      Serial.println("Calibration Factor:\t" + String(calibration_factor));
      Serial.println("Weight:\t\t" + String(g_food_level));
      delay(700);
    }
  #else
    scale.set_scale(calibration_factor); //Adjust to this calibration factor
    scale.tare();
  #endif
}


double get_water_level(int water_sensor_pin, bool is_persentage){
  // get_raw_water_level will return 0 to 650 range because of sensor output
  double water_level = double(get_raw_water_level(water_sensor_pin));

  // We limit our range 0 to 650
  water_level = double(constrain(water_level, 0, 650));
  water_level = double(map(water_level, 0, 650, 0, 100));

  if(!is_persentage){
    water_level = water_level / 100.0;
  }

  return water_level;
}


int get_raw_water_level(int water_sensor_pin){
  return analogRead(water_sensor_pin);  // Incoming analog signal read and appointed sensor
}


double get_food_level(int average, bool is_kg){
  // get_raw_food_level will return 0 to -100 range, empt is 0, 1kg is -100
  double raw_weight = get_raw_food_level(average);

  // Negative will be possitive
  raw_weight = -raw_weight;

  // We limit our range with 0 to 100
  raw_weight = double(constrain(int(raw_weight), 0, 100));

  if(is_kg){
    // To get kg, we divide 100.0
    raw_weight = raw_weight / 100.0;
  }

  return raw_weight;
}


double get_raw_food_level(int average){
  return double(scale.get_units(average));  // Incoming analog signal read and appointed sensor
}


bool send_information(){
  #if IS_WIFI_ACTIVE
    /*
      Connecting to thingspeak servers,
      with "AT+CIPSTART" command, we will take permission from servers,
      and lastly, TCP connection establishing with port 80
    */
    Serial.println(String("AT+CIPSTART=\"TCP\",\"") + String(TEAM_SPEAK_SERVER_IP) + "\",80");

    /*
    String information =  "POST /update HTTP/1.1\n";
    information +=        "Host: api.thingspeak.com\n";
    information +=        "Connection: close\n";
    information +=        "X-THINGSPEAKAPIKEY: " + String(TEAM_SPEAK_API_KEY) + "\n";
    information +=        "Content-Type: application/x-www-form-urlencoded\n";

    // We need to updaye "Content-Length:" value with +32 length of message lenght
    information +=        "Content-Length:";
    //information +=        String(g_message.length() + 32);

    // API KEY of teamspeak here
    information +=        "\r\n\r\napi_key=";
    information +=        String(TEAM_SPEAK_API_KEY);

    // Here will come our fields for information of water and food level
    information += "&field1=";
    information += String(g_water_level);
    information += "&field2=";
    information += String(g_food_level);
    information += "\r\n\r\n";
    */

    // Here will come our information send command to server of TeamSpeak
    String information = "GET https://api.thingspeak.com/update?api_key=" + String(TEAM_SPEAK_API_KEY);
    // Here will come our fields for information of water and food level
    information += "&field1=";
    information += String(g_water_level);
    information += "&field2=";
    information += String(g_food_level);
    information += "\r\n\r\n";

    /*
      "AT+CIPSEND=" command will send our data, but firstly we need give length of data
    */
    Serial.print("AT+CIPSEND=");
    Serial.println(information.length());
    delay(2000);

    /*
      If we connected to server which we are going to send our data, output will be ">"
      Otherwise, close the connection on ESP
    */
    if( Serial.find(">") ){
      Serial.println(information);
      Serial.println("AT+CIPCLOSE=0");
      //Serial.println("AT+CIPCLOSE"); // Is this right command???

      delay(100);
      return true;
    }
    else{
      Serial.println("AT+CIPCLOSE=0");
      //Serial.println("AT+CIPCLOSE"); // Is this right command???
      return false;
    }
  #else
    return false;
  #endif
}


void connect_to_ap(){
  #if IS_WIFI_ACTIVE
    String connection_command = String("AT+CWJAP=\"") + AP_NAME + "\",\"" + AP_PASSWORD + "\"";
    Serial.println(connection_command);
    delay(5000);
  #endif
}


bool is_connected_to_ap(){
  #if IS_WIFI_ACTIVE
    String connection_command = String("AT+CWJAP?");
    Serial.println(connection_command);
    delay(1000);

    if( Serial.find("OK") ){
      return true;
    }
    return false;
  #else
    return false;
  #endif
}


String pin_print_formatting(int pin){
  switch(pin){
    case A0:
      return "A0";
    break;
    case A1:
      return "A1";
    break;
    case A2:
      return "A2";
    break;
    case A3:
      return "A3";
    break;
    case A4:
      return "A4";
    break;
    case A5:
      return "A5";
    break;

    default:
      return String(pin);
    break;
  }
}


void sensor_pins_output(int water_sensor_pin, int weight_sensor_pin_first, int weight_sensor_pin_second){
  #if IS_VERBOSE
    String print_water_sensor_pin         = pin_print_formatting(water_sensor_pin);
    String print_weight_sensor_pin_first  = pin_print_formatting(weight_sensor_pin_first);
    String print_weight_sensor_pin_second = pin_print_formatting(weight_sensor_pin_second);

    Serial.print("\nWATER SENSOR PIN: ");
    Serial.println(print_water_sensor_pin);

    Serial.print("WEIGHT SENSOR PINs: ");
    Serial.print(print_weight_sensor_pin_first);
    Serial.print(" | ");
    Serial.println(print_weight_sensor_pin_second);
    #if IS_WIFI_ACTIVE
      Serial.println("Wifi Initilized and Activated.");
    #endif

  #endif
}


void weight_module_initialization(int weight_sensor_first_pin, int weight_sensor_second_pin){
  /*
    How to calibrate your load cell
    Call set_scale() with no parameter.
    Call tare() with no parameter.
    Place a known weight on the scale and call get_units(10).
    Divide the result in step 3 to your known weight. You should get about the parameter you need to pass to set_scale().
    Adjust the parameter in step 4 until you get an accurate reading.
  */
  scale.begin(weight_sensor_first_pin, weight_sensor_second_pin);   // Initializes the scaling process.
  // Used pins are A0 and A1.

  scale.set_scale();
  scale.tare();          // Resets the scale to 0.
  #if IS_WEIGHT_CALIBRATIN_ACTIVATED
    Serial.println("Initializing scale calibration.");  // Prints user commands.
    Serial.println("Please remove all weight from scale.");
    Serial.println("Place known weights on scale one by one.");
    Serial.println("Press '" + String(MORE_25_BUTTON) + "' to increase calibration factor by " + String(CALIBRATION_FACTOR_CHANGE));
    Serial.println("Press '" + String(LESS_25_BUTTON) + "' to decrease calibration factor by " + String(CALIBRATION_FACTOR_CHANGE));
    Serial.println("Press '" + String(MORE_250_BUTTON) + "' to increase calibration factor by " + String(CALIBRATION_FACTOR_CHANGE * 10));
    Serial.println("Press '" + String(LESS_250_BUTTON) + "' to decrease calibration factor by " + String(CALIBRATION_FACTOR_CHANGE * 10));
    Serial.println("Press '" + String(TARE_BUTTON) + "' for tare");
    Serial.println("Press '" + String(DEFAULT_BUTTON) + "' for change calibration factor to default");
    Serial.println("Press '" + String(EXIT_BUTTON) + "' for tare and finish the calibration");
    calibrate_weight_sensor();
  #endif
  scale.set_scale(calibration_factor);
  scale.tare();          // Resets the scale to calibration_factor.
}


void wifi_module_initialization(){
    Serial.println("AT"); // Check if ESP Module is connected to Arduino
    delay(3000);          // Wait 3 sec for answer

    /*
      If ESP module returns "OK" for "AT" command, we are connected! Time to change the mode of wifi module.
        AT+CWMODE=1 -> STA mode which is for connecting to other access points (or wifi)
        AT+CWMODE=2 -> Access Point (AP) which is for beign a wifi
        AT+CWMODE=3 -> For using both 1 and 2 mode actively
    */
    // Waiting until module getting ready
    while( !Serial.find("OK") ){
      #if IS_VERBOSE
        Serial.println("ESP8266 Not Found. Retrying...");
      #endif
      Serial.println("AT");
      delay(3000);
    }
    /*
    int i;
    for(i = 20; i > 0; i--){
      Serial.println("Last " + String(i) + " seconds to start");
      delay(1000);
    }
    */

    Serial.println("AT+CWMODE=1");
    #if IS_VERBOSE
      Serial.println("Client mode activated and Setup complated.");
    #endif
}


// ==================
// BUILT-IN FUNCTIONS
// ==================
void setup() {
  #if IS_WIFI_ACTIVE
    Serial.begin(115200); // Communication started with 115200 baud for ESP module (Wifi)
    wifi_module_initialization();
  #else
    Serial.begin(9600);
  #endif
  weight_module_initialization(WEIGHT_SENSOR_FIRST_PIN, WEIGHT_SENSOR_SECOND_PIN);

  sensor_pins_output(WATER_SENSOR_PIN, WEIGHT_SENSOR_FIRST_PIN, WEIGHT_SENSOR_SECOND_PIN);
  delay(500);
}


void loop() {
  automated_scenario(WATER_SENSOR_PIN);
}
