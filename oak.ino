/**
 * 
 * adapted BasicHTTPClient.ino example, which was dated 24.05.2015
 * by MC for Moons-Of-MAKLab project to work on Digistump Oak and control either a servo or motor
 * which would move when a fresh use of the hashtag is found on Twitter.
 * Settings for which hashtag to follow are hardcoded on the markcra.com/twit/maklab1.php page 
 * 
 */

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <Servo.h>
#define USE_SERIAL Serial

const int limit_sw_a = 7;
const int limit_sw_b = 8;
const int servo_pin = 9;
// Digishield Motor Driver
const int motor_dir_a = 2;
const int motor_pwm_a = 0;
//const int motor_dir_b = 5; // shared with Serial
//const int motor_pwm_b = 1; // LED

int last_val;

Servo myservo;

void setup() {
  pinMode(1, OUTPUT); //LED on Oak
  pinMode(motor_dir_a, OUTPUT);
  //pinMode(motor_dir_b, OUTPUT);
  pinMode(motor_pwm_a, OUTPUT);
  //pinMode(motor_pwm_b, OUTPUT);
  pinMode(limit_sw_a, INPUT_PULLUP);
  pinMode(limit_sw_b, INPUT_PULLUP);
  myservo.attach(servo_pin);
  myservo.write(0);
  blink(5,500); // 5 quick blinks at boot
  USE_SERIAL.begin(115200);
  // USE_SERIAL.setDebugOutput(true);

  USE_SERIAL.println();
  USE_SERIAL.println();
  USE_SERIAL.println();

  for (uint8_t t = 4; t > 0; t--) {
    USE_SERIAL.printf("[SETUP] WAIT %d...\n", t);
    USE_SERIAL.flush();
    delay(1000);
  }
  Particle.publish("ok", "lets go");
}

void loop() {
  HTTPClient http;

  USE_SERIAL.print("[HTTP] begin...\n");
  http.begin("markcra.com", 80, "/twit/maklab1.php?what=hashtag&short=1"); //HTTP

  USE_SERIAL.print("[HTTP] GET...\n");
  // start connection and send HTTP header
  int httpCode = http.GET();
  if (httpCode) {
    // HTTP header has been send and Server response header has been handled
    USE_SERIAL.printf("[HTTP] GET... code: %d\n", httpCode);
    //blink(5,200);
    // file found at server
    if (httpCode == 200) {
      String payload = http.getString();
      USE_SERIAL.println(payload);
      int new_val = payload.substring(2).toInt();
      USE_SERIAL.print("[HTTP] payload to int:");
      USE_SERIAL.println(new_val);
      if (new_val != last_val) {
        blink(15,200); // success, flash led 15 times fast
        Particle.publish("ok", "new tweet");
        //char* tempstring;
        //sprintf(tempstring, "%d", new_val);
        //Particle.publish("returned", tempstring);
        myservo.write(90);
        // move motor to limit switch b
        digitalWrite(motor_dir_a, HIGH);
        //digitalWrite(motor_dir_b, LOW);
        analogWrite(motor_pwm_a, 200);
        while(digitalRead(limit_sw_b) == LOW) {
          delay(10);
        }
        // stop the motor
        digitalWrite(motor_dir_a, LOW);
        //digitalWrite(motor_dir_b, LOW);
        digitalWrite(motor_pwm_a, LOW);
        // wait here
        delay(500);
        // return the motor to limit switch a
        myservo.write(0);
        digitalWrite(motor_dir_a, LOW);
        //digitalWrite(motor_dir_b, HIGH);
        analogWrite(motor_pwm_a, 150);
        while(digitalRead(limit_sw_a) == LOW) {
          delay(10);
        }
        digitalWrite(motor_dir_a, LOW);
        //digitalWrite(motor_dir_b, LOW);
        digitalWrite(motor_pwm_a, LOW);
        last_val = new_val;
      }
      else {  
        blink(2,200); // no new message, blink twice fast.
      }
    }
  } else {
    USE_SERIAL.print("[HTTP] GET... failed, no connection or no HTTP server\n");
    Particle.publish("ok", "cannot connect to HTTP server");
    blink(5,2000); // error, blink 5 times slow
  }

  delay(10000); // don't spam the webpage too frequently
}

void blink(int i, int t) { // number of blinks, length of on and off time in ms
  for (int a=0; a<i; a++) {
    digitalWrite(1, HIGH);   // turn the LED on (HIGH is the voltage level)
    delay(t);               // wait for a second
    digitalWrite(1, LOW);    // turn the LED off by making the voltage LOW
    delay(t);               // wait for a second
  }
}
