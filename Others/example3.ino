// https://steemit.com/utopian-io/@drencolha/making-a-weight-scale-with-the-hx711-module-hx711-arduino-library-and-a-load-cell-tutorial

#include <HX711.h>

HX711 scale;  // Initializes library functions.
int calibration_factor = -100000; // Defines calibration factor we'll use for calibrating.

void setup()
{
  Serial.begin(9600);   // Starts serial communication in 9600 baud rate.

  Serial.println("Initializing scale calibration.");  // Prints user commands.
  Serial.println("Please remove all weight from scale.");
  Serial.println("Place known weights on scale one by one.");
  Serial.println("Press '+' to increase calibration factor by 1000");
  Serial.println("Press '-' to decrease calibration factor by 1000");
  Serial.println("Press 'C' for tare");

  scale.begin(A1, A0);   // Initializes the scaling process.
  // Used pins are A0 and A1.

  scale.set_scale();
  scale.tare();          // Resets the scale to 0.
}


void loop()
{


  scale.set_scale(calibration_factor);  // Adjusts the calibration factor.

  Serial.print("Reading: ");            // Prints weight readings in .2 decimal kg units.
  Serial.print(scale.get_units(), 2);
  Serial.println(" kg");
  Serial.print("Calibration factor: "); // Prints calibration factor.
  Serial.println(calibration_factor);

  if (Serial.available()) // Calibration process starts if there is a serial connection present.
  {
    char temp = Serial.read();    // Reads users keyboard inputs.

    if (temp == '+')     // Increases calibration factor by 10 if '+' key is pressed.
      calibration_factor += 1000;
    else if (temp == '-') // Decreases calibration factor by 10 if '-' key is pressed.
      calibration_factor -= 1000;
    else if (temp == 'c' || temp == 'C')
      scale.tare();                     // Reset the scale to zero if 'C' key is pressed.
  }

  scale.power_down();    // Puts the scale to sleep mode for 5 seconds.
  delay(5000);
  scale.power_up();
}
