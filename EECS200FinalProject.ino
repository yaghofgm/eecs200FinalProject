#include <NewPing.h>
#define MAX_DISTANCE 200 // cm, if bigger than max distance, outputs zero

// Thresholds in cm
#define FCT 35 // Front close thresh //35
#define FFT 35 // Front far thresh
#define RCT 20 // Right close thresh
#define RFT 50 // Right far thresh

const int pwm_forward = 50; //50, 70, 35
// const int pwm_forward_slow = 50;
const int pwm_turn = 42; //55, 30, 32
const int thermalThreshold = 270;

// 8-bit PWM for Arduino Nano, thus 100% duty cycle is analogWrite(<pin>, 255)
const int PWM_right = 3; // D3 PWM output 3
const int PWM_left = 5;  // D5 PWM output 5
const int SF_toMUX = 2;  // D2 GPIO output
const int SR_toMUX = 4;  // D4 GPIO output

const int F_trig = 7; // GPIO output
const int F_echo = 8; // GPIO input

const int R_trig = 9;  // GPIO output
const int R_echo = 10; // GPIO input

const int Buzzer_pin = 6; //D6 GPIO output
const int thermalOut = A0; //A) GPIO input

// Notes of the melody
int melody[] = {262, 294, 330, 349, 392, 440, 494, 523};  // C, D, E, F, G, A, B, C
int duration = 500;  // Duration of each note in ms


const bool test_all =1;
const bool test_sonar = 0;
const bool test_motors = 0;
const bool test_buzzer = 0;
const bool test_thermal =0;
const bool zero_alter = 1;

bool just_turned_left = 0;

bool run = 1;

NewPing sF(F_trig, F_echo, MAX_DISTANCE);
NewPing sR(R_trig, R_echo, MAX_DISTANCE);

void setup() {
  Serial.begin(9600); // Initialize serial communication for debugging
  Serial.println("Setup started");

  pinMode(PWM_right, OUTPUT);
  pinMode(PWM_left, OUTPUT);
  pinMode(SF_toMUX, OUTPUT);
  pinMode(SR_toMUX, OUTPUT);

  Serial.println("Setup complete");
}

void loop() {

  while(run){
    double valF = sF.ping_cm(sF.ping_median(3, MAX_DISTANCE)); // Get median of 3 pings
    double valR = sR.ping_cm(sR.ping_median(3, MAX_DISTANCE)); // Get median of 3 pings

    if(zero_alter){
      valF = !valF ? 159 : valF;
      valR = !valR ? 159 : valR;
    }

    if (test_all == 1) {
      // Serial.println("Entering main logic test block");
      
      // Bottle is detected
      if(close_bottle()){
        run = 0;
        stop();
        sound_the_alarm();
      }
      
      // Wall in front
      if (valF < FCT) {
        // Wall detected front AND right
        if(valR <= 60){ //60
          turn_left();
        } 
        // Wall detected front but NOT right
        else if(valR >= 50) { //50
          turn_right();
        }
      } 
      // Wall NOT in front
      else {
        if(valR < 10){ //20, 8, 7
          drive_forward();
        } 
        // Drifitng away from the wall
        else if(valR > 10) { //20, 5, 6, 10 Ten worked for angled and curved wall, but NOT straight 90
          Serial.println("Action: Realign to wall - Right too far");
          turn_right();
        }
      }
    }

    
    if(test_sonar == 1){
      Serial.print("Front Right: ");
      Serial.print(valF);
      Serial.print(" ");
      Serial.print(valR);
      Serial.println(" cm");
      delay(2000);
    }

    if (test_buzzer){
      sound_the_alarm();
    }

    if (test_thermal){
      Serial.println(analogRead(thermalOut));
    }
  } 
  stop();
}

// Add logic for turns and speed adjustments as needed
void stop(){
  analogWrite(PWM_left,0);
  analogWrite(PWM_right,0);
}
void turn_right() {
  //Serial.println("Executing turn_right()");
  digitalWrite(SF_toMUX, LOW);
  digitalWrite(SR_toMUX, LOW);
  analogWrite(PWM_right,pwm_turn);
  analogWrite(PWM_left,pwm_turn);
}

void drive_forward() {
  //Serial.println("Executing drive_forward()");
  digitalWrite(SF_toMUX, LOW);
  digitalWrite(SR_toMUX, HIGH);
  int correction = 1.5;

  analogWrite(PWM_right,pwm_forward);
  analogWrite(PWM_left,pwm_forward*correction);
}

void turn_left() {
  //Serial.println("Executing turn_left()");
  digitalWrite(SF_toMUX, HIGH);
  digitalWrite(SR_toMUX, HIGH);
  analogWrite(PWM_right,pwm_turn);
  analogWrite(PWM_left,pwm_turn);
}

// Placeholder function to simulate a thermal data check
bool close_bottle() {
  // Add logic for checking thermal data here
  //Serial.println("Checking for close bottle");
  Serial.println("Bottle detected.");
  if(analogRead(thermalOut) >= thermalThreshold){
    return true;  
  }
  return false; // Placeholder return value
}

void sound_the_alarm() {
  Serial.println("Sounding the alarm");
  for (int i=0; i<8;i++){
    analogWrite(Buzzer_pin, 80);  // 50% duty cycle for sound
    delay(duration);
    analogWrite(Buzzer_pin, 0);  // Turn off the buzzer
    delay(100);  // Pause between notes
  }
}


