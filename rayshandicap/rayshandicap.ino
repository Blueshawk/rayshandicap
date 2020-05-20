/*Ray's super simple telescope OTA cover for indi
  Parts:
  Ardunio nano     -- Or pretty much any Arduino or even a wifi module(still needs power)
  Airplane servo   -- I used a metal 20kg one from amazon.
  Some stiff plastic sheeting, old coffee can lids etc. --to make the hat.
  something to make the cherry picker arm that holds it.
  Some massive wire ties, or other attachment device,
  or a permanant servo mount screwed to an adjustable dew cap
  Very likely hope and some Ductape.

The library assumes you are using 9 and it didn't work when I moved it to 5, so use pin 9 for the pwm to the servo.

O = open
C = close
F = firmware version
M+ = add 1 to open
M- = subtract 1 from open
m+ = add 1 to close
m- = subtract 1 from close
Mxxx = set open to xxx
mxxx = set close to xxx

todo - 
set up close/open rom storage.
add motor reverse and store.

*/

#include <Servo.h>

Servo servo;  // create servo object to control a servo
int pos;
int open = 120;
int close = 0;
bool status = 0;

//serial protocol variables
#define MAXCOMMAND 8
char inChar;
char cmd[MAXCOMMAND];
char param[MAXCOMMAND];
char line[MAXCOMMAND];
int eoc = 0;
int idx = 0;

//Begin Called Functions (putting them here makes arduino IDE sort out header files for you)
void moveToOpen() {
  for (pos = close; pos <= open ; pos += 1) { // goes from 0 degrees to 180 degrees in steps of 1 degree
    servo.write(pos);                // tell servo to go to position in variable 'pos'
    delay(15);                        // this slows down the servo so the cap isn't swatting meteors
  }
  status = 1;
  Serial.print("open");
}
void moveToClose() {
  for (pos = open; pos >= close ; pos -= 1) { // goes from 180 degrees to 0 degrees
    // in steps of 1 degree
    servo.write(pos);              // tell servo to go to position in variable 'pos'
    delay(15);                       // this slows down the servo so the cap isn't swatting flies
  }
  status = 0;
  Serial.print("close");
}

long hexstr2long(char *line) {
  long ret = 0;
  ret = strtol(line, NULL, 16);
  return (ret);
}
void SerialEvent(){
//void doSerialCmd() {
  // read the command until the tercloseating # character
  while (Serial.available() && !eoc) {
    inChar = Serial.read();
    if (inChar != '#' && inChar != ':') {
      line[idx++] = inChar;
      if (idx >= MAXCOMMAND) {
        idx = MAXCOMMAND - 1;
      }
    }
    else {
      if (inChar == '#') {
        eoc = 1;
      }
    }
  }
  // process the command we got
  if (eoc) {
    memset(cmd, 0, MAXCOMMAND);
    memset(param, 0, MAXCOMMAND);

    int len = strlen(line);
    if (len >= 2) {
      strncpy(cmd, line, 2);
    }

    if (len > 2) {
      strncpy(param, line + 2, len - 2);
    }

    memset(line, 0, MAXCOMMAND);
    eoc = 0;
    idx = 0;


    // Open
    if (!strcasecmp(cmd, "O") || !strcasecmp(cmd, "o")) {
      moveToOpen();
    }

    // Close
    if (!strcasecmp(cmd, "C") || !strcasecmp(cmd, "c")) {
      moveToClose();
    }

    // get firmware version (handshaking?)
    if (!strcasecmp(cmd, "F") || !strcasecmp(cmd, "f")) {
    }

    // get status
    if (!strcasecmp(cmd, "S") || !strcasecmp(cmd, "s")) {
      if (status = 0) {
        Serial.print("close");
      }
      if (status = 1) {
        Serial.print("open");
      }

    }

  }
} 
 //end serialevent

void setup() {
  Serial.begin(9600);
  servo.attach(9);  // PWM to servo. Must use 9, per servo library.
moveToOpen();
delay(1000);
moveToClose();
}

void loop() {
  //doSerialCmd();


}
