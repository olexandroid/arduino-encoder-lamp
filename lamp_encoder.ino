#define CLK 4
#define DT 3
#define SW 2
#define ENC_TYPE 1
#define MIN_STEP 1
#define STEP 0.1

#define DELAY 1

#define LIGHT 9
#include "GyverEncoder.h"
Encoder enc(CLK, DT, SW, ENC_TYPE);

#include "FastLED.h"
#define LED 13
#define NUM_LEDS 10


volatile float light = 128;
volatile byte color = 100, wave_cnt = 1;
volatile bool mode = 0;
CRGBArray<NUM_LEDS> leds;

void setup() {
  // D9 D10 - 62.5 кГц
  TCCR1A = 0b00000001;  // 8bit
  TCCR1B = 0b00001001;  // x1 fast pwm

  FastLED.addLeds<NEOPIXEL, LED>(leds, NUM_LEDS);
  FastLED.setBrightness(255);
  pinMode(LIGHT, OUTPUT);
  Serial.begin(9600);
}

void loop() {
  enc.tick();
  
  if (enc.isHolded()){
    changeMode();
  }

  if (enc.isLeft()) {
    if (mode){
      nextColor();
    } else {      
      increaseLight();
    } 
  }

  if (enc.isRight()) {
    if (mode){
      prevColor();
    } else {      
      decreaseLight();
    }
  }

  if (enc.isClick()){
    if (mode){
      changeWaves();
    } else {      
      if (light < 10) maxLight();
      else minLight();
    }
    
  }

  if (mode){
    for (byte i = 0; i < NUM_LEDS / 2.0; i++){
      leds[i] = CHSV(
        color, 
        255, 
        round(
          mapf(
            sin(
              (millis() % 6282) / 500.0 +
              mapf(
                i * 1.0, 0.0, NUM_LEDS / 2.0, 0.0, wave_cnt * 2 * 3.14
              )
            ),
            -1.0, 1.0,
            0.0, 255.0
          )
        )
      );
    }
    leds(NUM_LEDS - 1, NUM_LEDS / 2) = leds(0, NUM_LEDS / 2 - 1);
  }
  analogWrite(LIGHT, light);
  FastLED.delay(DELAY);
}

void logLight(){
  Serial.print("light: ");
  Serial.println(light);
}

void logColor(){
  Serial.print("color: ");
  Serial.println(color);
}

void increaseLight(){
    light = constrain(light + max(MIN_STEP, light * STEP), 0, 255);
    analogWrite(LIGHT, light);
    logLight();
}

void decreaseLight(){
    light = constrain(light - max(MIN_STEP, light * STEP), 0, 255);
    analogWrite(LIGHT, light);
    logLight();
}

void maxLight(){
  for (int i = 0; i <= 255; i++) {
    analogWrite(LIGHT, light = i);
    i += i * 0.1;
    delay(2);
    logLight();
  }
}

void minLight(){
 for (int i = light; i >= 0; i--) {
    analogWrite(LIGHT, light = i);
    i -= i * 0.1;
    delay(2);
    logLight();
  }
}

void changeMode(){
  mode = !mode;
  Serial.print("mode: ");
  Serial.println(mode);
  if (mode){
    minLight();
  } else {     
    leds.fadeToBlackBy(255); 
    maxLight();
  }    
}

void nextColor(){
  color+=5;
  logColor();
}

void prevColor(){
  color-=5;
  logColor();
}

void changeWaves(){
  wave_cnt++;
  if (wave_cnt >= NUM_LEDS / 2) {
    wave_cnt = 0;    
  }
}

float mapf(float x, float in_min, float in_max, float out_min, float out_max){
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}