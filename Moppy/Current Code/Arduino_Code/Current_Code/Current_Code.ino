//set to Arduino gen uno, usbmodem1421
#include <TimerOne.h>

boolean firstRun = true; // Used for one-run-only stuffs;

//First pin being used for floppies, and the last pin.  Used for looping over all pins.
const byte FIRST_PIN = 2;
const byte PIN_MAX = 17;
#define RESOLUTION 40 //Microsecond resolution for notes

int byte1 = 0;
int byte2 = 0;
int byte3 = 0;
unsigned int byte3_full = 0;


/*NOTE: Many of the arrays below contain unused indexes.  This is 
 to prevent the Arduino from having to convert a pin input to an alternate
 array index and save as many cycles as possible.  In other words information 
 for pin 2 will be stored in index 2, and information for pin 4 will be 
 stored in index 4.*/


/*An array of maximum track positions for each step-control pin.  Even pins
 are used for control, so only even numbers need a value here.  3.5" Floppies have
 80 tracks, 5.25" have 50.  These should be doubled, because each tick is now
 half a position (use 158 and 98).
 */
byte MAX_POSITION[] = {
  0,0,158,0,158,0,158,0,158,0,158,0,158,0,158,0,158,0};

//Array to track the current position of each floppy head.  (Only even indexes (i.e. 2,4,6...) are used)
byte currentPosition[] = {
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

/*Array to keep track of state of each pin.  Even indexes track the control-pins for toggle purposes.  Odd indexes
 track direction-pins.  LOW = forward, HIGH=reverse
 */
int currentState[] = {
  0,0,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW,LOW
};

//Current period assigned to each pin.  0 = off.  Each period is of the length specified by the RESOLUTION
//variable above.  i.e. A period of 10 is (RESOLUTION x 10) microseconds long.
unsigned int currentPeriod[] = {
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
};

//Current tick
unsigned int currentTick[] = {
  0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 
};



//Setup pins (Even-odd pairs for step control and direction
void setup(){
  pinMode(13, OUTPUT);// Pin 13 has an LED connected on most Arduino boards
  pinMode(2, OUTPUT); // Step control 1
  pinMode(3, OUTPUT); // Direction 1
  pinMode(4, OUTPUT); // Step control 2
  pinMode(5, OUTPUT); // Direction 2
  pinMode(6, OUTPUT); // Step control 3
  pinMode(7, OUTPUT); // Direction 3
  pinMode(8, OUTPUT); // Step control 4
  pinMode(9, OUTPUT); // Direction 4
  pinMode(10, OUTPUT); // Step control 5
  pinMode(11, OUTPUT); // Direction 5
  pinMode(12, OUTPUT); // Step control 6
  pinMode(13, OUTPUT); // Direction 6
  pinMode(14, OUTPUT); // Step control 7
  pinMode(15, OUTPUT); // Direction 7
  pinMode(16, OUTPUT); // Step control 8
  pinMode(17, OUTPUT); // Direction 8

  //With all pins setup, let's do a first run reset
  resetAll();
  delay(1000);
	
  Timer1.initialize(RESOLUTION); // Set up a timer at the defined resolution
  Timer1.attachInterrupt(tick); // Attach the tick function

  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  establishContact();
}


void loop()
{
  //Only read if we have 3 bytes waiting
  if (Serial.available() > 2){
    //Watch for special 100-message to act on
    if (Serial.peek() == 100) 
    {
      //Clear the peeked 100 byte so we can get the following data packet
      Serial.read();
      byte byte2 = Serial.read();
      Serial.print('in here');


      //This isn't used for anything right now, always set to 0
      //byte byte3 = Serial.read();
      
      switch(byte2) {
        case 0: resetAll(); break;
        case 1: Serial.print(' case 1 '); break;  //Connected
        case 2: Serial.print(' case 2 '); break;  //Disconnected
        case 3: Serial.print(' case 3 '); break;  //Sequence starting
        case 4: Serial.print(' case 4 '); break;  //Sequence stopping
        default: resetAll(); break;
      }      
      //Flush any remaining messages.
      while(Serial.available() > 0) { Serial.read(); }
    } 
    else{ //if first byte is not special message 100,
      /*
      currentPeriod[Serial.read()] = (Serial.read() << 8) | Serial.read();  //reads three bytes: #1 - location of currentPeriod to write to
      //#2 
      */
      Serial.println("Enter Function.");
      byte1 = Serial.read();
      byte2 = Serial.read();
      byte3 = Serial.read();
      byte byte3_high = 0;
      byte3_full = 0; //clear byte3 value   

      if (byte3 != 0)
      { //if last byte is > 0 
           byte byte3_high = Serial.read(); //reassign byte3_high
           
           byte3_full = byte3_full + byte3;
           byte3_full << 8;
           byte3_full = byte3_full + byte3_high;
      } 
      
       currentPeriod[byte1]= (byte2 << 8) | byte3_full; 
        Serial.println(byte1);          
          Serial.println(byte2);
          Serial.println(byte3_full);
  
          
      }//end else
} //end if serial.available >2
}


void establishContact() {
  while (Serial.available() <= 0) {
    Serial.println("0,0,0");   // send an initial string
    delay(300);
  }
}
/*
Called by the timer interrupt at the specified resolution.
 */
void tick()
{
  /* 
   If there is a period set for control pin 2, count the number of
   ticks that pass, and toggle the pin if the current period is reached.
   */

  if (currentPeriod[2]>0){
    currentTick[2]++;         //Serial.print('currentTick2 ++');

    if (currentTick[2] >= currentPeriod[2]){
      togglePin(2,3);  //Serial.print('toggle2,3');
      currentTick[2]=0;
    }
  }
  if (currentPeriod[4]>0){
    currentTick[4]++; //Serial.print('currentTick4 ++');

    if (currentTick[4] >= currentPeriod[4]){
      togglePin(4,5); //Serial.print('toggle4,5');
      currentTick[4]=0;
    }
  }
  if (currentPeriod[6]>0){
    currentTick[6]++; // Serial.print('currentTick6 ++');
    if (currentTick[6] >= currentPeriod[6]){
      togglePin(6,7); // Serial.print('toggle6,7');
      currentTick[6]=0;
    }
  }
  if (currentPeriod[8]>0){
    currentTick[8]++; // Serial.print('currentTick8 ++');
    if (currentTick[8] >= currentPeriod[8]){
      togglePin(8,9);  // Serial.print('toggle8,9');
      currentTick[8]=0;
    }
  }
  if (currentPeriod[10]>0){
    currentTick[10]++;//  Serial.print('currentTick10 ++');
    if (currentTick[10] >= currentPeriod[10]){
      togglePin(10,11); // Serial.print('toggle1011');
      currentTick[10]=0;
    }
  }
  if (currentPeriod[12]>0){
    currentTick[12]++; // Serial.print('currentTick12 ++');
    if (currentTick[12] >= currentPeriod[12]){
      togglePin(12,13); // Serial.print('toggle12,13');
      currentTick[12]=0;
    }
  }
  if (currentPeriod[14]>0){
    currentTick[14]++; //Serial.print('currentTick14 ++');
    if (currentTick[14] >= currentPeriod[14]){
      togglePin(14,15); //Serial.print('toggle14,15');
      currentTick[14]=0;
    }
  }
  if (currentPeriod[16]>0){
    currentTick[16]++; //Serial.print('currentTick16 ++');
    if (currentTick[16] >= currentPeriod[16]){
      togglePin(16,17); //Serial.print('toggle16,17');
      currentTick[16]=0;
    }
  }

}

void togglePin(byte pin, byte direction_pin) {

  //Switch directions if end has been reached
  if (currentPosition[pin] >= MAX_POSITION[pin]) {
    currentState[direction_pin] = HIGH; //Serial.print('we need to switch directions');
    digitalWrite(direction_pin,HIGH); 
  } 
  else if (currentPosition[pin] <= 0) {
    currentState[direction_pin] = LOW; 
    digitalWrite(direction_pin,LOW);
  }

  //Update currentPosition
  if (currentState[direction_pin] == HIGH){
    currentPosition[pin]--;  //Serial.print('update current position');
  } 
  else {
    currentPosition[pin]++;
  }

  //Pulse the control pin
  digitalWrite(pin,currentState[pin]);
  currentState[pin] = ~currentState[pin];
}


//
//// UTILITY FUNCTIONS
//

//Not used now, but good for debugging...
void blinkLED(){
  digitalWrite(13, HIGH); // set the LED on
  delay(250);              // wait for a second
  digitalWrite(13, LOW); 
}

//For a given controller pin, runs the read-head all the way back to 0
void reset(byte pin)
{
  //Serial.print('RESET function');
  digitalWrite(pin+1,HIGH); // Go in reverse
  for (byte s=0;s<MAX_POSITION[pin];s+=2){ //Half max because we're stepping directly (no toggle)
    digitalWrite(pin,HIGH);
    digitalWrite(pin,LOW);
    delay(5);
  }
  currentPosition[pin] = 0; // We're reset.
  digitalWrite(pin+1,LOW);
  currentPosition[pin+1] = 0; // Ready to go forward.
}

//Resets all the pins
void resetAll(){


  //Serial.print('RESET ALL');

  // Old one-at-a-time reset
  //for (byte p=FIRST_PIN;p<=PIN_MAX;p+=2){
  //  reset(p);
  //}

  //Stop all notes (don't want to be playing during/after reset)
  for (byte p=FIRST_PIN;p<=PIN_MAX;p+=2){
    currentPeriod[p] = 0; // Stop playing notes
  }

  // New all-at-once reset
  for (byte s=0;s<80;s++){ // For max drive's position
    for (byte p=FIRST_PIN;p<=PIN_MAX;p+=2){
      digitalWrite(p+1,HIGH); // Go in reverse
      digitalWrite(p,HIGH);
      digitalWrite(p,LOW);
    }
    delay(5);
  }

  for (byte p=FIRST_PIN;p<=PIN_MAX;p+=2){
    currentPosition[p] = 0; // We're reset.
    digitalWrite(p+1,LOW);
    currentState[p+1] = 0; // Ready to go forward.
  }

}
