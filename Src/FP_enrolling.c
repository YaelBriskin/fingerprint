#include "../Inc/FP_enrolling.h"

/**
 * @brief Initiates the process of enrolling a new fingerprint template.
 *
 * This function initiates the process of enrolling a new fingerprint template
 * by guiding the user through the necessary steps, such as placing their finger
 * on the fingerprint sensor and capturing the fingerprint image.
 *
 * @param pageId The page ID where the enrolled template will be stored.
 * @return int Returns 1 if the enrollment process is successful, otherwise returns an error code.
 */
int enrolling(uint16_t pageId)
{
    lcd20x4_i2c_clear();
    struct timespec start_time;
    const int max_execution_time = 60;
    struct timespec current_time;
    int ack = -1;
    int previous_ack = -1;

    // Display initial message
    lcd20x4_i2c_puts(1, 0, "Waiting finger to enroll");
    sleep(2);
    // Start timer
    clock_gettime(CLOCK_MONOTONIC, &start_time);
    // Loop until fingerprint enrollment is complete or timeout occurs
    while (ack != FINGERPRINT_OK)
    {
        // Check timeout
        clock_gettime(CLOCK_MONOTONIC, &current_time);
        long elapsed_time = (current_time.tv_sec - start_time.tv_sec) + (current_time.tv_nsec - start_time.tv_nsec) / 1000000000;
        if (elapsed_time >= max_execution_time)
        {
            lcd20x4_i2c_puts(1, 0, "Timeout: One minute has passed.");
            syslog_log(LOG_ERR, __func__, "stderr", "Timeout: One minute has passed");
            sleep(2);
            lcd20x4_i2c_clear();
            return FINGERPRINT_TIMEOUT;
        }
        // Get fingerprint image
        ack = (int)getImage();
        // Handle different response codes
        if (ack != previous_ack)
        {
            lcd20x4_i2c_clear();
            switch (ack)
            {
            case FINGERPRINT_OK:
                syslog_log(LOG_INFO, __func__, "OK", "Finger collection success");
                lcd20x4_i2c_puts(1, 0, "Finger collection success");
                break;
            case FINGERPRINT_PACKETRECIEVER:
                syslog_log(LOG_ERR, __func__, "stderr", "Error when receiving package");
                lcd20x4_i2c_puts(1, 0, "Error when receiving package");
                break;
            case FINGERPRINT_NOFINGER:
                syslog_log(LOG_ERR, __func__, "stderr", "Can't detect finger");
                lcd20x4_i2c_puts(1, 0, "Can't detect finger");
                break;
            case FINGERPRINT_IMAGEFAIL:
                syslog_log(LOG_ERR, __func__, "stderr", "Fail to collect finger");
                lcd20x4_i2c_puts(1, 0, "Fail to collect finger");
                break;
            default:;
            }
            previous_ack = ack; // Update previous_ack for next iteration
        }
    }
    ack = ERROR;
    while (ack != FINGERPRINT_OK)
    {
        // Convert image to template
        ack = image2Tz(1);
        // Handle conversion result
        switch (ack)
        {
        case FINGERPRINT_OK:
            syslog_log(LOG_INFO, __func__, "OK", "Generate character file complete");
            break;
        case FINGERPRINT_PACKETRECIEVER:
            syslog_log(LOG_ERR, __func__, "stderr", "Error when receiving package");
            break;
        case FINGERPRINT_IMAGEMESS:
            syslog_log(LOG_ERR, __func__, "stderr", "Fail to generate character file due to the over-disorderly fingerprint image");
            break;
        case FINGERPRINT_FEATUREFAIL:
            syslog_log(LOG_ERR, __func__, "stderr", "Fail to generate character file due to lackness of character point or over-smallness of fingerprint image");
            break;
        case FINGERPRINT_INVALIDIMAGE:
            syslog_log(LOG_ERR, __func__, "stderr", "Fail to generate the image for the lackness of valid primary image");
            break;
        }
    }
    // Check if fingerprint already exists
    if (fingerFastSearch() == FINGERPRINT_OK)
    {
        lcd20x4_i2c_puts(1, 0, "Fingerprint already exists");
        syslog_log(LOG_ERR, __func__, "stderr", "Attempt to re-register the fingerprint");
        sleep(SLEEP_LCD);
        lcd20x4_i2c_clear();
        return ERROR;
    }
    // Prompt for re-enrollment
    while (getImage() != FINGERPRINT_NOFINGER)
        ;
    ack = FINGERPRINT_NOFINGER;
    lcd20x4_i2c_puts(1, 0, "Put finger to enroll again");
    sleep(SLEEP_LCD);
    previous_ack = -1;
    // Re-enrollment process
    while (ack != FINGERPRINT_OK)
    {
        // Check timeout
        clock_gettime(CLOCK_MONOTONIC, &current_time);
        long elapsed_time = (current_time.tv_sec - start_time.tv_sec) + (current_time.tv_nsec - start_time.tv_nsec) / 1000000000;
        if (elapsed_time >= max_execution_time)
        {
            lcd20x4_i2c_puts(1, 0, "Timeout: One minute has passed.");
            syslog_log(LOG_ERR, __func__, "stderr", "Timeout: One minute has passed");
            sleep(SLEEP_LCD);
            lcd20x4_i2c_clear();
            return FINGERPRINT_TIMEOUT;
        }
        // Get fingerprint image
        ack = getImage();
        // Handle different response codes
        if (ack != previous_ack)
        {
            lcd20x4_i2c_clear();
            // Handle conversion result
            switch (ack)
            {
            case FINGERPRINT_OK:
                syslog_log(LOG_ERR, __func__, "OK", "Finger collection success");
                lcd20x4_i2c_puts(1, 0, "Finger collection success");
                break;
            case FINGERPRINT_PACKETRECIEVER:
                syslog_log(LOG_ERR, __func__, "stderr", "Error when receiving package");
                lcd20x4_i2c_puts(1, 0, "Error when receiving package");
                break;
            case FINGERPRINT_NOFINGER:
                syslog_log(LOG_ERR, __func__, "stderr", "Can't detect finger");
                lcd20x4_i2c_puts(1, 0, "Can't detect finger");
                break;
            case FINGERPRINT_IMAGEFAIL:
                syslog_log(LOG_ERR, __func__, "stderr", "Fail to collect finger ");
                lcd20x4_i2c_puts(1, 0, "Fail to collect finger ");
                break;
            default:;
            }
            previous_ack = ack; // Update previous_ack for next iteration
        }
    }
    sleep(SLEEP_LCD);
    lcd20x4_i2c_clear();
    // Convert image to template
    ack = image2Tz(2);
    // Handle conversion result
    switch (ack)
    {
    case FINGERPRINT_OK:
        syslog_log(LOG_ERR, __func__, "OK", "Generate character file complete");
        break;
    case FINGERPRINT_PACKETRECIEVER:
        syslog_log(LOG_ERR, __func__, "stderr", "Error when receiving package");
        break;
    case FINGERPRINT_IMAGEMESS:
        syslog_log(LOG_ERR, __func__, "stderr", "Fail to generate character file due to the over-disorderly fingerprint image");
        break;
    case FINGERPRINT_FEATUREFAIL:
        syslog_log(LOG_ERR, __func__, "stderr", "Fail to generate character file due to lackness of character point or over-smallness of fingerprint image");
        break;
    case FINGERPRINT_INVALIDIMAGE:
        syslog_log(LOG_ERR, __func__, "stderr", "Fail to generate the image for the lackness of valid primary image");
        break;
    default:;
    }
    // Create fingerprint model
    ack = createModel();
    switch (ack)
    {
    case FINGERPRINT_OK:
        syslog_log(LOG_ERR, __func__, "OK", "Operation success");
        break;
    case FINGERPRINT_PACKETRECIEVER:
        syslog_log(LOG_ERR, __func__, "stderr", "Error when receiving package");
        return FAILED;
    case FINGERPRINT_ENROLLMISMATCH:
        ack = createModel();
        switch (ack)
        {
        case FINGERPRINT_OK:
            syslog_log(LOG_ERR, __func__, "OK", "Operation success");
            break;
        case FINGERPRINT_PACKETRECIEVER:
            syslog_log(LOG_ERR, __func__, "stderr", "Error when receiving package");
            return FAILED;
        case FINGERPRINT_ENROLLMISMATCH:
            syslog_log(LOG_ERR, __func__, "stderr", "Fail to combine the character files. That’s, the character files don’t belong to one finger");
            return FAILED;
        default:;
        }
    }
    // Store fingerprint model
    ack = storeModel(pageId);
    switch (ack)
    {
    case FINGERPRINT_OK:
        syslog_log(LOG_ERR, __func__, "OK", "Storage success");
        return SUCCESS;
    case FINGERPRINT_PACKETRECIEVER:
        syslog_log(LOG_ERR, __func__, "stderr", "Error when receiving package");
        return FAILED;
    case FINGERPRINT_BADLOCATION:
        syslog_log(LOG_ERR, __func__, "stderr", "Addressing PageID is beyond the finger library");
        return FAILED;
    case FINGERPRINT_FLASHERR:
        syslog_log(LOG_ERR, __func__, "stderr", "Error when writing Flash");
        return FAILED;
    default:;
    }
}