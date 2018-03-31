#include <IRremote.h>
#include <CapacitiveSensor.h>


#define F_CPU 800000
#define TOUCH_THRESH 500
#define TOUCH_RANGE 100
#define IR_INTERVAL 45
#define NUM_COMMANDS 3

IRsend irsend;
CapacitiveSensor touch_sensor = CapacitiveSensor(A2,A5);

uint8_t TOUCH = A5;
uint8_t LASER = 8;
uint8_t BUTTON = 2;

bool button_pressed = false;
unsigned long next_ir = 0;
uint8_t num_commands_sent = 0;

void laser_on() {
  digitalWrite(LASER, HIGH);
}

void laser_off() {
  digitalWrite(LASER, LOW);
}

void send_signal(uint8_t times) {
  for (uint8_t i = 0; i < times; ++i) {
    irsend.mySendSony(0x01, 1);
  }
}

void send_command(uint8_t times) {
  for (uint8_t i = 0; i < times; ++i) {
    irsend.mySendSony(0xa90, 12);
  }
}

void buttonInterrupt() {
//  detachInterrupt(digitalPinToInterrupt(BUTTON));
  button_pressed = true;
}

void setup() {
  // put your setup code here, to run once:
  pinMode(LASER, OUTPUT);
  pinMode(TOUCH,INPUT);
  pinMode(BUTTON, INPUT);

  digitalWrite(LASER, LOW);
  digitalWrite(TOUCH, LOW);
  digitalWrite(BUTTON, HIGH);

  attachInterrupt(digitalPinToInterrupt(BUTTON), buttonInterrupt, FALLING);

  laser_on();
  delay(1000);
  laser_off();
  delay(1000);
}

void loop() {
  unsigned long touch_reading = touch_sensor.capacitiveSensor(30);
  Serial.println(touch_reading);
  if (button_pressed) {
    // Wait for IR_INTERVAL milliseconds since last IR signal
    while (millis() < next_ir);
    send_command(1);
    num_commands_sent += 1;
    next_ir = millis() + IR_INTERVAL;

    if (num_commands_sent >= NUM_COMMANDS) {
      // For the last button press, we have sent NUM_COMMANDS.
      // Now, we can attach interrupt and serve further button presses.
//      attachInterrupt(digitalPinToInterrupt(BUTTON), buttonInterrupt, FALLING); 
      button_pressed = false;
      num_commands_sent = 0;
    }
  }
  
  if (touch_reading > TOUCH_THRESH + TOUCH_RANGE) {
    // Touch active. Turn on laser
    laser_on();
    if (millis() > next_ir) {
      // It has been more than IR_INTERVAL milliseconds since last IR
      // signal was sent. Send the next IR signal
      send_signal(1);
      next_ir = millis() + IR_INTERVAL;
    }
  } else if (touch_reading < TOUCH_THRESH - TOUCH_RANGE) {
    // Touch inactive. Turn off laser
    laser_off();
  }
}
