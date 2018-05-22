/******************************************************************************
* Author: Team MotoGuardian
* Class:  ECE 140B
*
* File Name: MegaSMS.ino
* Description: Sends SMS to designated contact upon receiving appropriate signal
*              from master controller.
******************************************************************************/
// Include necessary libraries.
#include "Adafruit_FONA.h"

// Define global constants.
#define FONA_RX 18
#define FONA_TX 19
#define FONA_RST 4
#define TRIGGER_PIN 8
#define TRIGGER_RST 9
#define TRIGGER_THRESH 500

// Declare and initialize global variables.
bool allowMsg;
HardwareSerial *fonaSerial = &Serial1;
Adafruit_FONA fona = Adafruit_FONA(FONA_RST);
char smsaddr[] = "+18317940460";
char smsmsg[] = "This is a message from your MotoGuardian. A threat to your vehicle has been detected. "
                "Please inspect it.\n\nIf this was a false alarm, you may reset the system by disarming "
                "and then re-arming your MotoGuardian.\n\nIf your vehicle has been stolen, please call 9-1-1 "
                "to report the theft, and follow this link <insert-link-here> to begin tracking your vehicle.";


/*** Setup Function. Run once @ power up. ***/
void setup() {
    fonaSerial->begin(4800);
    if (! fona.begin(*fonaSerial)) {
        while (1);
    }

    pinMode(TRIGGER_PIN, INPUT);
    allowMsg = true;
}


/*** Main Control Loop ***/
void loop() {
    if ((analogRead(TRIGGER_PIN) >= TRIGGER_THRESH) && allowMsg) {
        fona.sendSMS(smsaddr, smsmsg);
        allowMsg = false;
    }
    else if (analogRead(TRIGGER_RST) >= TRIGGER_THRESH)
        allowMsg = true;
<<<<<<< HEAD
}
=======
}
>>>>>>> parent of b796ae7... Arduino Changes
