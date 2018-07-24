//THIS CODE IS OUTDATED, SEE YOUSHOE.INO FOR MOST RECENT CODE

#include <SoftwareSerial.h>

#include <TinyGPS++.h>
#include <SoftwareSerial.h>
#include <SD.h>

File fd; 
char filename[] = "TrueShoeData.txt"; //Name of GPS data file

static const int RXPin = 9, TXPin = 10; // Software Serial pins for GPS Module
static const uint32_t GPSBaud = 9600;

const int cardSelect = 7;  // This is really the only important pin you need. CardDetect is meant for another pin that simply checks if the SD card is in. 
const int cardDetect = 6;
int count = 0; // variable for pedometer

unsigned long starttime = millis(); 
unsigned long endtime = starttime;
unsigned long lastUpdateTime = 0;

static char outstr [15]; //stores GPS speed float
float desiredspeed; 
float duration; 
float runspeed;

bool alreadyBegan = false; 

TinyGPSPlus gps; //object to which GPS data sent to

SoftwareSerial ss(RXPin, TXPin); 
SoftwareSerial ble(0, 1); //BLE module 

String inString = "";

int fsrPin = 0; //A0
int fsrReading;

float gpslat = gps.location.lat();
float gpslng = gps.location.lng();
float gpsspeed = gps.speed.mph();

float latit; //used in navigation
float longit;

void setup()
{
  Serial1.begin(9600);
  ble.begin(9600);
  ss.begin(GPSBaud);

  delay(7000);
  Serial1.println("Rohan's Smart Shoe");
  Serial1.println("Testing TinyGPS++ library v. "); //Credits to Tiny GPS++ library 
  Serial1.println(TinyGPSPlus::libraryVersion());
  Serial1.println("Please enter a command");
  Serial1.println();
  
  
  //initializeCard(); For use with larger micro-controllers (enables SD card)
  //runcard();

  pinMode(cardSelect, INPUT);
  pinMode(6, OUTPUT); //motor disks
  pinMode(12, OUTPUT);
}

void loop()
{
  if (Serial1.available())
    {
      String str = Serial1.readString();
      
      if (str. equalsIgnoreCase("gps data"))
        {
          while (ss.available() > 0)
           if (gps.encode(ss.read()))
             displayInfo();

          if (millis() > 5000 && gps.charsProcessed() < 10)
             {
    Serial1.println(F("No GPS detected: check wiring."));
    Serial1.flush();
          }
          Serial1.flush();
        }

       if (str. equalsIgnoreCase("pedometer")) 
       {
        Serial1.println("Your current number of steps are: ");
        Serial1.print(count);
        Serial1.flush();
       }

       if (str. equalsIgnoreCase("navigation"))
       {

        Serial1.println(F("Enter latitude: "));
        while(Serial1.available() == 0); 
        latit = Serial1.parseFloat(); //try float for more accurate data
        Serial1.println(latit);
        Serial1.println(F("Enter longitude: "));
        while(Serial1.available() == 0);
        longit = Serial1.parseFloat(); // try float for more accurate data
        Serial1.println(longit);

        
        Serial1.println(F("Ready To Begin Course? Type yes/no")); 
        delay (5000);
        str = Serial1.readString();
        if (str. equalsIgnoreCase("yes")) {
            Serial1.flush();
            Serial1.println(F("Route Beginning. Please Follow Signals From Vibration Motors"));

          while(ss.available() > 0)
          gps.encode(ss.read());

          while (millis() - lastUpdateTime >= 5000)
        {
          lastUpdateTime = millis();
          Serial1.println();

           double distanceToDestination = TinyGPSPlus::distanceBetween(
            gps.location.lat(), gps.location.lng(), latit, longit);

          double courseToDestination = TinyGPSPlus::courseTo(
            gps.location.lat(), gps.location.lng(), latit, longit);

          const char *directionToDestination = TinyGPSPlus::cardinal(courseToDestination);
          int courseChangeNeeded = (int)(360 + courseToDestination - gps.course.deg()) % 360;

          
          if (distanceToDestination <= 20.0)
          {
            Serial1.println("You have arrived at:");
            Serial1.print(latit);
            Serial1.print(longit);
            digitalWrite(6, HIGH);
            delay(3000);
            digitalWrite(6, LOW);
            exit(1);
            Serial.flush();
          }

           if (gps.speed.kmph() < 2.0)
           {
            Serial1.print("Head ");         
            Serial1.print(directionToDestination);
            Serial1.flush();
           }
           if (courseChangeNeeded >=345 || courseChangeNeeded < 15)
              {
              digitalWrite(6,HIGH);
              digitalWrite(12, HIGH);
              delay(500);
              digitalWrite(6,LOW);
              digitalWrite(12, LOW);
              Serial1.println("Straight");
              Serial1.flush();

              }

            else if (courseChangeNeeded >= 315 && courseChangeNeeded < 345)
              {
                digitalWrite(6,HIGH);
                delay(500);
                digitalWrite(6,LOW);
                Serial1.println("Slight Left");
                Serial1.flush();

              }

            else if (courseChangeNeeded >= 255 && courseChangeNeeded < 315)
            {
              digitalWrite(6,HIGH);
              delay(1000);
              digitalWrite(6,LOW);
              Serial1.println("Left");
              Serial1.flush();

            }

            else if (courseChangeNeeded >= 15 && courseChangeNeeded < 45)
            {
              digitalWrite(12,HIGH);
              delay(500);
              digitalWrite(12,LOW);
              Serial1.println("Slight Right");
              Serial1.flush();

            }

            else if (courseChangeNeeded >= 45 && courseChangeNeeded < 105)
            {
              digitalWrite(12,HIGH);
              delay(1000);
              digitalWrite(12,LOW);
              Serial1.println("Right");
              Serial1.flush();

            }

            else 
            {
              Serial1.println("Turn around");
              digitalWrite(6,HIGH);
              digitalWrite(12, HIGH);
              delay(3000);
              digitalWrite(6,LOW);
              digitalWrite(12, LOW);
              Serial1.flush();

            }
        }
     }

  }
    if (str. equalsIgnoreCase("reset"))
     {
      count = 0;
      desiredspeed = 0;
      duration = 0;  
      Serial1.println("All data reset");
     }

     if (str. equalsIgnoreCase("workout"))
     {
     Serial1.println(F("Welcome to the workout setting. Please enter your desired speed, interval duration, break, and repetition."));
     workout();
     exercise();
  
     }
    }

       Serial1.flush();
   }
                   
void displayInfo()
{
  Serial1.flush();
  Serial1.print(F("Location: ")); 
  if (gps.location.isValid())
  {
    Serial1.print(gps.location.lat(), 6);
    Serial1.print(F(","));
    Serial1.print(gps.location.lng(), 6);
    Serial1.println(gps.date.value()); // Raw date in DDMMYY format (u32)
    Serial1.println(gps.time.value()); // Raw time in HHMMSSCC format (u32)
    Serial1.print(F("Speed in miles/hour:"));
    Serial1.println(gps.speed.mph()); // Speed in miles per hour (double)
    Serial1.print(F("Number of Satellites In Use:"));
    Serial1.println(gps.satellites.value()); // Number of satellites in use (u32) 
  }
  else
  {
    Serial1.print(F("INVALID"));
  }  
  Serial1.println();
  Serial1.flush();
}

void initializeCard(void)
{
    Serial1.println("Initializing SD Card...");
    if (!SD.begin(cardSelect))
    {
      Serial1.println(F("Please ensure card is securely in place"));
      Serial1.flush(); //fix later to while(1);
    }
    Serial1.print("Card Initialized");
    Serial1.flush();
}

void forcesensor ()
{
  fsrReading = analogRead(fsrPin);
  if (fsrReading > 200) {
    count + 2;
  }
}

void workout ()
{
 Serial1.println(F("Please enter desired speed in mintutes/mile."));
 delay (2000);
 String temp = Serial1.readString();
 desiredspeed = temp.toFloat();
 Serial1.println(temp);
 Serial1.println(F("Please enter desired duration (seconds)"));
 delay(2000);
 temp = Serial1.readString();
 Serial1.println(temp);
 delay(2000);
 duration = temp.toFloat(); 
}

void exercise ()
{
  Serial1.println(desiredspeed);
  Serial1.println(duration);
  duration = duration * 1000;
  Serial1.println("Program begins in twenty seconds. Buzzer will vibrate to indicate program beginning.");
  delay(20000);
  digitalWrite(6, HIGH);
  delay(500);
  digitalWrite(6, LOW);
  
  while ((endtime - starttime) <=duration)  
  {
    runspeed = (gps.speed.mph())/60; 
    if (runspeed + 0.2 < desiredspeed)
    {
      digitalWrite(6, HIGH);
      delay(1000);
      digitalWrite(6, LOW);
      delay(5000);
    }
     if (runspeed - 0.2 > desiredspeed)
     {
      digitalWrite(6, HIGH);
      delay(1000);
      digitalWrite(6, LOW);
      delay(500); 
      digitalWrite(6, HIGH);
      delay(1000);
      digitalWrite(6, LOW);
      delay(5000);
     }
    if (gps.speed.isUpdated())
    {
      runspeed = (gps.speed.mph())/60;
    }
  }
  Serial1.println("Program complete");
}

void runcard ()
{ 
  static float f_val = gps.speed.mph();
  String latitude = ("Latitude: ") + (gps.location.lat(), 6);
  String longitude = ("Longitude: ") + (gps.location.lng(), 6);
  String date1 = ("Data: ") + (gps.date.value());
  String time1 = ("Time: ") + (gps.time.value());
  dtostrf(f_val, 3, 2, outstr);
  String satellites1 = ("Satellites Used:") + (gps.satellites.value());
  String datapedometer = "";
  String datagps = "";
  String dataworkout1 = "";
  String dataworkout2 = "";
  datapedometer += String(count);
  dataworkout1 += String(desiredspeed);
  dataworkout2 += String(duration);
  datagps += latitude + longitude + date1 + time1 + outstr + satellites1;
  delay(120000);

  File dataFile = SD.open(filename, FILE_WRITE);
  if (dataFile) {
    dataFile.println(datapedometer);
    dataFile.println(dataworkout1);
    dataFile.println(dataworkout2);
    dataFile.println(datagps);
    dataFile.close();
    Serial.println("Data uploaded to SD card");
  }
  else {
    Serial.println("Error opening file");
  }
}





  
  

