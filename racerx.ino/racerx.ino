
int rows[8] = {5, 3, 7, 4, 16, 17, 18, 6};

int map1[256] = {
  1,0,0,0,0,0,0,1, // 1
  1,0,0,0,0,0,0,1,
  1,0,0,0,0,0,0,1,
  1,0,0,0,0,0,0,1,
  1,0,0,0,0,0,0,1,
  1,0,0,0,0,0,0,1,
  1,0,0,0,0,0,0,1,
  1,0,0,0,1,1,0,1, // 8
  1,0,0,0,0,0,0,1,
  1,1,0,0,0,0,0,1,
  1,1,1,1,0,0,0,1,
  1,1,1,0,0,0,0,1,
  1,0,0,0,0,0,0,1,
  1,0,0,0,0,0,1,1,
  1,0,0,0,0,1,1,1,
  1,0,0,0,0,0,1,1, // 16
  1,0,0,0,0,0,0,1,
  1,1,1,1,0,0,0,1,
  1,0,1,0,0,0,0,1,
  1,0,1,1,1,0,0,1,
  1,0,0,0,1,0,0,1,
  1,0,0,1,0,0,0,1,
  1,0,1,0,0,0,0,1,
  1,1,0,0,0,0,1,1, // 24
  1,0,0,0,0,1,1,1,
  1,0,0,0,0,0,1,1,
  1,0,0,0,0,0,0,1,
  1,0,0,0,0,0,0,1,
  1,0,0,0,0,0,0,1,
  1,0,0,0,0,0,0,1,
  1,0,0,0,0,0,0,1,
  1,0,0,0,0,0,0,1
};

const int leftbtn = 0;
const int rightbtn = 2;

//speed of track
int level = 200;
int curCol = 0;
//starting location for car
int carLoc = 3;
int mapCnt = 0;
int rowidx = 0;
int refresh = 1000;
int pin;
int counter = 0;
int throbcount = 0;
boolean isinc = true;
boolean crash = false;

//debounce variables
int lastLeftBtnState = LOW;
int lastRightBtnState = LOW;
long prevLeftDebounce = 0;
long prevRightDebounce = 0;
long debounceDelay = 50;

void setup() {
  //Serial.begin(9600);
  pinMode(0,INPUT);
  pinMode(2,INPUT);
  //set pins to output
  DDRD = DDRD | B11111000; // pins 7,6,5,4,3,x,x,x
  DDRB = B00111111;        // pins x,x,13,12,11,10,9,8
  DDRC = B00011111;        // pins x,x,x,a4,a3,a2,a1,a0
}

void loop() {
  if(!crash){
    findCar();
    moveRoad();
  }else{
   throbber();
   //reset defaults
   level = 200;
   carLoc = 3;
   rowidx = 0;
  }
}
/*
  Read the buttons to locate the position of the car.  
*/
void findCar(){
  int leftRead = digitalRead(leftbtn);
  int rightRead = digitalRead(rightbtn);
  //debounce button reads
  if(leftRead != lastLeftBtnState)
    prevLeftDebounce = millis();
    
  if(rightRead != lastRightBtnState)
    prevRightDebounce = millis();
  
  if((millis() - prevLeftDebounce) > debounceDelay){
    carLoc--;
  }
  if((millis() - prevRightDebounce) > debounceDelay){
    carLoc++; 
  }
  
  if(carLoc > 7) carLoc = 7;
  if(carLoc < 0) carLoc = 0;
}
/*
  Step through the map array
*/
void moveRoad(){
  for(int i=0; i<level; i++){
    PORTB = B00000000;
    PORTC = B00011100;
    PORTD = B11111000;
   
    delayMicroseconds(10);
    setColOff(curCol);
    readRows();
    setColOn(curCol);
    delayMicroseconds(800);
    
    //move to next col or reset if at last
    curCol++;
    if(curCol > 7)
      curCol = 0;
  }
  //move to next row in matrix
  rowidx++;
  if(rowidx > 25){
    rowidx = 0;
    //successfull loop so increase speed
    level -= 25;
  }
}
/*
  light the leds used for the car
*/
void drawCar(int pin){
  if(curCol == carLoc){
    //select port for carLoc
    setColOn(curCol);
    
    if(pin == 5 || pin == 3)
      PORTD ^= (1 << pin);
    delayMicroseconds(200);
  }
}
/*
  Read the rows for each loop
*/
void readRows(){
  int r = 0;
  for(int i=rowidx; i<rowidx+8; i++){
    //Get the arduino pin to update
    pin = rows[r];
    drawCar(pin);
    //Check the map array to see if the current row
    //and the column need to be high or low, then
    //push to the current register.
    if(map1[(i*8)+curCol]){
      if(digitalRead(pin) == LOW){
        crash = true;
        break;
      }
      if(pin <= 7) { //portd
        PORTD &=~ (1 << pin);
      }else if(pin >= 16){
        //subtract by 14 to find the port register
        PORTC &=~ (1 << pin-14); 
      }
    }else{
      if(pin <= 7) { //portd
        PORTD ^= (0 << pin);
      }else if(pin >= 16){
        //subtract by 14 to find the port register
        PORTC ^= (0 << pin-14);
      }
    }
    r++;
  }
}
//Sets desired column pin to LOW
void setColOff(int col){
  if(col == 2)
    PORTC &= ~(0 << 0); 
  else if(col == 4)
    PORTC &= ~(0 << 1); 
  else if(col == 0)
    PORTB &= ~(0 << 3); 
  else if(col == 1)
    PORTB &= ~(0 << 5); 
  else if(col == 3)
    PORTB &= ~(0 << 0); 
  else if(col == 5)
    PORTB &= ~(0 << 2); 
  else if(col == 6)
    PORTB &= ~(0 << 4); 
  else if(col == 7)
    PORTB &= ~(0 << 1); 
}
//Sets desired column pin to HIGH
void setColOn(int col){
  if(col == 2)
    PORTC ^= (1 << 0); 
  else if(col == 4)
    PORTC ^= (1 << 1); 
  else if(col == 0)
    PORTB ^= (1 << 3); 
  else if(col == 1)
    PORTB ^= (1 << 5); 
  else if(col == 3)
    PORTB ^= (1 << 0);
  else if(col == 5)
    PORTB ^= (1 << 2); 
  else if(col == 6)
    PORTB ^= (1 << 4); 
  else if(col == 7)
    PORTB ^= (1 << 1); 
}
/*
 Runs a pulsing led effect to signal a crash.
*/
void throbber(){
  if (throbcount > 500){
    crash = false;
    throbcount = 0;
    counter = 0;
  }
    
  PORTB = B000000;
  PORTC = B11100;
  PORTD = B11111;
  delayMicroseconds(counter);
  PORTB = B111111;
  PORTC = B00011;
  PORTD = B00000;
  delayMicroseconds(1000);
  
  if(isinc){
    counter += 100;
    if(counter > 9000) isinc = false;  
  }else{
    counter -= 100;
    if(counter < 0) isinc = true;
  } 
  throbcount++;
}
