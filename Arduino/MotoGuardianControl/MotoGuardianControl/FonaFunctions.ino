/******************************************************************************
* Author: Team MotoGuardian
* Class:  ECE 140B
*
* File Name: FonaFunctions.ino
* Description: Contains functions pertaining to FONA.
******************************************************************************/

/*** Initializes FONA, getting all relevant featues ready for use ***/
bool initFONA() {
    // Initialize serial port.
    Serial1.begin(57600);
    
    // Connect to FONA.
    if (!fona.begin(Serial1))
        return false;
    else {
        wait(15000); // Wait for 15 sec for FONA to get connected to network
    }

    // Turn GPRS on.
    if (!fona.enableGPRS(true))
        return false;
    else {
        wait(500);
    }

    // Set to accept SSL redirects.
    fona.setHTTPSRedirect(true);

    // Enable network time sync.
    if (!fona.enableNTPTimeSync(true, F("pool.ntp.org")))
        return false;
    else {
        wait(500);
    }
    if (!fona.enableNetworkTimeSync(true))
        return false;
    else {
        wait(500);
    }

    // Turn GPS on.
    if (!fona.enableGPS(true))
        return false;

    wait(2000);
    queryGPS();
    
    // Everything checks out thus far...
    return true;
}


/*** Imports user settings from DB ***/
bool importSettings() {
    // Define local variables.
    DynamicJsonBuffer jb;
    uint16_t statuscode;
    int16_t length;

    // GET request
    if (!fona.HTTP_GET_start((char*)SETTINGS_URL, &statuscode, (uint16_t *)&length))
        return false;

    // Parse JSON.
    JsonObject& settings = jb.parseObject(fona);
    if (!settings.success())
        return false;

    // Store settings in global variables.
    ownerPhoneNum = settings.get<const char*>("cellphone");
    contactPhoneNum = settings.get<const char*>("emergency_number");
    sensitivity = settings.get<int>("sensitivity");
    tripTrackingEnabled = settings.get<bool>("trip_tracking");
    audibleAlarmEnabled = settings.get<bool>("anti_theft");
    contactName = settings.get<const char*>("emergency_name");
    sprintf(ownerName, "%s %s", settings.get<const char*>("first_name"),
                                settings.get<const char*>("last_name"));
    sprintf(vehicleInfo, "%s %s %s %s", settings.get<const char*>("color"),
                                        settings.get<const char*>("year"),
                                        settings.get<const char*>("make"), 
                                        settings.get<const char*>("model"));

    return true;
}


/*** Retrieves GPS info ***/
bool queryGPS() {
    // Define local variables.
    unsigned long startTime;
    float speed_kph = 0;
    float heading = 0;
    float altitude = 0;
    bool gps_success = false;

    // Make query.
    startTime = millis() ;
    gps_success = fona.getGPS(&currentLatitude, &currentLongitude, &speed_kph, &heading, &altitude);
    while (!gps_success) {
        wait(2000);
        gps_success = fona.getGPS(&currentLatitude, &currentLongitude, &speed_kph, &heading, &altitude);
        if (millis() - startTime > 60000)
            break;
    }

    if (gps_success) {
        currentSpeed = speed_kph * 0.621371192;
        if (DEBUG) {
            wait(1000);
            fona.sendSMS((char*)"+18317940460", (char*)"GPS query success");
        }
        return true;
    }
    else {
        if (DEBUG) {
            wait(1000);
            fona.sendSMS((char*)"+18317940460", (char*)"GPS query failed");
        }
        return false;
    }
}


/*** POSTs notification to DB ***/
bool postNotification(byte type) {
    // Define local variables.
    DynamicJsonBuffer jb;
    uint16_t statuscode;
    int16_t length;
    char data[300];

    // Get location.
    if (!queryGPS()) {
        return false;
    }

    // Make JSON object.
    JsonObject& notification = jb.createObject();
    notification["device_IMEI"] = IMEI;
    notification["lat"] = currentLatitude;
    notification["lng"] = currentLongitude;
    if (type == 0)
        notification["notification_type"] = "security_armed";
    else if (type == 1)
        notification["notification_type"] = "security_disarmed";
    else if (type == 2)
        notification["notification_type"] = "ignition_on";
    else if (type == 3)
        notification["notification_type"] = "ignition_off";
    else if (type == 4)
        notification["notification_type"] = "theft_detected";
    else if (type == 5)
        notification["notification_type"] = "crash_detected";
    else
        return false;
    notification.printTo(data);

    // Make the POST request.
    wait(1000);
    if (!fona.HTTP_POST_start((char*)NOTIFICATION_URL, F("application/json"), (uint8_t *) data, strlen(data), &statuscode, (uint16_t *)&length)) {
        if (DEBUG) {
            wait(1000);
            fona.sendSMS((char*)"+18317940460", (char*)"Post Failed");
            wait(1000);
            fona.sendSMS((char*)"+18317940460", (char*)data);
        }
        return false;
    }
    else {
        // Update values for last posted GPS info.
        postedLatitude = currentLatitude;
        postedLongitude = currentLongitude;
        postedSpeed = currentSpeed;
        
        wait(1000);
        if (DEBUG)
            fona.sendSMS((char*)"+18317940460", (char*)data);
        return true;
    }
}

