#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "HX711.h"

//######################LCD CONFIGURATION########################################
//LCD WIRING DIAGRAM
// I2C LCD	    ESP32
// GND	        GND   
// VCC	        VIN   
// SDA	        GPIO  21
// SCL	        GPIO  22

// LCD constants
#define LCD_ADDRESS 0x27  
#define LCD_COLS 16
#define LCD_ROWS 4 //from third row, the column numbering starts from -4

//#####################LOADCELL CONFIGURATION####################################
// load cell wiring diagram
// Load Cell		  HX711	  HX711	  ESP32
// Red (E+)		    E+	    GND		  GND
// Black (E-)		  E-		  DT		  GPIO 16
// White (A-)		  A-		  SCK		  GPIO 4
// Green (A+)		  A+		  VCC		  3.3V

const int LOADCELL_DOUT_PIN = 16;
const int LOADCELL_SCK_PIN = 4;

//#####################ULTRASONIC SENSOR#########################################
//Ultrasonic sensor wiring diagram
// Ultrasonic Sensor	  ESP32
// VCC	                VIN
// Trig	                GPIO 5
// Echo	                GPIO 18
// GND	                GND

// Ultrasonic sensor pins
const int trigPin = 5;
const int echoPin = 18;

// Constants for BMI ranges
const float BMI_UNDERWEIGHT_MAX = 18.5;
const float BMI_NORMAL_MAX = 24.9; //healthy
const float BMI_OVERWEIGHT_MAX = 29.9;
const float MAX_HEIGHT = 7 * 0.3048;  // 1 foot = 0.3048 meters
// Constants for sound speed and conversion factors
#define SOUND_SPEED 0.035188  // Sound speed in cm/uS
#define CM_TO_INCH 0.393701  // Conversion factor from cm to inches



// HX711 and LCD instances
HX711 scale;
LiquidCrystal_I2C lcd(LCD_ADDRESS, LCD_COLS, LCD_ROWS);

// Ultrasonic sensor variables
long duration;
float distanceCm;

void setup() {
    // Initialize serial and I2C communication
    Serial.begin(115200);
    Wire.begin();

    // Initialize the LCD
    lcd.init();
    lcd.backlight();  // Turn on the backlight
    lcd.setCursor(0, 0);
    lcd.print("Remove weight");
    lcd.setCursor(0, 1);
    lcd.print("from load cell");
    lcd.setCursor(0, 2);

    lcd.setCursor(-4, 3);
    lcd.print("Tare time:");
    lcd.setCursor(8, 3);
    lcd.print(" sec");
    for(int i=5; i>=0; i--){
      lcd.setCursor(7, 3);
      lcd.print(i);
      delay(1000);
    }
    // Initialize HX711 scale
    scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
    scale.set_scale(-27.78);  // Calibration factor (adjust as needed)
    scale.tare();  // Reset to zero
    lcd.clear();
    lcd.print("** Tare done ** ");
    delay(1000);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Weight: ");  // Display HX711 readings
    lcd.setCursor(14, 0);
    lcd.print("kg");

    lcd.setCursor(0, 1);
    lcd.print("Height: ");
    lcd.setCursor(15, 1);
    lcd.print("m");

    lcd.setCursor(-4, 2);
    lcd.print("BMI: ");
    lcd.setCursor(7, 2);
    lcd.print("kg/m2");


    // Initialize ultrasonic sensor
    pinMode(trigPin, OUTPUT);  // Sets the trigPin as an Output
    pinMode(echoPin, INPUT);  // Sets the echoPin as an Input
}

void loop() {
    // Measure height with the ultrasonic sensor
    // Clears the trigPin, then sets to HIGH for 10 microseconds
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);

    // Measure echo duration and calculate distance in centimeters
    duration = pulseIn(echoPin, HIGH);
  
    // Convert height from cm to m  (/100 is for converting)
    float heightMeters = MAX_HEIGHT - (duration * SOUND_SPEED / (2 * 100));
    //if there is an error height measurement and it is showing a negative value then set the height to 0.0 meters
    if(heightMeters <= 0.0){
      heightMeters=0.0;
    }
    // HX711 readings for weight
    float weightKg = scale.get_units(10)/1000;  // Get average of 10 readings to convert grams to kg (/1000)
     //if there is an error weight measurement and it is showing a negative value then set the weight to 0.0 kg
    if(weightKg <= 0.0){
      weightKg=0.0;
    }
    

    float bmi;
    // Calculate BMI
    // if weight = 0.0 , then dividing by zero error is caused
    if(heightMeters > 0.0){
       bmi = weightKg / (heightMeters * heightMeters); //weight in kg / ( height in meters )^2 
    }
    else {
      bmi = 0.0;
      }

   

    // Display Weight in kg in first row of LCD
   lcd.setCursor(8, 0);
   lcd.print(weightKg, 1);
   

  // Display Height in m in second row of LCD
    lcd.setCursor(8,1);
    lcd.print(heightMeters, 1);


   // Display BMI in kg/m2 in fourth row of LCD
    lcd.setCursor(1, 2);
    lcd.print(bmi, 2);

  // Determine BMI category and display it
    lcd.setCursor(0, 3);
    if (bmi < BMI_UNDERWEIGHT_MAX) {
        lcd.print("Underweight");
    } else if (bmi <= BMI_NORMAL_MAX) {
        lcd.print("Healthy");
    } else if (bmi <= BMI_OVERWEIGHT_MAX) {
        lcd.print("Overweight");
    } else {
        lcd.print("Obese");
    }


}