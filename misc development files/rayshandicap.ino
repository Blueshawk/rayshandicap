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


  todo -
  set up close/open rom storage.
  add motor reverse and store.
*/

#include <Servo.h>

// 10 = Flat-Man_XL
// 15 = Flat-Man_L
// 19 = Flat-Man
// 98 = Flip-Mask/Remote Dust Cover
// 99 = Flip-Flat. //indi driver apparently turns off the cover if not 99

#define DEVICE "99"

Servo servo;  // create servo object to control a servo
int pos;
int open = 110;                         //angle when open
int close = 10;                         //angle when closed
long dly = 30;                          //delay for motor
char brightness[4] = "128";
short cover = 1;                  //status of cover 0=niether
short light = 0;                  //status of light
long motor = 0;                   //status of motor in motion(indi uses this)

//Begin Called Functions (putting them here makes arduino IDE sort out header files for you)
void moveToOpen() {
  for (pos = close; pos <= open ; pos += 1) { // goes from "close" degrees to "open" degrees in steps of 1 degree
    servo.write(pos);                // tell servo to go to position in variable 'pos'
    delay(dly);                       // this slows down the servo so the cap isn't swatting meteors
    cover = 0;                       //cover in motion
    digitalWrite(HIGH, LED_BUILTIN);
  }
  Serial.print("*O" DEVICE "000\n");
  cover = 2;
  motor = 0;
  digitalWrite(LOW, LED_BUILTIN);
}
void moveToClose() {
  for (pos = open; pos >= close ; pos -= 1) { // goes from 180 degrees to 0 degrees
    // in steps of 1 degree
    servo.write(pos);                   // tell servo to go to position in variable 'pos'
    delay(dly);  // this slows down the servo so the cap isn't swatting flies
    cover = 0;                          //cover in motion
    digitalWrite(HIGH, LED_BUILTIN);
  }
  Serial.print("*C" DEVICE "000\n");
  cover = 1;
  motor = 0;
  digitalWrite(LOW, LED_BUILTIN);
}

//SerialEvent() is automatically called when serial port recieves data
void serialEvent() {
  if (Serial.available()) {
    char buffer[16];
    memset(buffer, 0, sizeof(buffer));
    Serial.readBytesUntil("\n", buffer, sizeof(buffer)); //get command from serial

    //Ping - Connect and return device type - Indi_flipflat only uses 99 or non cover
    if (!strcmp(buffer, ">P000\n")) {
      Serial.print("*P" DEVICE "000\n");
    }
    //Open cover
    else if (!strcmp(buffer, ">O000\n")) {
      motor = 2000;
      cover = 0;                        //sets status=cover in motion.
      moveToOpen();
    }
    //Close Cover
    else if (!strcmp(buffer, ">C000\n")) {
      motor = 2000;
      cover = 0;                         
      moveToClose();
    }
    //Light on(simulated)
    else if (!strcmp(buffer, ">L000\n")) {
      light = 1;
      Serial.print("*L" DEVICE "000\n");
    }
    //light off(simulated)
    else if (!strcmp(buffer, ">D000\n")) {
      light = 0;
      Serial.print("*D" DEVICE "000\n");
    }
    //Set Brightness(simulated)
    else if (!strncmp(buffer, ">B", 2)) {
      strncpy(brightness, buffer + 2, 3);
      Serial.print("*B" DEVICE);
      Serial.print(brightness);
      //Serial.print("\n");
    }
    //Get Brightness(simulated)
    else if (!strcmp(buffer, ">J000\n")) {
      Serial.print("*J" DEVICE);
      Serial.print(brightness);
      Serial.print("\n");
    }
    //Get DEvice status
    else if (!strcmp(buffer, ">S000\n")) {
      Serial.print("*S" DEVICE);
      Serial.print(motor > 0 ? 1 : 0);
      Serial.print(light);
      Serial.print(motor > 0 ? 0 : cover);
      Serial.print("\n");
    }
    //Get Firmware version
    else if (!strcmp(buffer, ">V000\n")) {
      Serial.print("*V" DEVICE "123\n");
    }
  }
}

void setup() {
  Serial.begin(9600);
  while (!Serial) {  }  ;             // wait for serial port to connect.
  servo.attach(9);                 // set up PWM to servo. Must use 9, per servo library.
  moveToOpen();
}

void loop() {

}
