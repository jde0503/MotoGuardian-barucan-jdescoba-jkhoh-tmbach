/******************************************************************************
* Author: Team MotoGuardian
* Class:  ECE 140B
*
* File Name: MotoGuardianArduino.ino
* Description: Main file for MotoGuardian theft and crash detection and alert 
*              software.
******************************************************************************/
/*** Include necessary libraries ***/
#include <CurieTimerOne.h>
#include <BMI160.h>
#include <CurieIMU.h>
#include <math.h>


/*** Declare and initialize global variables ***/
// Knobs to tweak.
const int samplePeriod = 10000;  //100000 microsec -> 10 Hz freq
const float HPF = 0.98;  // High-Pass filter coeff. LPF = 1 - HPF.

// Settings from web app.
bool audibleTheftAlert = true;
byte armedSensitivity = 1;

// Variables to work with.
float A_x, A_y, A_z;
float G_x, G_y, G_z;
float currentAngle;
volatile bool newIMUSample = false;
byte state = 0; // 0 = Disarmed and OK, 1 = Disarmed and Crash Detected,
                // 2 = Armed and OK, 3 = Armed and Theft Detected.


/*** ISR to sample IMU ***/
void sampleIMU() {
    CurieIMU.readAccelerometerScaled(A_x, A_y, A_z);
    CurieIMU.readGyroScaled(G_x, G_y, G_z);
    newIMUSample = true;
}


/*** Calculates roll angle based on complementary filter results of IMU measurements ***/
float getRollAngle() {
    // Declare and initialize variables.
    float accelAngle = atan2(A_y, sqrt((A_x*A_x) + (A_z*A_z))) * 180 / PI;
    
    return HPF*(currentAngle + (G_x*samplePeriod/1000000)) + (1-HPF)*accelAngle;
}


/*** Setup Function. Run once @ power up. ***/
void setup() {
    // Initialize serial.
    Serial.begin(115200);

    // Initialize IMU, set ranges, and get initial angle.
    CurieIMU.begin();
    CurieIMU.setAccelerometerRange(15);
    CurieIMU.setGyroRange(250);
    CurieIMU.readAccelerometerScaled(A_x, A_y, A_z);
    currentAngle = atan2(A_y, sqrt((A_x*A_x) + (A_z*A_z))) * 180 / PI;

    // Initialize CurieTimer.
    CurieTimerOne.start(samplePeriod, &sampleIMU);
    
    // Function to GET settings from web app.

    // Function to calibrate lean angle, position, speed.

    while (!Serial) {
        // Wait for serial port to open.
        
    }
}


/*** Main Control Loop ***/
void loop() {
    while (Serial) {
        if (newIMUSample) {
            currentAngle = getRollAngle();
            Serial.println(currentAngle);
            newIMUSample = false;
        }
    }
}
