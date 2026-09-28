# Dynamic Soft-Robotic Vocal Tract

An embedded control system for a **soft-robotic vocal tract capable of
producing recognisable human vowel sounds through physical
articulation**.

Developed as my final-year Computer Systems Engineering project at Royal
Holloway, University of London, the system explores physical speech
synthesis by dynamically changing the internal geometry of a silicone
vocal tract.

The project combines **embedded programming, electronics, acoustics,
signal analysis and soft robotics** to reproduce aspects of the human
speech-production mechanism.

## Overview

Human speech is produced through the interaction between a sound source
at the larynx and the changing geometry of the vocal tract.

This project recreates that process physically.

A loudspeaker provides an artificial glottal source, while six
servo-controlled paddles deform a flexible silicone vocal tract. By
changing the internal geometry of the tract, different acoustic
resonances can be produced, allowing recognisable vowel sounds to
emerge.

The system was developed to produce and transition between three vowel
configurations:

-   `/iː/` --- **EE**
-   `/ɒ/` --- **OH**
-   `/ɑː/` --- **AH**

Rather than playing recordings of these vowels, the system generates
them through the physical acoustic behaviour of the tract.

## System Architecture

``` text
                    Arduino Uno
                         |
              +----------+----------+
              |                     |
              v                     v
         PCA9685 Driver          D9 Pulse
              |                     |
        6 Linear Servos         PAM8403
              |                  Amplifier
              v                     |
       Articulating Paddles          v
              |                 Loudspeaker
              v                     |
       Silicone Vocal Tract <--------+
              |
              v
         Acoustic Output
```

The Arduino controls two main parts of the system simultaneously:

1.  **Articulation** --- changing the shape of the vocal tract.
2.  **Excitation** --- generating the periodic source signal that
    excites the tract.

## Hardware

The system uses:

-   Arduino Uno
-   PCA9685 16-channel PWM servo driver
-   Six linear servo actuators
-   Servo-driven articulating paddles
-   Flexible silicone vocal tract
-   Plaster support jacket
-   PAM8403 5 V audio amplifier
-   Small cone loudspeaker
-   External servo power supply

The vocal tract itself was produced from anatomical geometry derived
from MRI data.

The loudspeaker acts as an artificial laryngeal source and is positioned
so that its output excites the vocal tract.

## Servo-Based Articulation

Six servo-controlled paddles alter different regions of the silicone
tract.

Each vowel is represented by a predefined six-servo configuration:

``` cpp
#define S1_EE 180
#define S2_EE 520
#define S3_EE 520
#define S4_EE 520
#define S5_EE INNER_UP
#define S6_EE INNER_UP
```

Equivalent configurations are defined for **OH** and **AH**.

These positions alter the location and size of constrictions and
cavities within the tract. The resulting geometry changes the tract's
resonant frequencies and therefore its vowel-like acoustic output.

## Smooth Articulatory Transitions

The tract can transition continuously between vowel configurations
rather than simply jumping between fixed positions.

`moveTogether()` interpolates the position of all six servos:

``` cpp
int pos1 = s1Start + ((long)(s1End - s1Start) * i) / steps;
```

The same calculation is performed for each actuator, producing
coordinated movement across the tract.

## Artificial Glottal Source

Arduino digital pin 9 generates the excitation signal used by the
loudspeaker.

Two parameters control the source:

``` cpp
float F0_HZ;
unsigned long OPEN_US;
```

`F0_HZ` determines the fundamental frequency of the source, while
`OPEN_US` controls the duration of the open phase.

The pulse timing is calculated using:

``` cpp
periodUs = 1000000.0 / F0_HZ;
closedUs = periodUs - OPEN_US;
```

The resulting signal is amplified by the PAM8403 and passed to the
loudspeaker. The vocal tract then acts as an acoustic filter, shaping
this excitation according to its current geometry.

## Pulse Generation

The glottal-source controller uses `micros()` to track the open and
closed phases of the waveform:

``` cpp
unsigned long now = micros();

if (now - lastChange >= OPEN_US) {
    ...
}
```

This approach allows pulse timing to be updated alongside other
controller operations. The demonstration code also uses a polling-based
`delayWithSound()` routine so the source continues to be updated while a
vowel is held.

## Vowel Sequence

The demonstration performs:

``` text
EE /iː/
   |
   v
OH /ɒ/
   |
   v
AH /ɑː/
   |
   v
EE /iː/
```

The system begins at its EE/rest configuration, starts the artificial
glottal source, and moves through the different articulatory
configurations.

At the end of the demonstration, the servos return to their rest
positions and are disabled.

## Servo Power Management

After the demonstration, PWM output to the servo channels is disabled:

``` cpp
void disableServos() {
    for (int ch = 0; ch < 6; ch++) {
        pwm.setPWM(ch, 0, 0);
    }
}
```

This reduces unnecessary holding current and heat while the system is
idle.

## Acoustic Analysis

The output of the physical tract was analysed using **Praat**.

Formant measurements were used to evaluate whether changes in the
physical configuration produced the expected acoustic differences
between vowels. Testing focused particularly on:

-   first formant (`F1`)
-   second formant (`F2`)
-   fundamental frequency (`F0`)

This provided a quantitative method of evaluating the output alongside
perceptual recognition of the generated vowels.

## Engineering Challenges

### Soft actuation

Early experiments investigated different approaches to dynamically
deforming the tract. The final design used servo-driven paddles because
they provided greater control over displacement, repeatability and
movement speed.

### Mechanical resistance

The silicone tract naturally resisted deformation, requiring careful
calibration of servo positions to achieve useful internal shape changes
without placing excessive load on the actuators.

### Electronics

The servo system required an external power source and PCA9685 PWM
controller rather than driving the actuators directly from the Arduino.

### Audio generation

The loudspeaker and amplifier system required experimentation to produce
a sufficiently strong excitation signal while avoiding distortion and
overheating.

### Coordinated control

Mechanical articulation and sound generation needed to operate together
so that the tract remained acoustically excited while changing shape.

## Project Structure

``` text
Dynamic-Vocal-Tract/
|
+-- src/
|   +-- dynamic_vocal_tract.ino
|
+-- tests/
|   +-- i2c_scanner.ino
|   +-- servo_test.ino
|   +-- glottal_source_test.ino
|
+-- docs/
|   +-- hardware.md
|   +-- vowel_calibration.md
|
+-- README.md
```

The main Arduino sketch contains the final integrated control system.
Development sketches were used independently to test communication,
actuator movement and source generation during prototyping.

## Technologies

-   **Embedded:** Arduino / C++
-   **Servo control:** PCA9685 / I²C
-   **Signal generation:** Arduino timing and GPIO
-   **Signal analysis:** Praat
-   **Mechanical system:** Soft silicone vocal tract with servo-driven
    articulation
-   **Audio:** PAM8403 amplifier and loudspeaker

## Wider Application

The project was developed under the supervision of **Professor David M.
Howard** and integrated with concepts from his **Vocal Tract Organ**
work.

The system demonstrates how physical vocal-tract modelling and embedded
control can be combined to explore speech production and alternative
approaches to voice synthesis.

The work was subsequently used as part of a wider project involving a
**choir of people who had undergone laryngectomies** and was presented
at the **Pan-European Voice Conference 2026**.

## Author

**Naomi Parfitt**\
BSc Computer Systems Engineering\
Royal Holloway, University of London
