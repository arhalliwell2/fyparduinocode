 #define squareout 2
 #define metaldetectin A5
 #define motorR1 A2
 #define motorR2 A3
 #define motorL1 12
 #define motorL2 A4
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

#define thresholdmetal 50
#define numofmetalreads 6 //larger = less oscillation but slower. smaller = more oscillation but faster. 3-8 is a good tradeoff.
#define numofspectrumreads 30

/////debug mode//////
#define debug 1
/////////////////////
struct LEDdata {
  char label;
  float norm850;
  float norm1050;
  float norm1200;
  float norm1310;
  float norm1450;
  float norm1550;
  float norm1600;
};


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

#define numofdatabaseentries 44
LEDdata database[numofdatabaseentries];

//////////////////////////////////////////////////////////////////////////////////////////////////////////
//no need to store metallic property. Metal test is done first, and if positive then automatically sorted into metal bin.
void setup_database(){ //labels: h = hard plastic, s = soft plastic, c = card/paper, n = non-recyclable other, g= glass.
  append_database(0,'h',1023,266,332,249,342,378,268);  // drink bottle                           /plastic
  append_database(1,'h',1023,213,181,172,173,172,174);  // drink bottle filled with water         /plastic
  append_database(2,'h',1023,252,286,248,301,402,262);  // drink bottle                           /plastic
  append_database(3,'h',361,178,175,175,173,173,172);   // drink bottle containing orange juice   /plastic
  append_database(4,'c',699,185,189,195,189,200,183);   // napkin (white)                         /non-recyclable
  append_database(5,'s',1023,204,233,246,252,323,224);  // shopping bag                           /non-recyclable
  append_database(6,'s',1023,236,287,298,304,411,252);  // shopping bag                           /non-recyclable
  append_database(7,'s',216,174,177,177,179,182,177);  // shopping bag                           /non-recyclable
  append_database(8,'s',248,174,177,179,178,182,178);   // shopping bag                           /non-recyclable
  append_database(9,'c',344,177,180,181,179,181,178);   // A4 paper                               /paper
  append_database(10,'c',124,174,174,174,173,173,173);  // folded A4 paper                        /paper
  append_database(11,'c',153,173,174,173,173,173,174);  // cardboard sheet                        /paper
  append_database(12,'c',102,173,172,173,173,172,172);  // cardboard box                          /paper
  append_database(13,'s',1023,246,329,298,329,504,264); // component bag                          /non-recyclable
  append_database(14,'s',1023,262,318,240,333,387,254); // component bag                          /non-recyclable
  append_database(15,'h',1023,249,286,292,290,405,246); // hard plastic                           /plastic
  append_database(16,'h',1023,276,334,286,328,494,267); // clear acrylic                          /plastic
  append_database(17,'h',806,189,194,207,203,245,195);  // pink acrylic                           /plastic
  append_database(18,'h',516,185,188,201,198,240,193);  // blue acrylic                           /plastic
  append_database(19,'h',1023,272,321,288,310,472,260); // thick clear acrylic                    /plastic
  append_database(20,'h',1023,275,344,257,362,457,274); // thermoplastic                          /plastic
  append_database(21,'h',1023,261,367,430,336,509,290); // thermoplastic                          /plastic
  append_database(22,'s',1023,264,322,293,325,466,264); // bag                                    /non-recyclable
  append_database(23,'h',1023,270,351,293,351,501,274); // thermoplastic                          /plastic
  append_database(24,'s',184,175,179,183,192,224,190);  // biscuit wrapper                        /non-recyclable
  append_database(25,'n',90,173,172,172,172,173,172);   // crisp packet                           /non-recyclable
  append_database(26,'h',1023,258,320,290,321,412,269); // starbucks plastic cup                  /plastic
  append_database(27,'h',1023,262,317,274,315,401,256); // starbucks plastic cup                  /plastic
  append_database(28,'h',404,181,181,187,185,196,183);  // solvent bottle                         /plastic
  append_database(29,'h',514,183,185,194,196,217,189);  // solvent bottle                         /plastic
  append_database(30,'g',1023,262,294,265,234,279,228); // glass bottle                           /glass
  append_database(31,'g',1023,316,339,256,277,437,253); // glass bottle                           /glass
  append_database(32,'h',1023,271,341,310,324,319,296); // thermoplastic                          /plastic
  append_database(33,'h',1023,249,294,196,299,279,250); // drink bottle                           /plastic
  append_database(34,'h',1023,230,303,383,245,574,319); // drink bottle                           /plastic
  append_database(35,'h',1023,355,316,315,492,368,368); // drink bottle                           /plastic
  append_database(36,'h',1023,228,284,412,261,424,239); // drink bottle                           /plastic
  append_database(37,'h',1023,245,307,359,288,384,248); // drink bottle                           /plastic
  append_database(38,'h',1023,279,304,372,323,444,244); // drink bottle                           /plastic
  append_database(39,'h',1023,241,302,452,252,530,206); // drink bottle                           /plastic
  append_database(40,'h',1023,215,234,294,222,326,208); // drink bottle                           /plastic
  append_database(41,'s',1023,192,208,225,222,250,212); // shopping bag                           /non-recyclable
  append_database(42,'s',1009,184,190,199,199,206,192); // shopping bag                           /non-recyclable
  append_database(43,'s',1023,235,333,500,272,467,213);
}
void append_database(int num, char a, float b, float c, float d, float e, float f, float g, float h){
  float maxi = max(max(max(b,c),max(d,e)),max(max(f,g),h));
  database[num].label=a;
  database[num].norm850=b/maxi;
  database[num].norm1050=c/maxi;
  database[num].norm1200=d/maxi;
  database[num].norm1310=e/maxi;
  database[num].norm1450=f/maxi;
  database[num].norm1550=g/maxi;
  database[num].norm1600=h/maxi;
}
void setup() {
  int i;
  move_left_flap(0);
  move_right_flap(0);
  deposit_left();
  Serial.begin(9600);
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
  Serial.println(F("Setting up database..."));
  setup_database();
  Serial.println(F("Database initialised"));
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

}    //Serial.println(metalreadarray[i]);

void loop() {/*
Serial.println(F("awaiting instruction"));
while (!Serial.available()){};
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
  normalise(spectrum);
  mostlikely=getmostlikely(spectrum);
  translate(mostlikely);
}*/
  
}

void deposit_left(){
  move_left_flap(60);
  move_right_flap(-10);
  delay(200);
  move_right_flap(9);
  delay(1000);
  move_left_flap(-59);
}

void deposit_right(){
  
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
  int i,j;
  float distance=1000;
  float temp = distance;
  char result= ' ';
  for (i=0;i<numofdatabaseentries;i++){
    distance=0;
    proportioned[0]=database[i].norm850/a[0];
    proportioned[1]=database[i].norm1050/a[1];
    proportioned[2]=database[i].norm1200/a[2];
    proportioned[3]=database[i].norm1310/a[3];
    proportioned[4]=database[i].norm1450/a[4];
    proportioned[5]=database[i].norm1550/a[5];
    proportioned[6]=database[i].norm1600/a[6];
    
    for(j=0;j<7;j++){
      proportioned[j]=pow((proportioned[j]-1),2);
      distance+=proportioned[j];
    }
    distance=sqrt(distance);
    if (temp>distance){
      temp=distance;
      result = database[i].label;
      Serial.println(i);
    }
  }
  Serial.println(temp); //show what the probability of error was
  if (temp>0.3){
    result = 'n';
  }
  if (temp>0.2){
    Serial.print(F("append_database(##,'")); Serial.print(result); Serial.print(F("',")); Serial.print(abc[0],0); Serial.print(F(",")); Serial.print(abc[1],0); Serial.print(F(",")); Serial.print(abc[2],0); Serial.print(F(",")); Serial.print(abc[3],0); Serial.print(F(",")); Serial.print(abc[4],0); Serial.print(F(",")); Serial.print(abc[5],0); Serial.print(F(",")); Serial.print(abc[6],0); Serial.println(F(");"));
  }
  return result;
}

void translate(char label){
  switch(label){
    case 'h':
      Serial.println(F("hard plastic (e.g. drink bottle)"));
    break;
    case 's':
      Serial.println(F("soft plastic (e.g. shopping bag)"));
    break;
    case 'c':
      Serial.println(F("Paper or cardboard"));
    break;
    case 'n':
      Serial.println(F("non-recyclable material (e.g. wrapper, crisp packet)"));
    break;
    case 'g':
      Serial.println(F("glass (e.g. glass bottle)"));
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
    return;
  }
  if (getaveragemetal(metalreadarray)-metalaverage>50){
    Serial.println(F("possibly magnetic object detected"));
    return;
  }

  if (metalaverage-getaveragemetal(metalreadarray)>370){
    Serial.println(F("unusually low reading, something may be malfunctioning"));
    return;
  }
  if (metalaverage-getaveragemetal(metalreadarray)>320){
    Serial.println(F("highly metallic object detected (defaulted as aluminium can)"));
    return;
  }
  if (metalaverage-getaveragemetal(metalreadarray)>200){
    Serial.println(F("metal can detected"));
    return;
  }
  if (metalaverage-getaveragemetal(metalreadarray)>=thresholdmetal){
    Serial.println(F("metal detected"));
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

