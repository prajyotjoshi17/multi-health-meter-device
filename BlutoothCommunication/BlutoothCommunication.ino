//Error mesages
#define ERROR1 "MAX30105 was not found."
#define ERROR2 " No finger "
#define ERROR3 "Stopped"

int input=0;
int hr,glu,spo2;
int result=0;


void setup() {
  // start serial port at 9600 bps:
  Serial.begin(9600);
  

  //establishContact();  // send a byte to establish contact until receiver responds
}

void loop() {
  //Serial.println("Enter Command to continue\n"); //TODO: Remove later
  //establishContact();
  // if we get a valid byte, read analog ins:
  if (Serial.available() > 0) {
    input=Serial.read();
    switch(input){
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
        Serial.println("Invalid input");
    }
  }
}

void establishContact() {
  while (Serial.available() <= 0) {
    Serial.print(".");
    delay(1000);
  }
}

int calculate_hr(){
  //send random readings
  long int start1=millis();
  long int current=start1;
  /*while((current-start1)<60000){
    hr=random(60,90);
    Serial.print("HR: ");
    Serial.println(hr);
   
    current=millis();
    delay(1000);
    if(Serial.read()=='X')
      return 2;
     
  }*/
  hr=random(60,90);
  //delay(10000);
  //Serial.print("HR: ");
  Serial.println(hr);
  return 1;
}

int calculate_spo2(){
  long int start1=millis();
  long int current=start1;
  /*while((current-start1)<60000){
    spo2=random(80,100);
    Serial.print(" SPo2:");
    Serial.println(spo2);
    current=millis();

    delay(1000);
    if(Serial.read()=='X')
      return 2;
  }*/
  spo2=random(80,100);
  //delay(5000);
  //Serial.print(" SPo2:");
  Serial.println(spo2);
  return 1;

}

int calculate_glu(){
  long int start1=millis();
  long int current=start1;
  /*while((current-start1)<60000){
    glu=random(100,120);
    Serial.print(" Glu:");
    Serial.println(glu);
    current=millis();

    delay(1000);
    if(Serial.read()=='X')
      return 2;
  }*/
  glu=random(100,120);
  //delay(5000);
  //Serial.print(" Glu:");
  Serial.println(glu);
  return 1;
  
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