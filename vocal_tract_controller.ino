#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();

// ─── Pin Definitions ────────────────────────────────────────────────────────

#define SOUND_PIN 9

// ─── Servo Channel Assignments ──────────────────────────────────────────────

#define SERVO_1 0
#define SERVO_2 1
#define SERVO_3 2
#define SERVO_4 3
#define SERVO_5 4
#define SERVO_6 5

// ─── Inner Servo Position Constants ─────────────────────────────────────────

#define INNER_UP   460
#define INNER_MID  340
#define INNER_DOWN 200

// ─── Vowel Servo Positions ───────────────────────────────────────────────────
//     EE /iː/  — tight anterior constriction, tongue forward (rest position)
//     OH /ɒ/   — moderate posterior compression, reduced aperture
//     AH /ɑː/  — open, large back cavity, minimal constriction

#define S1_EE 180       // Servo 1: right side open
#define S2_EE 520       // Servo 2: top raised
#define S3_EE 520       // Servo 3: lower raised
#define S4_EE 520       // Servo 4: left side open
#define S5_EE INNER_UP  // Servo 5: anterior — fully up
#define S6_EE INNER_UP  // Servo 6: posterior — fully up

#define S1_OH 360       // Servo 1: right side partially closed
#define S2_OH 340       // Servo 2: top mid
#define S3_OH 340       // Servo 3: lower mid
#define S4_OH 340       // Servo 4: left side partially closed
#define S5_OH INNER_DOWN // Servo 5: anterior — fully down
#define S6_OH INNER_MID  // Servo 6: posterior — mid

#define S1_AH 500       // Servo 1: right side wide open
#define S2_AH 200       // Servo 2: top lowered
#define S3_AH 200       // Servo 3: lower lowered
#define S4_AH 200       // Servo 4: left side wide open
#define S5_AH INNER_DOWN // Servo 5: anterior — fully down
#define S6_AH INNER_DOWN // Servo 6: posterior — fully down

// ─── Glottal Pulse Parameters ────────────────────────────────────────────────
//     F0_HZ   : fundamental frequency (pitch) of the glottal source
//     OPEN_US : open-phase duration in microseconds (controls spectral tilt)

float         F0_HZ    = 125.0;
unsigned long OPEN_US  = 1200;

unsigned long periodUs;   // total period = 1,000,000 / F0_HZ
unsigned long closedUs;   // closed-phase duration = periodUs - OPEN_US

bool          glottisOpen = false;
bool          soundOn     = false;
unsigned long lastChange  = 0;


// ════════════════════════════════════════════════════════════════════════════
//  SETUP — runs once on power-on: demo sequence EE → OH → AH → EE → off
// ════════════════════════════════════════════════════════════════════════════

void setup() {

  pinMode(SOUND_PIN, OUTPUT);
  digitalWrite(SOUND_PIN, LOW);

  pwm.begin();
  pwm.setPWMFreq(50);
  delay(10);

  // Initialise to EE rest position
  setServos(S1_EE, S2_EE, S3_EE, S4_EE, S5_EE, S6_EE);
  delay(1000);

  // ── EE /iː/ ───────────────────────────────────────────────────────────────
  setVoiceEE();
  startGlottalSound();
  delayWithSound(3000);

  // ── OH /ɒ/ ────────────────────────────────────────────────────────────────
  setVoiceOH();
  moveTogether(
    S1_EE, S1_OH,
    S2_EE, S2_OH,
    S3_EE, S3_OH,
    S4_EE, S4_OH,
    S5_EE, S5_OH,
    S6_EE, S6_OH
  );
  delayWithSound(3000);

  // ── AH /ɑː/ ───────────────────────────────────────────────────────────────
  setVoiceAH();
  moveTogether(
    S1_OH, S1_AH,
    S2_OH, S2_AH,
    S3_OH, S3_AH,
    S4_OH, S4_AH,
    S5_OH, S5_AH,
    S6_OH, S6_AH
  );
  delayWithSound(1500);

  stopGlottalSound();

  // ── Return to EE / rest ───────────────────────────────────────────────────
  moveTogether(
    S1_AH, S1_EE,
    S2_AH, S2_EE,
    S3_AH, S3_EE,
    S4_AH, S4_EE,
    S5_AH, S5_EE,
    S6_AH, S6_EE
  );

  setServos(S1_EE, S2_EE, S3_EE, S4_EE, S5_EE, S6_EE);
  delay(500);

  disableServos();
}


// ════════════════════════════════════════════════════════════════════════════
//  LOOP — empty; all behaviour runs once in setup()
// ════════════════════════════════════════════════════════════════════════════

void loop() {
}


// ════════════════════════════════════════════════════════════════════════════
//  VOICE PRESETS
//  Each vowel uses a different F0 and open-phase duration to match the
//  spectral requirements of its formant structure:
//    EE — high F0, short open phase → strong high-frequency energy for F2
//    OH — mid F0, longer open phase → moderate low-frequency emphasis
//    AH — lower F0, short open phase → suits open vowel formant structure
// ════════════════════════════════════════════════════════════════════════════

void setVoiceEE() {
  F0_HZ   = 125.0;
  OPEN_US = 1200;
  recalculatePulseTiming();
}

void setVoiceOH() {
  F0_HZ   = 115.0;
  OPEN_US = 1800;
  recalculatePulseTiming();
}

void setVoiceAH() {
  F0_HZ   = 110.0;
  OPEN_US = 1200;
  recalculatePulseTiming();
}

// Recalculate period and closed-phase duration after any F0 or OPEN_US change.
// Resets pulse state to ensure a clean transition with no partial cycles.
void recalculatePulseTiming() {
  periodUs = (unsigned long)(1000000.0 / F0_HZ);
  closedUs = periodUs - OPEN_US;

  glottisOpen = false;
  digitalWrite(SOUND_PIN, LOW);
  lastChange = micros();
}


// ════════════════════════════════════════════════════════════════════════════
//  SERVO CONTROL
// ════════════════════════════════════════════════════════════════════════════

// Set all six servos to explicit positions simultaneously.
void setServos(int s1, int s2, int s3, int s4, int s5, int s6) {
  pwm.setPWM(SERVO_1, 0, s1);
  pwm.setPWM(SERVO_2, 0, s2);
  pwm.setPWM(SERVO_3, 0, s3);
  pwm.setPWM(SERVO_4, 0, s4);
  pwm.setPWM(SERVO_5, 0, s5);
  pwm.setPWM(SERVO_6, 0, s6);
}

// Smoothly interpolate all six servos from start to end positions
// over 100 steps at 15 ms each (1.5 s total transition).
// updateGlottalPulse() is called on every step so audio continues
// uninterrupted during tract movement.
void moveTogether(
  int s1Start, int s1End,
  int s2Start, int s2End,
  int s3Start, int s3End,
  int s4Start, int s4End,
  int s5Start, int s5End,
  int s6Start, int s6End
) {
  const int steps     = 100;
  const int stepDelay = 15;   // ms per step → 1.5 s total

  for (int i = 0; i <= steps; i++) {
    int pos1 = s1Start + ((long)(s1End - s1Start) * i) / steps;
    int pos2 = s2Start + ((long)(s2End - s2Start) * i) / steps;
    int pos3 = s3Start + ((long)(s3End - s3Start) * i) / steps;
    int pos4 = s4Start + ((long)(s4End - s4Start) * i) / steps;
    int pos5 = s5Start + ((long)(s5End - s5Start) * i) / steps;
    int pos6 = s6Start + ((long)(s6End - s6Start) * i) / steps;

    setServos(pos1, pos2, pos3, pos4, pos5, pos6);
    updateGlottalPulse();

    delay(stepDelay);
  }
}

// Set all servo channels to zero (no PWM signal) to cut power and
// eliminate holding current — prevents overheating during idle periods.
void disableServos() {
  for (int ch = 0; ch < 6; ch++) {
    pwm.setPWM(ch, 0, 0);
  }
}


// ════════════════════════════════════════════════════════════════════════════
//  GLOTTAL SOUND CONTROL
// ════════════════════════════════════════════════════════════════════════════

// Begin glottal pulse generation.
void startGlottalSound() {
  soundOn     = true;
  glottisOpen = false;
  digitalWrite(SOUND_PIN, LOW);
  lastChange  = micros();
}

// Stop glottal pulse generation and ensure pin is left LOW.
void stopGlottalSound() {
  soundOn     = false;
  glottisOpen = false;
  digitalWrite(SOUND_PIN, LOW);
}

// Block for durationMs milliseconds while keeping the glottal pulse running.
// Uses a millis() polling loop rather than delay() so updateGlottalPulse()
// can be called continuously throughout.
void delayWithSound(unsigned long durationMs) {
  unsigned long start = millis();
  while (millis() - start < durationMs) {
    updateGlottalPulse();
  }
}

// Non-blocking glottal pulse update — call as frequently as possible.
//
// How it works:
//   Each glottal cycle has two phases:
//     Open phase  (pin HIGH, duration = OPEN_US)   → pressure pulse / speaker forward
//     Closed phase (pin LOW,  duration = closedUs) → no airflow   / speaker back
//
//   Rather than using delay() — which would freeze the processor — this
//   function checks elapsed time on every call and only acts when a phase
//   threshold has been crossed. The processor is free to do other work
//   (servo updates, serial checks) between calls.
//
//   lastChange stores the micros() timestamp of the last state transition.
//   now - lastChange gives the time elapsed in the current phase.
void updateGlottalPulse() {
  if (!soundOn) return;

  unsigned long now = micros();

  if (glottisOpen) {
    // Currently in open phase — check if it's time to close
    if (now - lastChange >= OPEN_US) {
      digitalWrite(SOUND_PIN, LOW);
      glottisOpen = false;
      lastChange  = now;
    }
  } else {
    // Currently in closed phase — check if it's time to open
    if (now - lastChange >= closedUs) {
      digitalWrite(SOUND_PIN, HIGH);
      glottisOpen = true;
      lastChange  = now;
    }
  }
}
