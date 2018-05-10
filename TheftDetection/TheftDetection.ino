i/******************************************************************************
* Author: Team MotoGuardian
* Class:  ECE 140B
*
* File Name: TheftDetection.ino
* Description: Master control software for theft detection using Arduino 101 IMU.
*              Sends signal to slave notify end-user.
******************************************************************************/
// Include necessary libraries.
#include <CurieTimerOne.h>
#include <BMI160.h>
#include <CurieIMU.h>
#include <math.h>

// Define global constants.
#define TRIGGER_PIN 8
#define TRIGGER_RST 9
const byte sensitvity = 20; // 1-More sensitive -> 100-Less Sensitive
const float HPF = 0.98;  // High-Pass filter coeff. LPF = 1 - HPF.
const int samplePeriod = 100000;  //microsec
const bool verbosity = false;

// Declare and initialize global variables.
volatile bool newSample = false;
bool armed = false;
bool onAlert = false;
float A_x, A_y, A_z;
float G_x, G_y, G_z;
float currentAngle;
float oldAngle;
char command;


/*** ISR to sample IMU ***/
void sampleIMU() {
    CurieIMU.readAccelerometerScaled(A_x, A_y, A_z);
    CurieIMU.readGyroScaled(G_x, G_y, G_z);
    newSample = true;
}


/*** Calculates roll angle based on complementary filter results of IMU measurements ***/
float getRollAngle() {
    // Declare and initialize variables.
    oldAngle = currentAngle;
    float accelAngle = atan2(A_y, sqrt((A_x*A_x) + (A_z*A_z))) * 180 / PI;
    
    return HPF*(currentAngle + (G_x*samplePeriod/1000000)) + (1-HPF)*accelAngle;
}


/*** Helper Function to calculate percent change ***/
float percentChange(float newNum, float oldNum) {
    float PC = abs(((newNum - oldNum)/(oldNum)) * 100);

    if (PC < 1000) {
        if (verbosity) {
            Serial.print("Percent Change: ");
            Serial.println(PC);
        }
        
        return PC;
    }
    else
        return 0;
}


/*** Helper Function to trigger SMS alert ***/
void triggerSMS() {
    Serial.println("SMS Alert Sent!");
    onAlert = true;
    digitalWrite(TRIGGER_PIN, HIGH);
    delay(100);
    digitalWrite(TRIGGER_PIN, LOW);
}


/*** Setup Function. Run once @ power up. ***/
void setup() {
    // Initialize state.
    armed = false;
    onAlert = false;
    newSample = false;
    
    // Set pins.
    pinMode(TRIGGER_PIN, OUTPUT);
    pinMode(TRIGGER_RST, OUTPUT);

    // Initialize serial port.
    Serial.begin(115200);
    while (!Serial);

    // Print instructions and set initial state.
    Serial.println("Enter 'A' to arm system or 'D' to disarm at any time");
    Serial.println("Please set initial state.");
    while (!Serial.available());
    if (Serial.available()) {
        command = Serial.read();
        if (command == 'A') {
            Serial.println("Ok. System Armed.");
            armed = true;
        }
        else if (command == 'D') {
            Serial.println("Ok. System Disarmed.");
            armed = false;
            onAlert = false;
        }
    }

    // Initialize IMU, set ranges, and get initial angle.
    CurieIMU.begin();
    CurieIMU.setAccelerometerRange(15);
    CurieIMU.setGyroRange(250);
    CurieIMU.readAccelerometerScaled(A_x, A_y, A_z);
    currentAngle = atan2(A_y, sqrt((A_x*A_x) + (A_z*A_z))) * 180 / PI;
    oldAngle = currentAngle;

    // Initialize CurieTimer.
    CurieTimerOne.start(samplePeriod, &sampleIMU);
}


/*** Main Control Loop ***/
void loop() {
    while (Serial) {
    
        // Check for arm/disarm command.
        if (Serial.available()) {
            command = Serial.read();
            if (command == 'A') {
                armed = true;
                Serial.println("Ok. System Armed.");
            }
            else if (command == 'D') {
                armed = false;
                onAlert = false;
                Serial.println("Ok. System Disarmed.");
                digitalWrite(TRIGGER_RST, HIGH);
                delay(100);
                digitalWrite(TRIGGER_RST, LOW);
            }
        }
    
        // Check for potential thefts.
        if (newSample) {
            currentAngle = getRollAngle();
            if (armed && (percentChange(currentAngle, oldAngle) > sensitvity) && !onAlert)
                triggerSMS();
                
            newSample = false;
        }
    }
}
