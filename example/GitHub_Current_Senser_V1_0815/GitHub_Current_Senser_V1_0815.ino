// ###################################################################################
// # Project: Power Consumption_System
// # Engineer:  Mic.Tsai
// # Date:  08 January 2020
// # Objective: Dev.board
// # Usage: ATMEL328P AVR
// ###################################################################################
// # Reference firmware: 2016-03-22 by Eadf (https://github.com/jrowberg/i2cdevlib)
// # I2C device class (I2Cdev) demonstration Arduino sketch for ADS1115 class
// ###################################################################################
/*
 SD card read/write
 * SD card attached to SPI bus as follows:
 ** MOSI - pin 11
 ** MISO - pin 12
 ** CLK - pin 13
 ** CS - pin 4
 * 

I2C interface
 ** SDA - A3
 ** SCK - A4
 * 

Mode select
 ** Button - 3 (Pull low resistor 10k)
 * 
 */

#include "ADS1115.h"

//For display 
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

//Declare 4x adc board as following I2C address.
ADS1115 adc0(0x48);

//Set the gain (PGA) with compared multiply
const float ads_print_Group1_Single_multiplier0 = 0.125000;


//Mode change declaration
const int  buttonPin = 3;    // the pin that the pushbutton is attached to

// Variables will change:
int buttonPushCounter = 0;   // counter for the number of button presses
int buttonState = 0;         // current state of the button
int lastButtonState = 0;     // previous state of the button

// -----------------------------------------------------------------------------
// Arduino-Setup
// -----------------------------------------------------------------------------

void setup() {   
   
    //I2Cdev::begin();  // join I2C bus
    Wire.begin();
    Serial.begin(9600); // initialize serial communication 
    
    Serial.println("Testing device connections...");
    Serial.println(adc0.testConnection() ? "ADS1115 connection successful" : "ADS1115 connection failed");
    
    // initialize ADS1115 16 bit A/D chip
    adc0.initialize(); 

/*
    // Single shot sampling
    adc0.setMode(ADS1115_MODE_SINGLESHOT);
*/
    // Continuous sampling
    adc0.setMode(ADS1115_MODE_CONTINUOUS);


    // Slow things down so that we can see that the "poll for conversion" code works
    adc0.setRate(ADS1115_RATE_128);


    // Set the gain (PGA) +/- 6.144V
    // Note that any analog input must be higher than –0.3V and less than VDD +0.3
//    adc0.setGain(ADS1115_PGA_0P256);


    //Display show...
    // initialize with the I2C addr 0x3C
    if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
    }
    
    // Clear the buffer.
    display.clearDisplay();

    // Display "8WA - Current Senser"
    display.setTextSize(2); display.setTextColor(WHITE); display.setCursor(0,0); display.println("PowerCUMP");
    display.setTextSize(1); display.setCursor(0,20); display.println("Current Senser v1");
    display.display();
    delay(1000);

    // Display "By Mic.Tsai - BU8"
    //display.clearDisplay();
    display.setTextSize(1); display.setTextColor(WHITE); 
    display.setCursor(60,40); display.println("Engineer:");
    display.setCursor(75,50); display.println("Mic.Tsai");
    display.display();
    delay(1000);
    display.clearDisplay(); 
    display.display();
 
    //Set PWM pin
    pinMode(9, OUTPUT);
    //Set Pin 9 output 2V
    analogWrite(9, 70);

    //Mode change input
    pinMode(buttonPin, INPUT);
}

// -----------------------------------------------------------------------------
// Arduino-Loop
// -----------------------------------------------------------------------------

void loop() {
  
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
  // Status change mode

  buttonState = digitalRead(buttonPin);

  if (buttonState != lastButtonState) {
    if (buttonState == HIGH) {
      buttonPushCounter++;
    }
    delay(100);
  } 
  lastButtonState = buttonState;

//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
  //Mode 0, 1, 2 (Over 2 will be set to zero)

  if (buttonPushCounter > 3) {
    buttonPushCounter = 0;
  }
  
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

  if (buttonPushCounter == 0) {
    Group1_A0_A1_V1();
  }
  if (buttonPushCounter == 1) {
    Group1_A0_A1_V2();
  }
  if (buttonPushCounter == 2) {
    Group2_A2_A3_V1();
  }
  if (buttonPushCounter == 3) {
    Group2_A2_A3_V2();
  }
}

//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

// -----------------------------------------------------------------------------
// Different Group function
// -----------------------------------------------------------------------------

void Group1_A0_A1_V1()
{
    //Get Differential of A0-A1, and get single-end reading A1.
    //Group1

    //Get differential
    adc0.setGain(ADS1115_PGA_0P256);
    float diff = 0;
    float Current = 0;
    int sensorOneCounts = adc0.getConversionP0N1();  // counts up to 16-bits  
    diff = sensorOneCounts*adc0.getMvPerCount();
    Current = diff / 0.2; // (0.2 ohm)



    //Get single-end V-load
    float adcA1;
    adc0.setGain(ADS1115_PGA_6P144);
    adc0.setMultiplexer(ADS1115_MUX_P1_NG);
    adcA1 = adc0.getConversionP1GND();  // counts up to 16-bits  
    adcA1 = adcA1*adc0.getMvPerCount() /1000;
    
    //Calculate PWR
    float PWR;
    PWR = Current * adcA1 ;  //  mA * I = mW


    // Display data to 0.96" OLED
    display.clearDisplay();
    display.setTextColor(WHITE);


    display.setTextSize(1); display.setTextColor(BLACK, WHITE); 
    display.setCursor(0,0); display.println(" A0-A1 ");
    display.setCursor(0,10); display.println("0.2/Ohm");
    display.setTextColor(WHITE);
    
    display.setTextSize(1); display.setCursor(48,0); display.println("V-Load:");
    display.setTextSize(1); display.setCursor(90,0); display.println(adcA1, 3);
    display.setTextSize(1); display.setCursor(122,0); display.println("V");
    
    display.setTextSize(1); display.setCursor(55,10); display.println("*V:");
    display.setTextSize(1); display.setCursor(72,10); display.println(diff, 4);
    display.setTextSize(1); display.setCursor(115,10); display.println("mV");

    display.drawRoundRect(0, 19, 128, 13, 2, WHITE);
    display.setTextSize(1); display.setCursor(1,22); display.println("  POWER CONSUMPTION");

    display.setTextSize(1); display.setCursor(0,38+2); display.print("IL:");
    display.setTextSize(2); display.setCursor(18,32+2); display.print(Current, 3);
    display.setTextSize(2); display.setCursor(104,32+2); display.print("mA");
    
    display.setTextSize(1); display.setCursor(0,54+2); display.print("PL:");
    display.setTextSize(2); display.setCursor(18,48+2); display.print(PWR, 3);
    display.setTextSize(2); display.setCursor(104,48+2); display.print("mW");
    
    display.display();
    display.clearDisplay(); 
}   

void Group1_A0_A1_V2()
{
    //Get Differential of A0-A1, and get single-end reading A1.
    //Group1

    //Get differential
    adc0.setGain(ADS1115_PGA_0P256);
    float diff = 0;
    float Current = 0;
    int sensorOneCounts = adc0.getConversionP0N1();  // counts up to 16-bits  
    diff = sensorOneCounts*adc0.getMvPerCount();
    Current = diff / 0.2; // (0.2 ohm)

    //Get single-end V-load
    float adcA1;
    adc0.setGain(ADS1115_PGA_6P144);
    adc0.setMultiplexer(ADS1115_MUX_P1_NG);
    adcA1 = adc0.getConversionP1GND();  // counts up to 16-bits  
    adcA1 = adcA1*adc0.getMvPerCount() /1000;
    
    //Calculate PWR
    float PWR;
    PWR = Current * adcA1 ;  //  mA * I = mW

    // Display data to 0.96" OLED
    display.clearDisplay();
    display.setTextColor(WHITE);
    
    display.setTextSize(1); display.setCursor(0,6); display.println("VL:");
    display.setTextSize(2); display.setCursor(18,0); display.println(adcA1, 3);
    display.setTextSize(2); display.setCursor(104,0); display.println("V");

    display.setTextSize(1); display.setCursor(0,22); display.println("*V:");
    display.setTextSize(2); display.setCursor(18,16); display.println(diff, 3);
    display.setTextSize(2); display.setCursor(104,16); display.println("mV");
    
    display.setTextSize(1); display.setCursor(0,38); display.print("I:");
    display.setTextSize(2); display.setCursor(18,32); display.print(Current, 3);
    display.setTextSize(2); display.setCursor(104,32); display.print("mA");
    
    display.setTextSize(1); display.setCursor(0,54); display.print("PL:");
    display.setTextSize(2); display.setCursor(18,48); display.print(PWR, 3);
    display.setTextSize(2); display.setCursor(104,48); display.print("mW");
    
    display.display();
    display.clearDisplay();
    delay(20);

    //UART transfer data to PC, will tearing while this open.
    Serial.print("V-load: "); Serial.print(adcA1, 3); Serial.print("V | ");
    Serial.print("*Voltage: "); Serial.print(diff, 3); Serial.print("mV | ");
    Serial.print("I-load: "); Serial.print(Current, 3); Serial.print("mA | ");
    Serial.print("P-load: "); Serial.print(PWR, 3); Serial.println("mW");
    delay(40);
}   

void Group2_A2_A3_V1()
{
    //Get Differential of A2-A3, and get single-end reading A1.
    //Group2

    //Get differential
    adc0.setGain(ADS1115_PGA_0P256);
    float diff = 0;
    float Current = 0;
    int sensorOneCounts = adc0.getConversionP2N3();  // counts up to 16-bits  
    diff = sensorOneCounts*adc0.getMvPerCount();
    Current = diff / 0.2; // (0.2 ohm)

    //Get single-end V-load
    float adcA1;
    adc0.setGain(ADS1115_PGA_6P144);
    adc0.setMultiplexer(ADS1115_MUX_P3_NG);
    adcA1 = adc0.getConversionP3GND();  // counts up to 16-bits  
    adcA1 = adcA1*adc0.getMvPerCount() /1000;
    
    //Calculate PWR
    float PWR;
    PWR = Current * adcA1 ;  //  mA * I = mW


    // Display data to 0.96" OLED
    display.clearDisplay();
    display.setTextColor(WHITE);


    display.setTextSize(1); display.setTextColor(BLACK, WHITE); 
    display.setCursor(0,0); display.println(" A2-A3 ");
    display.setCursor(0,10); display.println("0.2/Ohm");
    display.setTextColor(WHITE);
    
    display.setTextSize(1); display.setCursor(48,0); display.println("V-Load:");
    display.setTextSize(1); display.setCursor(90,0); display.println(adcA1, 3);
    display.setTextSize(1); display.setCursor(122,0); display.println("V");
    
    display.setTextSize(1); display.setCursor(55,10); display.println("*V:");
    display.setTextSize(1); display.setCursor(72,10); display.println(diff, 4);
    display.setTextSize(1); display.setCursor(115,10); display.println("mV");

    display.drawRoundRect(0, 19, 128, 13, 2, WHITE);
    display.setTextSize(1); display.setCursor(1,22); display.println("  POWER CONSUMPTION");

    display.setTextSize(1); display.setCursor(0,38+2); display.print("IL:");
    display.setTextSize(2); display.setCursor(18,32+2); display.print(Current, 3);
    display.setTextSize(2); display.setCursor(104,32+2); display.print("mA");
    
    display.setTextSize(1); display.setCursor(0,54+2); display.print("PL:");
    display.setTextSize(2); display.setCursor(18,48+2); display.print(PWR, 3);
    display.setTextSize(2); display.setCursor(104,48+2); display.print("mW");
    
    display.display();
    display.clearDisplay(); 
}   

void Group2_A2_A3_V2()
{
    //Get Differential of A2-A3, and get single-end reading A1.
    //Group2

    //Get differential
    adc0.setGain(ADS1115_PGA_0P256);
    float diff = 0;
    float Current = 0;
    int sensorOneCounts = adc0.getConversionP2N3();  // counts up to 16-bits  
    diff = sensorOneCounts*adc0.getMvPerCount();
    Current = diff / 0.2; // (0.2 ohm)

    //Get single-end V-load
    float adcA1;
    adc0.setGain(ADS1115_PGA_6P144);
    adc0.setMultiplexer(ADS1115_MUX_P3_NG);
    adcA1 = adc0.getConversionP3GND();  // counts up to 16-bits  
    adcA1 = adcA1*adc0.getMvPerCount() /1000;
    
    //Calculate PWR
    float PWR;
    PWR = Current * adcA1 ;  //  mA * I = mW

    // Display data to 0.96" OLED
    display.clearDisplay();
    display.setTextColor(WHITE);
    
    display.setTextSize(1); display.setCursor(0,6); display.println("VL:");
    display.setTextSize(2); display.setCursor(18,0); display.println(adcA1, 3);
    display.setTextSize(2); display.setCursor(104,0); display.println("V");

    display.setTextSize(1); display.setCursor(0,22); display.println("*V:");
    display.setTextSize(2); display.setCursor(18,16); display.println(diff, 3);
    display.setTextSize(2); display.setCursor(104,16); display.println("mV");
    
    display.setTextSize(1); display.setCursor(0,38); display.print("I:");
    display.setTextSize(2); display.setCursor(18,32); display.print(Current, 3);
    display.setTextSize(2); display.setCursor(104,32); display.print("mA");
    
    display.setTextSize(1); display.setCursor(0,54); display.print("PL:");
    display.setTextSize(2); display.setCursor(18,48); display.print(PWR, 3);
    display.setTextSize(2); display.setCursor(104,48); display.print("mW");
    
    display.display();
    display.clearDisplay();
    
    //UART transfer data to PC, will tearing while this open.
    Serial.print("V-load: "); Serial.print(adcA1, 3); Serial.print("V | ");
    Serial.print("*Voltage: "); Serial.print(diff, 3); Serial.print("mV | ");
    Serial.print("I-load: "); Serial.print(Current, 3); Serial.print("mA | ");
    Serial.print("P-load: "); Serial.print(PWR, 3); Serial.println("mW");
    delay(40); 
}   
