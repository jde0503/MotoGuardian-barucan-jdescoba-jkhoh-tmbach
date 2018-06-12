/******************************************************************************
* Author: Team MotoGuardian
* Class:  ECE 140B
*
* File Name: TheftDetectionFunctions.ino
* Description: Contains functions pertaining to theft detection.
******************************************************************************/

/*** ISR to detect motion ***/
static void detectMotion(void){
    if (CurieIMU.getInterruptStatus(CURIE_IMU_MOTION)) {
        if ( (CurieIMU.motionDetected(X_AXIS, POSITIVE)) || (CurieIMU.motionDetected(X_AXIS, NEGATIVE)) ||
             (CurieIMU.motionDetected(Y_AXIS, POSITIVE)) || (CurieIMU.motionDetected(Y_AXIS, NEGATIVE)) ||
             (CurieIMU.motionDetected(Z_AXIS, POSITIVE)) || (CurieIMU.motionDetected(Z_AXIS, NEGATIVE)) )
            theftDetected = true;
    }
}
