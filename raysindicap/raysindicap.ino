/*Ray's super simple telescope OTA cover for indi
  Parts:
  Ardunio nano     -- Or pretty much any Arduino or even a wifi module(still needs power)
  Airplane servo   -- I used a metal 20kg one from amazon.
  Some stiff plastic sheeting, old coffee can lids etc. --to make the hat.
  something to make the cherry picker arm that holds it.
  Some massive wire ties, or other attachment device,
  or a permanant servo mount screwed to an adjustable dew cap
  Very likely hope and some Ductape.

*/

#include <Servo.h>

Servo servo;  // create servo object to control a servo
int pos;
int max = 120;
int min = 0;

//serial protocol variables
#define MAXCOMMAND 8
char inChar;
char cmd[MAXCOMMAND];
char param[MAXCOMMAND];
char line[MAXCOMMAND];
int eoc = 0;
int idx = 0;

//Begin Called Functions (putting them here makes arduino IDE sort out header files for you)
void moveToMax() {
  for (pos = min; pos <= max ; pos += 1) { // goes from 0 degrees to 180 degrees in steps of 1 degree
    servo.write(pos);                // tell servo to go to position in variable 'pos'
    delay(15);                        // this slows down the servo so the cap isn't swatting meteors
  }
Serial.print ("open");
}
void moveToMin() {
  for (pos = max; pos >= min ; pos -= 1) { // goes from 180 degrees to 0 degrees
    // in steps of 1 degree
    servo.write(pos);              // tell servo to go to position in variable 'pos'
    delay(15);                       // this slows down the servo so the cap isn't swatting flies
  }
Serial.print ("close");
}

long hexstr2long(char *line) {
  long ret = 0;
  ret = strtol(line, NULL, 16);
  return (ret);
}

void serialEvent() { //serialEvent is called automatically any time serial data is recieved.
  // read the command until the terminating # character
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
  if (!strcasecmp(cmd, "O")||!strcasecmp(cmd, "o")) {
    moveToMax();  
    }

  // Close
  if (!strcasecmp(cmd, "C")||!strcasecmp(cmd, "c")) {
    moveToMin();  
    }

  // get firmware version (handshaking?)
  if (!strcasecmp(cmd, "F")||!strcasecmp(cmd, "f")) {
    }

}
} // end loop

void setup() {
  Serial.begin(9600);
  servo.attach(9);  // assigns a pin to the pwm control line on the servo.

}

void loop() {

}
