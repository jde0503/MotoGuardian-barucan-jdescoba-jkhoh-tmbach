/******************************************************************************
* Author: Team MotoGuardian
* Class:  ECE 140B
*
* File Name: MotoGuardianControl.ino
* Description: Main control program for MotoGuardian device.
******************************************************************************/

/*** Include necessary libraries ***/
#include <CurieTimerOne.h>
#include <BMI160.h>
#include <CurieIMU.h>
#include <ArduinoJson.h>
#include <Adafruit_FONA.h>
#include <math.h>


/*** Define global constants ***/
#define IMEI "865067027614042"
#define SETTINGS_URL "motoguardian.herokuapp.com/device-settings/?mg_imei=865067027614042"
#define NOTIFICATION_URL "motoguardian.herokuapp.com/notification/?mg_imei=865067027614042"
#define TRACKING_URL "motoguardian.herokuapp.com/trip/?mg_imei=865067027614042"
#define FONA_RST 2
#define FONA_KEY 3
#define EMR_CANCEL 4
#define IGN 5
#define RELAY 6
#define REMOTE 7
#define DEBUG true // Set to false and reupload before deployment.


/*** Declare global variables ***/
char ownerName[30];
const char* contactName;
const char* ownerPhoneNum;
const char* contactPhoneNum;
char vehicleInfo[60];
float postedLatitude, postedLongitude, postedSpeed;
float currentLatitude, currentLongitude, currentSpeed;
char crashMsg[400];
int sensitivity = 1;
bool audibleAlarmEnabled = false;
bool tripTrackingEnabled = false;
bool fonaInitialized = false;
bool settingsCurrent = false;
volatile bool armed = false;
volatile bool theftDetected = false;
volatile bool crashDetected = false;
volatile bool ignitionOn = false;
Adafruit_FONA fona = Adafruit_FONA(FONA_RST);
byte mode = 0; // 0 = rest, 1 = monitor security, 2 = theft response,
               // 3 = monitor trip, 4 = crash response
char theftMsg[] = "This is a message from your MotoGuardian. A threat to your "
                  "vehicle has been detected. Please inspect it.\n\nIf this was "
                  "a false alarm, you may reset the system by disarming "
                  "and then re-arming your MotoGuardian.\n\nIf your vehicle has been "
                  "stolen, please call 9-1-1 to report the theft and login to your "
                  "MotoGuardian online dashboard begin tracking your vehicle.";


/*** Function that implements delay() ***/
void wait(unsigned long timeToWait) {
    unsigned long startTime = millis();
    while (millis() - startTime < timeToWait) {
        // Wait
    }
}


/*** Function that blinks LED ***/
void blinkLED() {
    digitalWrite(LED_BUILTIN, HIGH);
    wait(1000);
    digitalWrite(LED_BUILTIN, LOW);
    wait(1000);
}


/*** ISR to change armed state ***/
void toggleArmed() {
    if (!ignitionOn) {
        armed = !armed;

        if (!armed) {
            theftDetected = false;
            CurieIMU.noInterrupts(CURIE_IMU_MOTION);
        }
        else
            CurieIMU.interrupts(CURIE_IMU_MOTION);
    }
}


/*** ISR to change ignition state ***/
void toggleIgnition() {
    if (digitalRead(IGN)) {
        ignitionOn = true;
        if (armed)
            theftDetected = true;
    }
    else
        ignitionOn = false;
}


/*** Setup Function. Run once @ power up. ***/
void setup() {
    // Initialize state.
    sensitivity = 1;
    currentLatitude = 0;
    currentLongitude = 0;
    audibleAlarmEnabled = false;
    tripTrackingEnabled = false;
    settingsCurrent = false;
    mode = 0;
    armed = false;
    theftDetected = false;
    crashDetected = false;
    ignitionOn = false;

    // Initialize digital pins.
    pinMode(LED_BUILTIN, OUTPUT);
    //pinMode(FONA_KEY, OUTPUT);
    //digitalWrite(FONA_KEY, LOW);
    pinMode(RELAY, OUTPUT);
    digitalWrite(RELAY, HIGH);
    pinMode(IGN, INPUT);
    pinMode(REMOTE, INPUT);
    pinMode(EMR_CANCEL, INPUT_PULLUP);

    // Initialize FONA.
    if (initFONA()) {
        fonaInitialized = true;
        if (DEBUG) {
            fona.sendSMS((char*)"+18317940460", (char*)"FONA initialized successfully");
        }

        // Import account settings.
        if (importSettings()) {
            settingsCurrent = true;
            if (DEBUG) {
                fona.sendSMS((char*)"+18317940460", (char*)"Settings imported successfully");
            }
        }
    }

    // Attach interrupts.
    attachInterrupt(digitalPinToInterrupt(REMOTE), toggleArmed, RISING);
    attachInterrupt(digitalPinToInterrupt(IGN), toggleIgnition, CHANGE);

    // Initialize IMU and set interrupt for motion detection.
    CurieIMU.begin();
    CurieIMU.attachInterrupt(detectMotion);

    // Enable motion detection.
    CurieIMU.setDetectionThreshold(CURIE_IMU_MOTION, (sensitivity * 10)); // 20mg
    CurieIMU.setDetectionDuration(CURIE_IMU_MOTION, 10);  // trigger times of consecutive slope data points
    CurieIMU.interrupts(CURIE_IMU_MOTION);

    // Disable motion interrupt until system is armed.
    CurieIMU.noInterrupts(CURIE_IMU_MOTION);
}



/*** Main Control Loop ***/
void loop() {
    switch (mode) {
        case 0: // Resting
            while (!armed && !ignitionOn) {
                // Wait
            }
            // If Ignition is on, send notifications and enter monitor trip mode.
            if (ignitionOn) {
                mode = 3;
            }
            // If ignition is off and device has been armed, enter monitor security mode.
            else if (armed) {
                postNotification(0);
                if (DEBUG) {
                    wait(1000);
                    fona.sendSMS((char*)"+18317940460", (char*)"MotoGuardian armed");
                }
                mode = 1;
            }
            break;

        case 1: // Monitor security
            while (armed && !theftDetected) {
                // Wait
            }
            // If disarmed, send notification and enter resting mode.
            if (!armed) {
                postNotification(1);
                if (DEBUG) {
                    wait(1000);
                    fona.sendSMS((char*)"+18317940460", (char*)"MotoGuardian disarmed");
                }
                mode = 0;
            }
            // If armed and theft has been detected, enter theft response mode.
            else if (theftDetected)
                mode = 2;
            break;

        case 2: // Theft response
            // Send notifications
            fona.sendSMS((char*)ownerPhoneNum, (char*)theftMsg);
            wait(1000);
            postNotification(4);

            
            while (armed && theftDetected) {
                // Track the motorcycle, engage alarm.
            }
            // If disarmed, send notification and return to resting mode.
            postNotification(1);
            if (DEBUG) {
                wait(1000);
                fona.sendSMS((char*)"+18317940460", (char*)"MotoGuardian disarmed");
            }
            mode = 0;
            break;

        case 3: // Monitor trip
            // Import settings.

            // Get starting location.
            while (ignitionOn && !crashDetected) {
                // Wait for trip to actually begin.
                // Track trips and look for crashes.
            }
            // If a crash is detected, enter crash response mode.
            if (crashDetected)
                mode = 4;
            // If a crash has not been detected and the ignition has been turned off, enter resting mode.
            else if (!ignitionOn)
                mode = 0;
            break;

        case 4: // Crash response
            // Warn about upcoming call via honking.
            // Update location.
            // Make notification
            // Make emergncy call.
            while (crashDetected) {
                // Honk SoS
            }
            // If crash detection is reset and ignition is off, return to resting mode.
            if (!ignitionOn)
                mode = 0;
            // If crash detection is reset and igniton is on, return to trip monitoring mode.
            else
                mode = 3;

        default:
            mode = 0;
            break;
    }
}
