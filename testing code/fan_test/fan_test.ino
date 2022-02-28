
/*--------------------------- Version ------------------------------------*/
#define FW_NAME       "FanControllerTest-ESP32-FW"
#define FW_SHORT_NAME "Fan Controller Test"
#define FW_MAKER      "Austin's Creations"
#define FW_VERSION    "0.0.1"

/*--------------------------- Configuration ------------------------------*/
// Should be no user configuration in this file, everything should be in;
//#include "config.h"

/*--------------------------- Libraries ----------------------------------*/
#include <Wire.h>                 // For I2C
#include <Adafruit_EMC2101.h>     // For EMC driver control

/*--------------------------- Constants ----------------------------------*/
// Serial
#define SERIAL_BAUD_RATE 115200

// I2C Addresses
#define TCA_I2C_ADDRESS  0x70   // i2c address for the TCA9548 i2c mux IC
#define EMC_I2C_ADDRESS  0x4C   // i2c address for the EMC2101 fan driver

/*--------------------------- Global Variables ---------------------------*/
// Each bit corresponds to a EMC2101 found on the IC2 bus
uint8_t g_emc_found  = 0;

uint8_t testComplete = 0;
uint8_t fanCheck     = 0;
uint8_t emcCheck     = 0;

uint16_t prevRPM     = 0;

const int numReadings = 20;
int readings[numReadings];      // the readings from the analog input
int readIndex = 0;              // the index of the current reading
int total = 0;                  // the running total
int average = 0;                // the average


/*--------------------------- Instantiate Global Objects -----------------*/

Adafruit_EMC2101  emc2101;

/*--------------------------- Program ------------------------------------*/

void tcaselect(uint8_t i) {
  if (i > 7) return;
 
  Wire.beginTransmission(TCA_I2C_ADDRESS);
  Wire.write(1 << i);
  Wire.endTransmission();  
}


void setup() {

  // Startup logging to serial
  Serial.begin(SERIAL_BAUD_RATE);
  delay(200);
  Serial.println(F("\n========================================"));
  Serial.print  (F("FIRMWARE: ")); Serial.println(FW_NAME);
  Serial.print  (F("MAKER:    ")); Serial.println(FW_MAKER);
  Serial.print  (F("VERSION:  ")); Serial.println(FW_VERSION);
  Serial.println(F("========================================"));

  // Start the I2C bus
  //  Wire.begin(); //Wire.begin(pin_sda, pin_scl)
  //  Wire.begin();
  Wire.begin(33,32); // LilyGO POE pinout
  
  // Scan the I2C bus and set up PWM drivers
  scanI2CBus();

}

void loop() {
//test1(); // run the test
//test2(); // run the test - uses a for loop
//test3(); // run the test - uses a for loop - with error test for the automated loop

automatedTest(); // run automated test

//tempSmooth();    // runs test of getting smooth temp
  
}

void tempSmooth()
{
  uint8_t x = 0;
  
  if (testComplete == false)
  {
    tcaselect(x);
    emc2101.setDutyCycle(50);
    testComplete = true;
  }

   total = total - readings[readIndex];
  // read from the sensor:
  readings[readIndex] = emc2101.getExternalTemperature();
  // add the reading to the total:
  total = total + readings[readIndex];
  // advance to the next position in the array:
  readIndex = readIndex + 1;

  // if we're at the end of the array...
  if (readIndex >= numReadings) {
    // ...wrap around to the beginning:
    readIndex = 0;
  }

  // calculate the average:
  average = total / numReadings;
  // send it to the computer as ASCII digits
  Serial.println(average);
  delay(40);        // delay in between reads for stability

  
}

void automatedTest()
{
  if (testComplete == false)
  {
    Serial.print("\nTest Status");Serial.println("|| FAN  || TEMP ||\n");
    for (uint8_t tca=0; tca<8; tca++) 
    {
      tcaselect(tca);
      Serial.print("TCA Port #"); Serial.print(tca);
            
      for (uint8_t percent=4; percent<101; percent=percent+8) 
      {
        emc2101.setDutyCycle(percent);
        delay(300);
        if (percent >= 12)
        {
          if (emc2101.getFanRPM() > prevRPM)
          {
            fanCheck++;
          }
          prevRPM = emc2101.getFanRPM();
        }
        
        delay(300);
      }
      
      emc2101.setDutyCycle(0);
      prevRPM = 0;
      
      if (fanCheck == 12)                                           // checks if fan has passed rpm test
      {
        Serial.print("   PASS  ");
      }
      else
      {
        Serial.print("   FAIL  ");
      }
      if (emc2101.getExternalTemperature() < 120)                   // checks if external temp is reporting
      {
        Serial.println("  PASS   ");
      }
      else
      {
        Serial.println("  FAIL   ");
      }
      if (fanCheck == 12 && emc2101.getExternalTemperature() < 120) // checks that both fan and temp have passed
      {
        emcCheck++;
      }
      fanCheck = 0; // reset our error output
      delay(10000);
    }
    testComplete = true;
    if (emcCheck == 8)
    {
      Serial.println(F("\n========================================"));
      Serial.println(F(" Fan Controller test || PASSED "));
      Serial.println(F("========================================"));
    }
    else
    {
      Serial.println(F("\n========================================"));
      Serial.println(F(" Fan Controller test || FAILED "));
      Serial.println(F("========================================"));
    }
  }
}




void test1()
{
  if (testComplete == false)
  {
    tcaselect(1); // set what EMC we're controlling
    
    delay(5000);
    
    emc2101.setDutyCycle(5);
    emc2101.setDutyCycle(5);

    delay(2000);
    
    Serial.print("External Temperature: ");
    Serial.print(emc2101.getExternalTemperature());
    Serial.println(" degrees C");

    Serial.print("Internal Temperature: ");
    Serial.print(emc2101.getInternalTemperature());
    Serial.println(" degrees C");

    Serial.print("Duty Cycle: ");
    Serial.print(emc2101.getDutyCycle());
    Serial.print("% / Fan RPM: ");
    Serial.print(emc2101.getFanRPM());
    Serial.println(" RPM");
    Serial.println("");

    delay(5000);

    emc2101.setDutyCycle(51);
    emc2101.setDutyCycle(51);

    delay(2000);
    
    Serial.print("External Temperature: ");
    Serial.print(emc2101.getExternalTemperature());
    Serial.println(" degrees C");

    Serial.print("Internal Temperature: ");
    Serial.print(emc2101.getInternalTemperature());
    Serial.println(" degrees C");

    Serial.print("Duty Cycle: ");
    Serial.print(emc2101.getDutyCycle());
    Serial.print("% / Fan RPM: ");
    Serial.print(emc2101.getFanRPM());
    Serial.println(" RPM");
    Serial.println("");

    delay(5000);

    emc2101.setDutyCycle(100);
    emc2101.setDutyCycle(100);

    delay(2000);
    
    Serial.print("External Temperature: ");
    Serial.print(emc2101.getExternalTemperature());
    Serial.println(" degrees C");

    Serial.print("Internal Temperature: ");
    Serial.print(emc2101.getInternalTemperature());
    Serial.println(" degrees C");

    Serial.print("Duty Cycle: ");
    Serial.print(emc2101.getDutyCycle());
    Serial.print("% / Fan RPM: ");
    Serial.print(emc2101.getFanRPM());
    Serial.println(" RPM");
    Serial.println("");

    delay(5000);
    
    emc2101.setDutyCycle(0);

    delay(1000);
    Serial.print(emc2101.getFanRPM());
    Serial.println(" RPM");

    testComplete = true;
  }
}


void test2()
{
  if (testComplete == false)
  {
    tcaselect(0); // set what EMC we're controlling

    emc2101.setDutyCycle(4);
    delay(500);

//    delay(5000);

    for (uint8_t percent=4; percent<101; percent=percent+8) 
    {
      emc2101.setDutyCycle(percent);
      delay(250);
      Serial.print("Duty Cycle: ");
      Serial.print(emc2101.getDutyCycle());
      Serial.print("% / Fan RPM: ");
      Serial.print(emc2101.getFanRPM());
      Serial.println(" RPM");
      delay(250);
    }
    for (uint8_t percent=100; percent>4; percent=percent-8) 
    {
      emc2101.setDutyCycle(percent);
      delay(100);
      Serial.print("Duty Cycle: ");
      Serial.print(emc2101.getDutyCycle());
      Serial.print("% / Fan RPM: ");
      Serial.print(emc2101.getFanRPM());
      Serial.println(" RPM");
      delay(250);
    }
    
    emc2101.setDutyCycle(0);
    testComplete = true;
  }
}

void test3()
{
  if (testComplete == false)
  {

   for (uint8_t tca=0; tca<4; tca++) 
    {
      tcaselect(tca);
      Serial.print("TCA Port #"); Serial.println(tca);
            
      for (uint8_t percent=4; percent<101; percent=percent+8) 
      {
        emc2101.setDutyCycle(percent);
        delay(300);
        Serial.print("Duty Cycle: ");
        Serial.print(emc2101.getDutyCycle());
        Serial.print("% / Fan RPM: ");
        
        if (percent >= 12)
        {
          if (emc2101.getFanRPM() > prevRPM)
          {
            Serial.print(emc2101.getFanRPM());
            Serial.println(" RPM");
          }
          else
          {
            Serial.print(emc2101.getFanRPM());
            Serial.print(" RPM - prev = ");
            Serial.println(prevRPM);
          }
          prevRPM = emc2101.getFanRPM();
        }
        else
        {
          Serial.print(emc2101.getFanRPM());
          Serial.println(" RPM - too low percent");
        }

        delay(300);
      }
      emc2101.setDutyCycle(0);
      prevRPM = 0;
      delay(10000);
    }
    testComplete = true;
  }
}

/**
  I2C bus
 */
void scanI2CBus()
{
  Serial.println(F("[] scanning for EMC drivers on TCA MUX..."));

  for (uint8_t tca=0; tca<8; tca++) 
  {
    tcaselect(tca);
    Serial.print("TCA Port #"); 
    Serial.print(tca);
    Serial.print(F(" - 0x"));
    Serial.print(EMC_I2C_ADDRESS, HEX);
    Serial.print(F("..."));

    // Check if there is anything responding on this address
    Wire.beginTransmission(EMC_I2C_ADDRESS);
    if (Wire.endTransmission() == 0)
    {
      bitWrite(g_emc_found, tca, 1);

      if (!emc2101.begin())
      {
         Serial.println(F("FAILED"));
      }
      else
      {
        Serial.print(F("EMC2101"));
        Serial.print(F("... set to: "));
         //  emc2101.setDataRate(EMC2101_RATE_1_16_HZ);
        switch (emc2101.getDataRate()) 
        {
          case EMC2101_RATE_1_16_HZ: Serial.println("1/16_HZ"); break;
          case EMC2101_RATE_1_8_HZ: Serial.println("1/8_HZ"); break;
          case EMC2101_RATE_1_4_HZ: Serial.println("1/4_HZ"); break;
          case EMC2101_RATE_1_2_HZ: Serial.println("1/2_HZ"); break;
          case EMC2101_RATE_1_HZ: Serial.println("1 HZ"); break;
          case EMC2101_RATE_2_HZ: Serial.println("2 HZ"); break;
          case EMC2101_RATE_4_HZ: Serial.println("4 HZ"); break;
          case EMC2101_RATE_8_HZ: Serial.println("8 HZ"); break;
          case EMC2101_RATE_16_HZ: Serial.println("16 HZ"); break;
          case EMC2101_RATE_32_HZ: Serial.println("32 HZ"); break;
        }
        emc2101.enableTachInput(true);
        emc2101.setPWMDivisor(0);
        emc2101.setDutyCycle(0);

      }
    }
    else
    {
      Serial.println(F("empty"));
    }
  }
}
