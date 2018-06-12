/******************************************************************************
* Author: Team MotoGuardian
* Class:  ECE 140B
*
* File Name: TheftDetection.ino
* Description: Implements Theft Detection for MotoGuardian.
******************************************************************************/
// Include necessary libraries.
#include <CurieTimerOne.h>
#include <BMI160.h>
#include <CurieIMU.h>
#include <Adafruit_FONA.h>


// Define global constants.
#define FONA_RST 2
#define RF_RX 7
#define LED 13
const bool debug = true;  // Set to false and reupload before using with FONA.
char smsaddr[] = "+18317940460";
char smsmsg[] = "This is a message from your MotoGuardian. A threat to your vehicle has been detected. "
                "Please inspect it.\n\nIf this was a false alarm, you may reset the system by disarming "
                "and then re-arming your MotoGuardian.\n\nIf your vehicle has been stolen, please call 9-1-1 "
                "to report the theft and login to you MotoGuardian online dashboard begin tracking your vehicle.";


// Declare and initialize global variables.
volatile bool newState = false;
volatile bool newMotionDetected = false;
bool armed = false;
bool alertOn = false;
Adafruit_FONA fona = Adafruit_FONA(FONA_RST);


/*** ISR to change armed state ***/
void notifyChangeOfState() {
    newState = true;
}


/*** ISR to detect motion ***/
static void detectMotion(void){
    if (CurieIMU.getInterruptStatus(CURIE_IMU_MOTION)) {
        if ( (CurieIMU.motionDetected(X_AXIS, POSITIVE)) || (CurieIMU.motionDetected(X_AXIS, NEGATIVE)) ||
             (CurieIMU.motionDetected(Y_AXIS, POSITIVE)) || (CurieIMU.motionDetected(Y_AXIS, NEGATIVE)) ||
             (CurieIMU.motionDetected(Z_AXIS, POSITIVE)) || (CurieIMU.motionDetected(Z_AXIS, NEGATIVE)) )
            newMotionDetected = true;
    }
}


/*** Helper function to change Armed/Disarmed state ***/
void changeState() {
    armed = !armed;
    if (!armed) {
        alertOn = false;
        CurieIMU.noInterrupts(CURIE_IMU_MOTION);
    }
    else
        CurieIMU.interrupts(CURIE_IMU_MOTION);
        
    if (debug) {
        if (armed) {
            Serial.println("System armed.");
        }
        else {
            Serial.println("System Disarmed.");
        }
    }
    
    if (armed)
        digitalWrite(LED, HIGH);
    else
        digitalWrite(LED, LOW);
}


/*** Helper function to send SMS alert ***/
void sendAlert() {
    if (debug)
        Serial.println("Motion Detected! SMS Alert Sent!");
    else
        fona.sendSMS(smsaddr, smsmsg);

    alertOn = true;
}


/*** Setup Function. Run once @ power up. ***/
void setup() {
    // Initialize state.
    armed = true;
    alertOn = false;
    newState = false;
    newMotionDetected = false;
    
    // Set pins.
    pinMode(RF_RX, INPUT);
    pinMode(LED, OUTPUT);

    // Initialize serial port.
    if (debug) {
        Serial.begin(115200);
        while (!Serial);
    }
    else {
        Serial1.begin(4800);
        if (!fona.begin(Serial1))
            while (1);
    }

    // Attach interrupt for RF remote.
    attachInterrupt(digitalPinToInterrupt(RF_RX), notifyChangeOfState, RISING);

    // Initialize IMU and set interrupt for motion detection.
    CurieIMU.begin();
    CurieIMU.attachInterrupt(detectMotion);

    // Enable Motion Detection.
    CurieIMU.setDetectionThreshold(CURIE_IMU_MOTION, 120); // 20mg
    CurieIMU.setDetectionDuration(CURIE_IMU_MOTION, 10);  // trigger times of consecutive slope data points
    CurieIMU.interrupts(CURIE_IMU_MOTION);

    // Disable motion interrupt until system is armed.
    CurieIMU.noInterrupts(CURIE_IMU_MOTION);
}


/*** Main Control Loop ***/
void loop() {
    // Check for change in armed/disarmed state.
    if (newState) {
        changeState();
        newState = false;
    }

    // Check whether motion has been detected.
    if (newMotionDetected) {
        if (!alertOn && armed)
            sendAlert();
        newMotionDetected = false;
    }
}
