#include "MAX30105.h"
#include "heartRate.h"
#include "spo2_algorithm.h"
//Error mesages
#define ERROR1 "MAX30105 was not found."
#define ERROR2 " No finger "
#define ERROR3 "Stopped"

char input = 'Z';
int result = 0;
long int start1,current;
//CODE FOR MAX30102

MAX30105 particleSensor;






//CODE FOR MAX30102 ============================X======================

void setup() {
  Serial.begin(9600);
  
   // Initialize sensor
  if (!particleSensor.begin(Wire)) //Use default I2C port, 400kHz speed
  {
    Serial.println(ERROR1);
    while (1);
  }
  particleSensor.shutDown(); //Put Module to low power mode

}

void loop() {
  //Serial.println(">"); //TODO: Remove later
  //establishContact();
  if (Serial.available() > 0) {
    input = Serial.read();
    switch (input) {
      case 'H':
        result = calculate_hr();
        print_result(result);
        break;
      case 'O':
        result = calculate_spo2();
        print_result(result);
        break;
      case 'G':
        result = calculate_glu();
        print_result(result);
        break;
      case 10:
        break;
      case 13:
        break;
      default:
        Serial.print(input);
        Serial.println("Error");
    }
  }
}

void establishContact() {
  while (Serial.available() <= 0) {
    Serial.print(".");   // send time after startup
    delay(1000);
  }
}

void print_result(int result){
  switch(result){
    case 1:
      //Serial.println("Done");
      break;
    case 2:
      Serial.println(ERROR2);
      break;
    case 3:
      Serial.println(ERROR3);
      break;
  }
}

int calculate_hr() {

  const byte RATE_SIZE = 10; //Increase this for more averaging. 4 is good.
  byte rates[RATE_SIZE]; //Array of heart rates
  byte rateSpot = 0;
  long lastBeat = 0; //Time at which the last beat occurred

  float beatsPerMinute;
  int beatAvg;

  particleSensor.wakeUp();//Put Module out of Low power mode
  //Serial.println("Place your index finger on the sensor with steady pressure.");

  particleSensor.setup(); //Configure sensor with default settings
  particleSensor.setPulseAmplitudeRed(0x0A); //Turn Red LED to low to indicate sensor is running

  start1=millis();
  current=start1;

  while ((current-start1)<6000) {
    long irValue = particleSensor.getIR();
    if (irValue < 10000)
    {
      particleSensor.shutDown();
      return 2;
    }
    if (checkForBeat(irValue) == true)
    {
      //We sensed a beat!
      long delta = millis() - lastBeat;
      lastBeat = millis();

      beatsPerMinute = 60 / (delta / 1000.0);

      if (beatsPerMinute < 255 && beatsPerMinute > 20)
      {
        rates[rateSpot++] = (byte)beatsPerMinute; //Store this reading in the array
        rateSpot %= RATE_SIZE; //Wrap variable

        //Take average of readings
        beatAvg = 0;
        for (byte x = 0 ; x < RATE_SIZE ; x++)
          beatAvg += rates[x];
        beatAvg /= RATE_SIZE;
      }
    }

  //Serial.print("BPM:");
  //Serial.print(beatsPerMinute);
  //Serial.print(" Avg BPM:");
  //Serial.print(beatAvg);

  /*if (irValue < 50000)
    Serial.print(" No finger?");
  */
  //Serial.println();
  current=millis();
  if(Serial.read()=='X'){
      particleSensor.shutDown(); //Put sensor to low power mode on cancel operation by user
      return 2;
    }
  
  }
  //Serial.print(" Avg BPM:");
  Serial.println(beatAvg);

  particleSensor.shutDown(); //Put sensor to low power mode after specified time
  return 1;
}

int calculate_spo2(){
  //Arduino Uno doesn't have enough SRAM to store 100 samples of IR led data and red led data in 32-bit format
  //To solve this problem, 16-bit MSB of the sampled data will be truncated. Samples become 16-bit data.
  uint16_t irBuffer[50]; //infrared LED sensor data
  uint16_t redBuffer[50];  //red LED sensor data
  

  int32_t bufferLength; //data length
  int32_t spo2; //SPO2 value
  int8_t validSPO2=0; //indicator to show if the SPO2 calculation is valid
  int32_t heartRate; //heart rate value
  int8_t validHeartRate=0; //indicator to show if the heart rate calculation is valid


  byte ledBrightness = 60; //Options: 0=Off to 255=50mA
  byte sampleAverage = 4; //Options: 1, 2, 4, 8, 16, 32
  byte ledMode = 2; //Options: 1 = Red only, 2 = Red + IR, 3 = Red + IR + Green
  byte sampleRate = 100; //Options: 50, 100, 200, 400, 800, 1000, 1600, 3200
  int pulseWidth = 411; //Options: 69, 118, 215, 411
  int adcRange = 4096; //Options: 2048, 4096, 8192, 16384
  bufferLength = 50; //buffer length of 100 stores 4 seconds of samples running at 25sps
  start1=millis();
  current=start1;
  particleSensor.wakeUp();
  particleSensor.setup(ledBrightness, sampleAverage, ledMode, sampleRate, pulseWidth, adcRange); //Configure sensor with these settings

  //read the first 100 samples, and determine the signal range
  while(validSPO2!=1){
    for (byte i = 0 ; i < bufferLength && (current-start1)<60000 ; i++)
    {
      while (particleSensor.available() == false) //do we have new data?
        particleSensor.check(); //Check the sensor for new data

      redBuffer[i] = particleSensor.getRed();
      irBuffer[i] = particleSensor.getIR();
      particleSensor.nextSample(); //We're finished with this sample so move to next sample
      //Serial.print(irBuffer[i]);
      if (irBuffer[0] < 10000)
      {
        particleSensor.shutDown();
        return 2;
      }
      if(Serial.read()=='X'){
        particleSensor.shutDown();
        return 3;
      }
      //Serial.print(F("red="));
      //Serial.print(redBuffer[i], DEC);
      //Serial.print(F(", ir="));
      //Serial.println(irBuffer[i], DEC);
    }
    //calculate heart rate and SpO2 after first 100 samples (first 4 seconds of samples)  
    maxim_heart_rate_and_oxygen_saturation(irBuffer, bufferLength, redBuffer, &spo2, &validSPO2, &heartRate, &validHeartRate);
    //Serial.print(spo2);
  }
  //Serial.print("Spo2: ");
  Serial.println(spo2);
  particleSensor.shutDown();
  return 1;
}

int calculate_glu(){
  int glu=0;
  bool validGlu=0;
//  start1=millis();
//  current=start1;
//  while((current-start1)<60000){
//    glu=random(100,120);
//    Serial.print(" Glu:");
//    Serial.println(glu);
//    current=millis();
//
//    delay(1000);
//    if(Serial.read()=='X')
//      return 3;
//  }
  while(validGlu==0){
    glu=analogRead(A3);
    if(glu>100){
      Serial.println(F("No Finger"));
      break;
    }
    else{
      glu=glu*15;
      glu=(3*pow(10,-5)*pow(glu,2)) + (0.2903*glu)-4.798;
      Serial.println(glu);
      break;
    }
  }
  
  return 1;

}
