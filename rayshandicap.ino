/*Ray's super simple telescope OTA cover for indi
   This one works with the existing Indi flip-flat driver
  Parts:
  Ardunio nano     -- Or pretty much any Arduino or even a wifi module(still needs power)
  Airplane servo   -- I used a metal 20kg one from amazon.
  Some stiff plastic sheeting, old coffee can lids etc. --to make the hat.
  something to make the cherry picker arm that holds it.
  Some massive wire ties, or other attachment device,
  or a permanant servo mount screwed to an adjustable dew cap
  Very likely hope and some Ductape.

  The library assumes you are using 9 and it didn't work when I moved it to 5, so use pin 9 for the pwm to the servo.
  // -=ALWAYS MOVE TO OPEN POSITION BEFORE POWERING=- The unit has no internal speed control and will assume maximum.

  todo -
  set up close/open rom storage. I think this might make startup less hectic.
  When the initial position surges at full spped it draws heavy current.
*/

#include <Servo.h>

/*Hardware ID codes are used to set up drivers. Only 99 works with indi
  10 = Flat-Man_XL
  15 = Flat-Man_L
  19 = Flat-Man
  98 = Flip-Mask/Remote Dust Cover
  99 = Flip-Flat */

//indi driver apparently turns off the cover if not 99

#define DEVICE "99"

Servo servo;  // create servo object to control a servo
int pos = 0;                                   //Current angle when read from servo.
int open = 80;                                 //angle when open
int close = 0;                                 //angle when closed
long dly = 15;                                 //delay for motor
char brightness[4] = "128";
short cover = 1;                               //status of cover 0=niether 2=open 1=closed
short light = 0;                               //status of light
long motor = 0;                                //status of motor in motion(indi uses this)

//Begin Called Functions (putting them here makes arduino IDE sort out header files for you)
void moveToOpen() {
  pos = servo.read();
  for (pos ; pos <= open ; pos += 1) {          // goes from "close" degrees to "open" degrees in steps of 1 degree
    servo.write(pos);                           // tell servo to go to position in variable 'pos'
    delay(dly);                                 // this slows down the servo so the cap isn't swatting meteors
    cover = 0;                                  //cover in motion
    digitalWrite(HIGH, LED_BUILTIN);
  }
  Serial.print("*O" DEVICE "000\n");
  cover = 2;
  motor = 0;
  digitalWrite(LOW, LED_BUILTIN);
}
void moveToClose() {
  pos = servo.read();
  for ( pos ; pos >= close ; pos -= 1) {                   // goes from 180 degrees to 0 degrees
    // in steps of 1 degree
    servo.write(pos);                                      // tell servo to go to position in variable 'pos'
    delay(dly);                                            // this slows down the servo so the cap isn't swatting meteors
    cover = 0;                                             //cover in motion
    digitalWrite(HIGH, LED_BUILTIN);
  }
  Serial.print("*C" DEVICE "000\n");
  cover = 1;
  motor = 0;
  digitalWrite(LOW, LED_BUILTIN);
}


void serialEvent() {                                       //SerialEvent() is automatically called when serial port recieves data
  if (Serial.available()) {
    char buffer[16];
    memset(buffer, 0, sizeof(buffer));
    Serial.readBytesUntil("\n", buffer, sizeof(buffer));   //get command from serial

    if (!strcmp(buffer, ">P000\n")) {                      //Ping - Connect and return device type
      Serial.print("*P" DEVICE "000\n");                   //Indi_flipflat only uses 99 or non cover
    }

    else if (!strcmp(buffer, ">O000\n")) {                //Open cover
      motor = 2000;
      cover = 0;                                          //set status cover in motion during move
      moveToOpen();
    }

    else if (!strcmp(buffer, ">C000\n")) {                //Close Cover
      motor = 2000;
      cover = 0;
      moveToClose();
    }

    else if (!strcmp(buffer, ">L000\n")) {                //Light on(simulated)
      light = 1;
      Serial.print("*L" DEVICE "000\n");
    }

    else if (!strcmp(buffer, ">D000\n")) {                //light off(simulated)
      light = 0;
      Serial.print("*D" DEVICE "000\n");
    }

    else if (!strncmp(buffer, ">B", 2)) {                 //Set Brightness(simulated)
      strncpy(brightness, buffer + 2, 3);
      Serial.print("*B" DEVICE);
      Serial.print(brightness);
      dly == long(brightness);
      //Serial.print("\n");
    }

    else if (!strcmp(buffer, ">J000\n")) {                //Get Brightness(simulated)
      Serial.print("*J" DEVICE);
      Serial.print(brightness);
      Serial.print("\n");
    }

    else if (!strcmp(buffer, ">S000\n")) {                //Get Device status
      Serial.print("*S" DEVICE);
      Serial.print(motor > 0 ? 1 : 0);
      Serial.print(light);
      Serial.print(motor > 0 ? 0 : cover);
      Serial.print("\n");
    }

    else if (!strcmp(buffer, ">V000\n")) {                //Get Firmware version
      Serial.print("*V" DEVICE "123\n");
    }
  }
}

void setup() {
  Serial.begin(9600);
  while (!Serial) {  };                                  // wait for serial port to connect.
  servo.attach(9);                                       // set up PWM to servo. Must use 9, per servo library.
  delay(10);
  moveToClose();
  moveToOpen();
}

void loop() {

}
