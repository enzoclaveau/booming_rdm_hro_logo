#include "FastLED.h"

// PINS
#define RDM_PIN          3
#define RDMD_PIN         4
#define CENT_PIN         5
#define OEXP_PIN         6
#define LINE_PIN         7 
#define HRLOGO_PIN       8
#define HS_PIN           9
#define ROT_PIN         10

// NUMER OF LEDS
#define NUM_LEDS_RDM    60
#define NUM_LEDS_RDMD   60
#define NUM_LEDS_CENT   60
#define NUM_LEDS_OEXP   60
#define NUM_LEDS_LINE   60
#define NUM_LEDS_HRLOGO 60
#define NUM_LEDS_HS     60
#define NUM_LEDS_ROT    60

// HARDWARE SETTINGS
#define LED_TYPE    WS2812
#define COLOR_ORDER    GRB

// SOFTWARE SETTINGS
#define RANDOM_DIRECTION
//#define SPARKLES
#define DIMMED_HRO
#define ORANGE_COLOR_HUE       30
#define COLOR_STEPS_RDM         1
#define FADE_STEPS_HRLOGO      10           // The steps with which the leds from the hrlogo fade in and fade out
#define REFRESH_SPEED_HRLOGO   20           // 
#define BRIGHTNESS            255
#define FRAMES_PER_SECOND      30

// DEBUG SETTINGS
//#define DEBUG_GHOST_ARRAY
//#define DEBUG_RDM
#define DEBUG_HRLOGO

#ifdef DEBUG_GHOST_ARRAY
  #define DEBUG_PC
#elif defined(DEBUG_RDM)
  #define DEBUG_PC
#elif defined(DEBUG_HRLOGO)
  #define DEBUG_PC
#endif


CRGB rdm[NUM_LEDS_RDM];
CRGB rdmd[NUM_LEDS_RDMD];
CRGB cent[NUM_LEDS_CENT];
CRGB oexp[NUM_LEDS_OEXP];
CRGB line[NUM_LEDS_LINE];
CRGB hrlogo[NUM_LEDS_HRLOGO];
CRGB hs[NUM_LEDS_HS];
CRGB rot[NUM_LEDS_ROT];


// Initialisations for the RDM strip
uint8_t colorArray[2+ORANGE_COLOR_HUE/COLOR_STEPS_RDM*2];
uint8_t ghostArray[NUM_LEDS_RDM+2*sizeof(colorArray)];
uint8_t sizeCA = sizeof(colorArray);
uint8_t sizeGA = sizeof(ghostArray);


uint8_t headPos = sizeCA;
uint8_t tailPos, fade_out_hrlogo, fade_in_hrlogo2, reset_hrlogo2 = 0;
uint8_t fade_in_hrlogo = 1;
uint8_t reset_hrlogo = 1;
uint8_t fade_out_hrlogo2 = 1;

unsigned long count = 1;

void setup() {
  
  #ifdef DEBUG_PC
    delay(2000);
    Serial.begin(250000);
    Serial.print("Size of color array: ");
    Serial.println(sizeCA);
    Serial.print("Size of Ghost array: ");
    Serial.println(sizeGA);
  #endif


  
  initLeds();
  makeColors();

}

void loop() {

  make_rdm();

  sparkle_fade();
  
  FastLED.show();
  FastLED.delay(1000/FRAMES_PER_SECOND);

  count++;
}

void initLeds(){
  FastLED.addLeds<LED_TYPE,RDM_PIN,COLOR_ORDER>(rdm, NUM_LEDS_RDM).setCorrection(TypicalLEDStrip);
  FastLED.addLeds<LED_TYPE,RDMD_PIN,COLOR_ORDER>(rdmd, NUM_LEDS_RDMD).setCorrection(TypicalLEDStrip);
  FastLED.addLeds<LED_TYPE,CENT_PIN,COLOR_ORDER>(cent, NUM_LEDS_CENT).setCorrection(TypicalLEDStrip);
  FastLED.addLeds<LED_TYPE,OEXP_PIN,COLOR_ORDER>(oexp, NUM_LEDS_OEXP).setCorrection(TypicalLEDStrip);
  FastLED.addLeds<LED_TYPE,LINE_PIN,COLOR_ORDER>(line, NUM_LEDS_HRLOGO).setCorrection(TypicalLEDStrip);
  FastLED.addLeds<LED_TYPE,HRLOGO_PIN,COLOR_ORDER>(hrlogo, NUM_LEDS_HRLOGO).setCorrection(TypicalLEDStrip);
  FastLED.addLeds<LED_TYPE,HS_PIN,COLOR_ORDER>(hs, NUM_LEDS_HS).setCorrection(TypicalLEDStrip);
  FastLED.addLeds<LED_TYPE,ROT_PIN,COLOR_ORDER>(rot, NUM_LEDS_ROT).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(BRIGHTNESS);
}

void make_rdm(){
  
  fillGhostArray();
  computePositions();
  copy_ghost2rdm(); 
   
}

void make_hrlogo(){
  
}

void makeColors(){
  
  int j = 0;
  for(int i = 0; i < sizeCA; i++){  
    if(i<sizeCA/2){
      colorArray[i] = i * COLOR_STEPS_RDM;
    }
    else{     
       colorArray[i] = colorArray[i - (2 * j + 1)];
       j++;
    }
  }

  // Print the color array
  #ifdef DEBUG_PC
    Serial.println("The color array hue values: ");
    for(int i = 0; i < sizeCA; i++){
      Serial.print(colorArray[i]);
      Serial.print(" ");
    }
    Serial.println(" ");
  #endif
    
}

void fillGhostArray(){
  
  // First fill the whole strip with one value
  for(int i = 0; i < sizeGA; i++){
    ghostArray[i] = 0;
  }
  
  // Then copy the color array into a certain part of it

  // ALS HEADPOS KLEINER IS DAN NUM LEDS ONDERSTAANDE DOEN EN ANDERS IETS ANDERS DOEN, DAN SPRINGT IE ALS HET WARE TERUG.
  for(int i = 0; i < sizeCA; i++){
    ghostArray[i + tailPos] = colorArray[i];
  }
  
  // Print the whole "ghost array"
  #ifdef DEBUG_GHOST_ARRAY
    for(int i = 0; i < sizeGA; i++){
      Serial.print(ghostArray[i]);
      Serial.print(" ");
    }
    Serial.println(" ");
  #endif
}

void computePositions(){
  
  uint8_t randDir = random8(3);
    
  if(tailPos < NUM_LEDS_RDM + sizeCA){
    #ifdef RANDOM_DIRECTION      
      if(randDir > 1 && tailPos > 0){
        tailPos--;
      }
      else{
        tailPos++;
      }
    #else  
      tailPos++;
    #endif
  }
  else{
    tailPos = 0;
  }

  if(headPos < NUM_LEDS_RDM + 2 * sizeCA){
    #ifdef RANDOM_DIRECTION
      if(randDir > 1 && headPos > sizeCA){
        headPos--;
      }
      else{
        headPos++;
      }
    #else 
      headPos++;
    #endif
  }
  else{
    headPos = sizeCA;
  }  
  
}


void copy_ghost2rdm(){

  for(int i = 0; i < NUM_LEDS_RDM; i++){
    rdm[i] = CHSV(ghostArray[sizeCA + i], 255, 255);
  }

  #ifdef SPARKLES
    rdm[random(NUM_LEDS_RDM)] = CHSV(random(ORANGE_COLOR_HUE), 255, 255);
  #endif 

  #ifdef DEBUG_RDM
    Serial.print("Green led array: ");
    for(int i = 0; i < NUM_LEDS_RDM; i++){
      Serial.print(rdm[i].g);
      Serial.print(" ");
    }
    Serial.println(" ");
  #endif    
}


void sparkle_fade(){

// Initialise the so called position arrays just once (hence they are static)
static int pos[NUM_LEDS_HRLOGO]  = {0};
static int pos2[NUM_LEDS_HRLOGO] = {0};

    // Select randomly some leds to be turned on, but do so only once in a while, based on the value of "fade_in_hrlogo".
    if(reset_hrlogo){
      
      for(int i = 0; i < NUM_LEDS_HRLOGO; i++){
        pos[i] = 0 ;
      }
      
      for(int i = 0; i < NUM_LEDS_HRLOGO / 4; i++){ 
        pos[random16(NUM_LEDS_HRLOGO)] = 1;
      }
      
      reset_hrlogo = 0;  
    }

  
    if(reset_hrlogo2){
        
      for(int i = 0; i < NUM_LEDS_HRLOGO; i++){
        pos2[i] = 0 ;
      }
      
      for(int i = 0; i < NUM_LEDS_HRLOGO / 4; i++){ 
        pos2[random16(NUM_LEDS_HRLOGO)] = 1;
      }

      for(int i = 0; i < NUM_LEDS_HRLOGO; i++){ 
        if(pos[i] == pos2[i]){
          pos2[i] = 0;
        }
      }      
      
      reset_hrlogo2 = 0;  
    }
  
    if(fade_in_hrlogo){
      
      // Increment the value of the leds that have been randomly chosen
      for(int i = 0; i < NUM_LEDS_HRLOGO; i++){
        if(pos[i]){
          hrlogo[i] += CRGB(FADE_STEPS_HRLOGO, 0, 0);
        }
      }
  
    }
  
    if(fade_in_hrlogo2){
      
      // Increment the value of the leds that have been randomly chosen
      for(int i = 0; i < NUM_LEDS_HRLOGO; i++){
        if(pos2[i]){
          hrlogo[i] += CRGB(FADE_STEPS_HRLOGO, 0, 0);
        }
      }
  
    }
  
    if(fade_out_hrlogo){

      for(int i = 0; i < NUM_LEDS_HRLOGO; i++){
        if(pos[i]){
          hrlogo[i] -= CRGB(FADE_STEPS_HRLOGO, 0, 0);
        }
      }
    }     
    

    if(fade_out_hrlogo2){

      //void fadeLightBy(hrlogo, NUM_LEDS_HRLOGO, FADE_STEPS_HRLOGO);
      //fadeToBlackBy( hrlogo, NUM_LEDS_HRLOGO, FADE_STEPS_HRLOGO);
      for(int i = 0; i < NUM_LEDS_HRLOGO; i++){
        if(pos2[i]){
          hrlogo[i] -= CRGB(FADE_STEPS_HRLOGO, 0, 0);
        }
      }     
    }
    
    #ifdef DIMMED_HRO
      if (hrlogo[i].r = 0) {
        hrlogo[i].r = 1;
      }
    #endif


    // Set the flags depending on the loop counter
    if(count % (REFRESH_SPEED_HRLOGO) == 0){
      fade_out_hrlogo = 0;
      reset_hrlogo    = 1;
      fade_in_hrlogo  = 1;
      fade_out_hrlogo2 = 1;
      fade_in_hrlogo2  = 0;
    }
    
    if(count % (REFRESH_SPEED_HRLOGO*2) == 0){
      fade_out_hrlogo2 = 0;
      reset_hrlogo2    = 1;
      fade_in_hrlogo2  = 1;
      fade_out_hrlogo = 1;
      fade_in_hrlogo  = 0;
    }


  #ifdef DEBUG_HRLOGO
    for(int i = 0; i < NUM_LEDS_HRLOGO; i++){
      Serial.print(hrlogo[i].r);
      Serial.print(" ");
    }
    Serial.println(" ");
  #endif
  
}
