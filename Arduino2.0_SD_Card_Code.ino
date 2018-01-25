#include <SoftwareSerial.h>
#include <Adafruit_VC0706.h>
#include <Adafruit_INA219.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <SD.h>
#include <SPI.h>

#define chipSelect 10
#define DHTPIN 6
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);
Adafruit_INA219 ina219;
SoftwareSerial cameraconnection = SoftwareSerial(0, 1);
Adafruit_VC0706 cam = Adafruit_VC0706(&cameraconnection);

unsigned long  time;
int  hours;
int  minutes;
int  seconds;

void setup(void)
{
  Serial.begin(9600);
  if (!SD.begin(chipSelect))
  {
    Serial.println("initialization failed!");
    return;
  }
  ina219.begin();
  dht.begin();
  if (!cam.begin()) {
    Serial.println("No camera found?");
    return;
  }
  cam.setImageSize(VC0706_160x120);
  
  for (int i = 0; i < 100; i++) {
    if (chipSelect != 10) pinMode(10, OUTPUT);
    delay(3000);
    if (! cam.takePicture()){
      Serial.println("Failed to snap!");
    }
    char filename[13];
    strcpy(filename, "IMAGE00.JPG");
    for (int i = 0; i < 100; i++) {
      filename[5] = '0' + i / 10;
      filename[6] = '0' + i % 10;
      if (! SD.exists(filename)) {
        break;
      }
    }
    File imgFile = SD.open(filename, FILE_WRITE);

    uint16_t jpglen = cam.frameLength();
    pinMode(8, OUTPUT);
    byte wCount = 0; // For counting # of writes
    while (jpglen > 0) {
      uint8_t *buffer;
      uint8_t bytesToRead = min(32, jpglen); // change 32 to 64 for a speedup but may not work with all setups!
      buffer = cam.readPicture(bytesToRead);
      imgFile.write(buffer, bytesToRead);
      if (++wCount >= 64) { // Every 2K, give a little feedback so it doesn't appear locked up
        wCount = 0;
      }
      jpglen -= bytesToRead;
    }
    imgFile.close();

    delay(15000);

    char logname[13];
    strcpy(logname, "LOG00.txt");
    for (int i = 0; i < 100; i++)
    {
      logname[3] = '0' + i / 10;
      logname[4] = '0' + i % 10;
      // create if does not exist, do not open existing, write, sync after write
      if (! SD.exists(logname))
      {
        break;
      }
    }
    File myFile = SD.open(logname, FILE_WRITE); \
    if (myFile) {
      time = millis();
      seconds =       (time / 1000) % 60;
      //minutes =       ((time / (1000 * 60)) % 60);
      //hours   =       ((time / (1000 * 60 * 60)) % 24);
      delay(5000);
      myFile.print("Hours:");         myFile.print(hours);        myFile.println("");
      myFile.print("Minutes:");       myFile.print(minutes);      myFile.println("");
      myFile.print("Seconds:");       myFile.print(seconds);      myFile.println("");
      //myFile.print("\n");

      float busvoltage = 0;
      float current_mA = 0;
      float power = 0;

      busvoltage = ina219.getBusVoltage_V();
      current_mA = ina219.getCurrent_mA();
      power = busvoltage * current_mA;
      
      float h = dht.readHumidity();
      // Read temperature as Fahrenheit (isFahrenheit = true)
      float f = dht.readTemperature(true);

      // Check if any reads failed and exit early (to try again).
      if (isnan(h) || isnan(f)) {
        Serial.println("Failed to read from DHT sensor!");
      }
      myFile.print("Power:"); myFile.print(power); myFile.println("mW");
      myFile.print("\n");
      myFile.print("Humidity: ");
      myFile.print(h);
      myFile.print(" %\t");
      myFile.print("Temperature: ");
      myFile.print(f);
      myFile.println(" *F\t");
      myFile.close();
      
      Serial.print("Log Complete!");
    }
    else {
      Serial.println("error opening LOG00.txt");
    }
    delay(2000);
  }
}

void loop(void)
{
}



