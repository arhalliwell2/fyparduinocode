 /* Code designed and written by Adam Halliwell
 *  Imperial College London, EEE department, Undergraduate 2017
 *  Shortcode 'arh113'
 *  Final Year Project, Waste recognition and separation
 *  Code intended for ATMega2560 Microprocessor
 *  
 */
 #include <SPI.h>
 #include <SD.h>

 #define squareout 2
 #define metaldetectin A5
 #define motorR1 A2
 #define motorR2 A3
 #define motorL1 12
 #define motorL2 A4
 #define doorRC A8
 #define doorRO A9
 #define doorLO A11
 #define doorLC A12
 #define PHD1 A0
 #define PHD2 A1
 #define LED850 4
 #define LED1050 6
 #define LED1200 9
 #define LED1310 3
 #define LED1450 7
 #define LED1550 5
 #define LED1600 11
 #define LED1900 10
 #define LED2100 8

#define motorspeed 80 //132.0 //deg/s
#define doormovespeed 0.05

#define thresholdmetal 50
#define numofmetalreads 6 //larger = less oscillation but slower. smaller = more oscillation but faster. 3-8 is a good tradeoff.
#define numofspectrumreads 30

/////debug modes//////
#define debug 0
#define trainingpin A7
/////////////////////
char datain_char;
float datain[7]; //read-in line of values from .txt file. [0] -> 950nm, [1] -> 1050nm etc.

String towrite;
File myfile;
float readings[numofspectrumreads]={0};
 
float metalreadarray[numofmetalreads];
float metalaverage = 0;
float spectrumaverage[7] = {0};
float spectrum[7] = {0};
float proportioned[7]={0};
float abc[7]={0};

char mostlikely = ' ';

int count = 0;

//for testing
float smallest=2000;
float largest=0;

float metalread = 0;
int settlingdelay = 30;
int metaldelay = 50;

int count2 = 0; //for loops in the main loop part, where new variables shouldn#t be continuously declared.
int count3 = 0; //for counting lines in SD card file

//////////////////////////////////////////////////////////////////////////////////////////////////////////
//no need to store metallic property. Metal test is done first, and if positive then automatically sorted into metal bin.


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void setup() {
  int i;
  Serial.begin(9600);
  if (!SD.begin(53)){
    Serial.println(F("Card failed, or not present"));
    while(1){}; //hang if no SD card present
  }
  else{
    Serial.println(F("Card initialised"));
  }
  pinMode(trainingpin,INPUT);
  pinMode(PHD1,INPUT);
  pinMode(PHD2,INPUT);
  pinMode(LED850,OUTPUT);
  pinMode(LED1050,OUTPUT);
  pinMode(LED1200,OUTPUT);
  pinMode(LED1310,OUTPUT);
  pinMode(LED1450,OUTPUT);
  pinMode(LED1550,OUTPUT);
  pinMode(LED1600,OUTPUT);
  pinMode(LED1900,OUTPUT);
  pinMode(LED2100,OUTPUT);
  pinMode(squareout,OUTPUT);
  pinMode(metaldetectin,INPUT);
  pinMode(motorL1,OUTPUT);
  pinMode(motorL2,OUTPUT);
  pinMode(motorR1,OUTPUT);
  pinMode(motorR2,OUTPUT);
  Serial.println(F("calibrating metal detector..."));
  tone(squareout,23360);
  delay(metaldelay); //so the metal detector can get up to speed
  for(i=0;i<numofmetalreads;i++){
    metalreadarray[i]=analogRead(metaldetectin);
    delay(100);
  }
  tone(squareout,0);
  delay(metaldelay); //so the next IR readings arent affected
  metalaverage=getaveragemetal(metalreadarray);
  Serial.print(metalaverage); Serial.println(F(" calibrated as initial average reading"));
  Serial.println(F("calibrating spectral analyser..."));
  spectrumaverage[0]=getspectra(LED850);
  spectrumaverage[1]=getspectra(LED1050);
  spectrumaverage[2]=getspectra(LED1200);
  spectrumaverage[3]=getspectra(LED1310);
  spectrumaverage[4]=getspectra(LED1450);
  spectrumaverage[5]=getspectra(LED1550);
  spectrumaverage[6]=getspectra(LED1600);

  for (i=0;i<7;i++){
    Serial.print(F("Calibration ")); Serial.print(i);
    Serial.print(F(": ")); Serial.println(spectrumaverage[i]);
  }

  Serial.print(F("zero PHD1 reading: ")); Serial.println(PDreading(PHD1));
  Serial.print(F("zero PHD2 reading: ")); Serial.println(PDreading(PHD2));

  Serial.println(F(""));
  Serial.println(F("ready for operation"));
  if (digitalRead(trainingpin)){
    Serial.println(F("currently in training mode"));
  }
  else{
    Serial.println(F("currently in normal mode"));
  }

}    //Serial.println(metalreadarray[i]);

void loop() {
Serial.println(F("awaiting instruction"));
while (!Serial.available()){
  //door control goes here
  if (digitalRead(doorRO)){
    move_right_flap(doormovespeed);
  }
  if (digitalRead(doorRC)){
    move_right_flap(-doormovespeed);
  }
  if (digitalRead(doorLO)){
    move_left_flap(doormovespeed);
  }
  if (digitalRead(doorLC)){
    move_left_flap(-doormovespeed);
  }
  };
delay(20);
while(Serial.available()){
Serial.read();}

for (count2=0;count2<numofmetalreads-1;count2++)
  {
    checkformetal();
  }
if (checkformetal())
  {
    identifymetal();
  }
else
{
  Serial.println(F("non metallic"));
  get_spectral_readings(spectrum);
  if (debug)
  {
    Serial.println(spectrum[0]);
    Serial.println(spectrum[1]);
    Serial.println(spectrum[2]);
    Serial.println(spectrum[3]);
    Serial.println(spectrum[4]);
    Serial.println(spectrum[5]);
    Serial.println(spectrum[6]);
  }
  if (digitalRead(trainingpin)){
    towrite="";
    Serial.println(F("what material was that? h = hard plastic, s = soft plastic, c = paper/card, n = non-recyclable other"));
    while (!Serial.available()){};
    delay(20);
    towrite+=char(Serial.read());
    towrite+=',';
    towrite+=int(spectrum[0]);
    towrite+=',';
    towrite+=int(spectrum[1]);
    towrite+=',';
    towrite+=int(spectrum[2]);
    towrite+=',';
    towrite+=int(spectrum[3]);
    towrite+=',';
    towrite+=int(spectrum[4]);
    towrite+=',';
    towrite+=int(spectrum[5]);
    towrite+=',';
    towrite+=int(spectrum[6]);
    towrite+=',';
    Serial.println(towrite);
    Serial.println(F("train with this data?"));
    while (!Serial.available()){};
    delay(20);
    if (char(Serial.read())=='y'){
      myfile=SD.open("training.txt",FILE_WRITE);
      myfile.println(towrite);
      myfile.close();
      Serial.println(F("data trained"));
    }
    else{
      Serial.println(F("data not trained"));
    }
  }
  else{
    normalise(spectrum);
    mostlikely=getmostlikely(spectrum);
    translate(mostlikely);
  }
}
  
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void deposit_left(){
  move_left_flap(60);
  move_right_flap(-10);
  delay(200);
  move_right_flap(9);
  delay(1000);
  move_left_flap(-59);
}

void deposit_right(){
  move_right_flap(60);
  move_left_flap(-10);
  delay(200);
  move_left_flap(9);
  delay(1000);
  move_right_flap(-61);
}

void move_left_flap(float angle){
  if (angle>=0){
  digitalWrite(motorL1,HIGH);
  delay(1000*(abs(angle)/motorspeed));
  digitalWrite(motorL1,LOW);
  }
  if (angle<0){
  digitalWrite(motorL2,HIGH);
  delay(1000*(abs(angle)/motorspeed));
  digitalWrite(motorL2,LOW);
  }
}

void move_right_flap(float angle){
  if (angle>=0){
  digitalWrite(motorR1,HIGH);
  delay(1000*(abs(angle)/motorspeed));
  digitalWrite(motorR1,LOW);
  }
  if (angle<0){
  digitalWrite(motorR2,HIGH);
  delay(1000*(abs(angle)/motorspeed));
  digitalWrite(motorR2,LOW);
  }
}
void get_spectral_readings(float a[]){
  int i;
  a[0]=getspectra(LED850);
  a[1]=getspectra(LED1050);
  a[2]=getspectra(LED1200);
  a[3]=getspectra(LED1310);
  a[4]=getspectra(LED1450);
  a[5]=getspectra(LED1550);
  a[6]=getspectra(LED1600);
  for(i=0;i<7;i++){
    abc[i]=a[i];
  }
}

void normalise(float a[]){
  float maxi = max(max(max(a[0],a[1]),max(a[2],a[3])),max(max(a[4],a[5]),a[6]));
  int i=0;
  for (i=0;i<7;i++){
    a[i]=a[i]/maxi;
  }
}

char getmostlikely(float a[]){
  int i,j,k;
  float distance=1000;
  float temp = distance;
  char result= ' ';
  count3=0;
  myfile=SD.open("training.txt",FILE_READ);
  while (myfile.available()){
    count3++;
    datain_char=(myfile.readStringUntil(','))[0];
    datain[0]=(myfile.readStringUntil(',')).toFloat();
    datain[1]=(myfile.readStringUntil(',')).toFloat();
    datain[2]=(myfile.readStringUntil(',')).toFloat();
    datain[3]=(myfile.readStringUntil(',')).toFloat();
    datain[4]=(myfile.readStringUntil(',')).toFloat();
    datain[5]=(myfile.readStringUntil(',')).toFloat();
    datain[6]=(myfile.readStringUntil(',')).toFloat();
    normalise(datain);
    myfile.readStringUntil('\n');
    distance=0;
    proportioned[0]=datain[0]/a[0];
    proportioned[1]=datain[1]/a[1];
    proportioned[2]=datain[2]/a[2];
    proportioned[3]=datain[3]/a[3];
    proportioned[4]=datain[4]/a[4];
    proportioned[5]=datain[5]/a[5];
    proportioned[6]=datain[6]/a[6];
    
    for(j=0;j<7;j++){
      proportioned[j]=pow((proportioned[j]-1),2);
      distance+=proportioned[j];
    }
    distance=sqrt(distance);
    if (temp>distance){
      temp=distance;
      result = datain_char;
      k=count3;
    }
  }
  myfile.close();
  Serial.println(temp); //show what the probability of error was
  if (temp>0.3){
    result = 'n';
  }
  if (temp>0.2){
    Serial.println(F("high chance of error"));
  }
  Serial.print(F("closest data match on line ")); Serial.println(k);
  return result;
}

void translate(char label){
  switch(label){
    case 'h':
      Serial.println(F("hard plastic (e.g. drink bottle)"));
      deposit_right();
    break;
    case 's':
      Serial.println(F("soft plastic (e.g. shopping bag)"));
      deposit_left();
    break;
    case 'c':
      Serial.println(F("Paper or cardboard"));
      deposit_left();
    break;
    case 'n':
      Serial.println(F("non-recyclable material (e.g. wrapper, crisp packet)"));
      deposit_left();
    break;
    case 'g':
      Serial.println(F("glass (e.g. glass bottle)"));
      deposit_right();
    break;
  }
}

bool checkformetal(){
  tone(squareout,23360);
  delay(metaldelay);
  metalread = analogRead(metaldetectin);
  metalreadarray[count]=metalread;
    count++;
    if (count>=numofmetalreads){
      count=0;
    }
  /*if (metalread>largest){
    largest=metalread;
    Serial.print("new largest:");
  }
  if (metalread<smallest){
    smallest=metalread;
    Serial.print("new smallest:");
  }*/
  if (debug){
  Serial.println(getaveragemetal(metalreadarray));}
  if (abs(metalaverage-getaveragemetal(metalreadarray))>=thresholdmetal){
    return true;
  }
  else{
    return false;
  }
}

float getaveragemetal(float a[]){
  float total = 0;
  int i = 0;
  for (i=0;i<numofmetalreads;i++){
    total+=a[i];
  }
  total = total/numofmetalreads;
  return total;
}

void identifymetal(){
  if (getaveragemetal(metalreadarray)-metalaverage>100){
    Serial.println(F("strong magnetic object detected"));
    deposit_left();
    return;
  }
  if (getaveragemetal(metalreadarray)-metalaverage>50){
    Serial.println(F("possibly magnetic object detected"));
    deposit_left();
    return;
  }

  if (metalaverage-getaveragemetal(metalreadarray)>370){
    Serial.println(F("unusually low reading, something may be malfunctioning"));
    return;
  }
  if (metalaverage-getaveragemetal(metalreadarray)>320){
    Serial.println(F("highly metallic object detected (defaulted as aluminium can)"));
    deposit_right();
    return;
  }
  if (metalaverage-getaveragemetal(metalreadarray)>200){
    Serial.println(F("metal can detected"));
    deposit_right();
    return;
  }
  if (metalaverage-getaveragemetal(metalreadarray)>=thresholdmetal){
    Serial.println(F("metal detected"));
    deposit_right();
    return;
  }
  
}

float PDreading(int PD){
  int i;
  float total=0;
  for (i=0;i<numofspectrumreads;i++){
    total+=analogRead(PD);
    delay(1);
  }
  total/=numofspectrumreads;
  total=round(total);
  return total;
}

float getspectra(int wavelength){
  float reading=0;
  setLED(wavelength);
  delay(settlingdelay);
  switch(wavelength){
    case LED850:
    reading = PDreading(PHD1);
    break;
    default:
    reading = PDreading(PHD2);
    break;
  }
  setLED(0);
  delay(settlingdelay);
  return reading;
}

void setLED(int wavelength){
  digitalWrite(LED850,LOW);
  digitalWrite(LED1050,LOW);
  digitalWrite(LED1200,LOW);
  digitalWrite(LED1310,LOW);
  digitalWrite(LED1450,LOW);
  digitalWrite(LED1550,LOW);
  digitalWrite(LED1600,LOW);
  digitalWrite(LED1900,LOW);
  digitalWrite(LED2100,LOW);
  switch(wavelength){
    case LED850:
    digitalWrite(LED850,HIGH);
    break;
    case LED1050:
    digitalWrite(LED1050,HIGH);
    break;
    case LED1200:
    digitalWrite(LED1200,HIGH);
    break;
    case LED1310:
    digitalWrite(LED1310,HIGH);
    break;
    case LED1450:
    digitalWrite(LED1450,HIGH);
    break;
    case LED1550:
    digitalWrite(LED1550,HIGH);
    break;
    case LED1600:
    digitalWrite(LED1600,HIGH);
    break;
    case LED1900:
    digitalWrite(LED1900,HIGH);
    break;
    case LED2100:
    digitalWrite(LED2100,HIGH);
    break;
    default:
    break;
  }
}

