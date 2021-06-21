
/**************************************************************************

CAN BUS VESC to 128x64px OLED DISPLAY

Displays VESC data over CAN BUS

Written by Jack Davies - Feb 2021 - Amsterdam

http://jackdavies.co

128x64 pixel display using I2C to communicate
3 pins are required to interface (two I2C and one reset).

**************************************************************************/

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <mcp2515.h>

// this is the working file

unsigned long previousMillis = 0;        // will store last time display was updated
const long interval = 100;  //100         // interval at which to update the display (milliseconds) 10fps


#include <Fonts/goodbyeDespair25pt7b.h>
//#include <Fonts/goodbyeDespair10pt7b.h>
#include <Fonts/goodbyeDespair8pt7b.h>
//#include <Fonts/goodbyeDespair5pt7b.h>

uint32_t rpm;
int amp = 1;
int speedKph;
float current;
float voltage;
float voltPerCell;
float odoDistance;

const int battSeriesCells = 12; // number of series cells in pack for single cell voltage calc
const int wheelDia = 50; // Wheel dia in CM to calculate speed from RPM
const int hallPin = 2; // Pin connected to the hall sensor (pin 2 is interupt capable)

struct can_frame canMsg;

MCP2515 mcp2515(10);


#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET -1 //4 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);


union ArrayToInteger { // for combining the 4 sections of data in the RPM can message
  byte array[4];
  uint32_t integer;
};

//union ArrayToInteger1 { // for combining the 4 sections of data in the RPM can message
//  byte array[2];
//  uint32_t integer1;
//};

//#define SCREEN_WIDTH 128
//#define SCREEN_HEIGHT 64
//#define OLED_RESET 4
//Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);




void setup() {
  Serial.begin(115200);

  Serial.print("Setup Begin");

  pinMode(hallPin, INPUT);

  mcp2515.reset();
  mcp2515.setBitrate(CAN_500KBPS, MCP_16MHZ); // MCP_8MHZ or MCP_16MHZ or MCP_20MHZ
  mcp2515.setNormalMode();


  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
//
  // Clear the buffer
  display.clearDisplay();

  
  display.setTextColor(SSD1306_WHITE);
  display.setFont(&goodbyeDespair8pt7b);         // Use full 256 char 'Code Page 437' font
  display.setCursor(10, 30);
  display.println(F("UNDERGROUND"));
  display.setCursor(16, 45);
  display.println("ENGINEERING");
  display.display();
  delay(2000);
  
  display.clearDisplay();
  
}

void loop() {


  

    
  if (mcp2515.readMessage(&canMsg) == MCP2515::ERROR_OK) {

        if(canMsg.can_id == 0x80000967) // Motor RPM // Speed
        {
                 
           ArrayToInteger converter = {canMsg.data[3],canMsg.data[2],canMsg.data[1],canMsg.data[0]}; // merge 4 bits of data into a 32 bit number for RPM
           rpm = converter.integer / 38; // /38


           current = canMsg.data[4] << 8 | canMsg.data[5];


           


           //current = canMsg.data[5]; // / 10; // Current value in Amps

//            Serial.print("amps4 - ");
//            Serial.println(canMsg.data[4]);
//
//            Serial.print("amps5 - ");
//            Serial.println(canMsg.data[5]);


//          ArrayToInteger1 converter1 = {canMsg.data[4],canMsg.data[5]}; // merge 4 bits of data into a 32 bit number for RPM

            //int combined = ((int) highByte << 8) | lowByte;

   //         current = ((int) canMsg.data[5] << 8) | canMsg.data[4];
          
          //current = (converter1.integer1 / 38) / 10; // /38


//          int current1 = canMsg.data[4];  // voltage is split over 2 bytes, read both then add together
//          int current2 = canMsg.data[5];
//
//          current = (canMsg.data[4] + canMsg.data[5]) / 38;

//
//           Serial.print(" current5 - ");
//           Serial.println(canMsg.data[5]);
//
//           Serial.print(" current - ");
//           Serial.println(current);
          
        }

        if(canMsg.can_id == 0x80001B67) //  Can ID for Voltage
        {
          
          int volt1 = canMsg.data[4];  // voltage is split over 2 bytes, read both then add together
          int volt2 = canMsg.data[5];


          //voltage = ((volt1 & 0xFF) << 8) | (volt2 & 0xFF);

          voltage = volt1 << 8 | volt2;

//           Serial.print(" volt5 - ");
//           Serial.println(canMsg.data[4]);
//
//           Serial.print(" volt6 - ");
//           Serial.println(canMsg.data[5]);

//           Serial.print(" voltage - ");
//           Serial.println(voltage);

//          voltage = (volt1 + volt2) / 10;


//          //tacho = canMsg.data[5] / 10; // Current value in Amps
//
//          Serial.print(" tacho6 - ");
//          Serial.println(canMsg.data[6]);
//
//          Serial.print(" tacho7 - ");
//          Serial.println(canMsg.data[7]);
//
//          delay(1000);

          
        }


 }

//   else {
//
//     
//      display.clearDisplay();
//      display.setTextColor(SSD1306_WHITE);
//      display.setFont(&goodbyeDespair8pt7b);         // Use full 256 char 'Code Page 437' font
//      display.setCursor(16, 30);
//      display.println(F("CONNECTION"));
//      display.setCursor(16, 45);
//      display.println("ERROR");
//      display.display();
//      delay(200);
//      
//    }


        unsigned long currentMillis = millis(); // timer to update the display every n seconds defined above

        if (currentMillis - previousMillis >= interval) {


            previousMillis = currentMillis;  // save the last time the display was updated

            speedKph = wheelDia * rpm * 0.001885;
            
            if (speedKph > 100) {  // filters out noise - speed wont be above 100kph... probably!
              speedKph = 0;
            }

            if (current >= 250 && current <= 255) {  // filters out noise
              current = 0;
            }

            current = current / 10;

            voltage = voltage / 10;

            voltPerCell = voltage / battSeriesCells;

            Serial.println(voltage);

            //odoDistance = odoDistance + (((speedKph * interval) / 3600) / 1000); // rough odometer from rpm and interval of timer - /3600 to convert 0.1 second to hour

            odoDistance = odoDistance + (speedKph * interval); // rough odometer from rpm and interval of timer - /3600 to convert 0.1 second to hour

//            Serial.print("ODO - ");
//            Serial.println(odoDistance/3600000); // 3600000 calcualtion needs to be done just here on the display output

//            Serial.print("amps - ");
//            Serial.println(current);

            display.clearDisplay();

            display.drawFastHLine(0, 45, 128, SSD1306_WHITE); // draw divider line

            display.setFont(&goodbyeDespair25pt7b);         // Use full 256 char 'Code Page 437' font
      
            if (speedKph < 10) {  // if the number is single digits then move the cursor so the number is positioned correctly 
              display.setCursor(37, 35);  // shouldnt get to tripple digits
            }
            else {
              display.setCursor(14, 35);
            }
            
            

            display.println(speedKph);

            if (speedKph < 10) {  // if the number is single digits then move the cursor so the number is positioned correctly
            display.setCursor(70, 35);
            }
            else {
              display.setCursor(80, 35);
            }
            display.setFont(&goodbyeDespair8pt7b);
            display.println(F("KPH"));


            
            
            // voltage display
            display.setCursor(2, 62);   
            display.println(voltPerCell, 1); // round to 1 decimal place
            display.setCursor(28, 62);
            display.println(F("v"));

            // current display
            display.setCursor(50, 62);   
            display.println(current, 1); // round to 1 decimal place
            
            if (current < 10) {  // if the number is single digits then move the cursor so the number is positioned correctly
            display.setCursor(76, 62);
            }
            else {
              display.setCursor(86, 62);
            }
            display.println(F("A"));

            // odometer display
            display.setCursor(95, 62);   
            display.println(odoDistance/3600000, 1); // round to 1 decimal place // 3600000 calcualtion needs to be done just here on the display output
            
            display.display();

        }


        
    
//    if (digitalRead(hallPin) == 1) {  // hall sensor trigger
//      Serial.println("hallTrigger");
//    }



}
