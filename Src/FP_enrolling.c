#include "../Inc/FP_enrolling.h"

int enrolling(uint16_t pageId)
{
    lcd20x4_i2c_clear();
    struct timespec start_time;
    const int max_execution_time = 60;
    struct timespec current_time;
    int ack = -1;
    int previous_ack = -1;
    lcd20x4_i2c_puts(1,0,"Waiting finger to enroll");
    sleep(2);
    clock_gettime(CLOCK_MONOTONIC, &start_time);
    while (ack != FINGERPRINT_OK)
    {
        clock_gettime(CLOCK_MONOTONIC, &current_time);
        long elapsed_time = (current_time.tv_sec - start_time.tv_sec) + (current_time.tv_nsec - start_time.tv_nsec) / 1000000000;
        if (elapsed_time >= max_execution_time) 
        {
            lcd20x4_i2c_puts(1, 0,"Timeout: One minute has passed.");
            sleep(2);
            lcd20x4_i2c_clear();
            return FINGERPRINT_TIMEOUT;
        }
        ack = (int)getImage();
        if (ack != previous_ack)
        {
            lcd20x4_i2c_clear();
            switch (ack)
            {
            case FINGERPRINT_OK:
                lcd20x4_i2c_puts(1, 0,"Finger collection success");
                break;
            case FINGERPRINT_PACKETRECIEVER:
                lcd20x4_i2c_puts(1, 0,"Error when receiving package");
                break;
            case FINGERPRINT_NOFINGER:
                //printf("Can't detect finger");
                lcd20x4_i2c_puts(1, 0,"Can't detect finger");
                break;
            case FINGERPRINT_IMAGEFAIL:
                lcd20x4_i2c_puts(1, 0,"Fail to collect finger");
                break;
            default:;
            }
            previous_ack = ack; // Update previous_ack for next iteration
        }
    }
    ack = -1;
    while (ack != FINGERPRINT_OK)
    {
        ack = image2Tz(1);
        switch (ack)
        {
        case FINGERPRINT_OK:
            printf("Generate character file complete\r\n ");
            break;
        case FINGERPRINT_PACKETRECIEVER:
            printf("Error when receiving package\r\n ");
            break;
        case FINGERPRINT_IMAGEMESS:
            printf("Fail to generate character file due to the over-disorderly fingerprint image\r\n ");
            break;
        case FINGERPRINT_FEATUREFAIL:
            printf("Fail to generate character file due to lackness of character point or over-smallness of fingerprint image\r\n ");
            break;
        case FINGERPRINT_INVALIDIMAGE:
            printf("Fail to generate the image for the lackness of valid primary image\r\n ");
            break;
        }
    }
    ack = fingerFastSearch();
    if (ack == FINGERPRINT_OK) 
    {
        lcd20x4_i2c_puts(1, 0,"Fingerprint already exists");
        return -1; 
    }
    while (getImage() != FINGERPRINT_NOFINGER)
        ;
    ack = FINGERPRINT_NOFINGER;
    lcd20x4_i2c_puts(1, 0,"Put finger to enroll again");
    sleep(2);
    previous_ack = -1;
    while (ack != FINGERPRINT_OK)
    {
        clock_gettime(CLOCK_MONOTONIC, &current_time);
        long elapsed_time = (current_time.tv_sec - start_time.tv_sec) + (current_time.tv_nsec - start_time.tv_nsec) / 1000000000;
        if (elapsed_time >= max_execution_time) 
        {
            lcd20x4_i2c_puts(1, 0,"Timeout: One minute has passed.");
            sleep(3);
            lcd20x4_i2c_clear();
            return FINGERPRINT_TIMEOUT;
        }
        ack = getImage();
        if (ack != previous_ack)
        {
            lcd20x4_i2c_clear();
            switch (ack)
            {
            case FINGERPRINT_OK:
                lcd20x4_i2c_puts(1, 0, "Finger collection success");
                break;
            case FINGERPRINT_PACKETRECIEVER:
                lcd20x4_i2c_puts(1, 0, "Error when receiving package");
                break;
            case FINGERPRINT_NOFINGER:
                lcd20x4_i2c_puts(1, 0, "Can't detect finger");
                break;
            case FINGERPRINT_IMAGEFAIL:
                lcd20x4_i2c_puts(1, 0, "Fail to collect finger ");
                break;
            default:;
            }
            previous_ack = ack; // Update previous_ack for next iteration
        }
    }
    lcd20x4_i2c_clear();
    ack = image2Tz(2);

    switch (ack)
    {
    case FINGERPRINT_OK:
        printf("Generate character file complete\r\n ");
        break;
    case FINGERPRINT_PACKETRECIEVER:
        printf("Error when receiving package\r\n ");
        break;
    case FINGERPRINT_IMAGEMESS:
        printf("Fail to generate character file due to the over-disorderly fingerprint image\r\n ");
        break;
    case FINGERPRINT_FEATUREFAIL:
        printf("Fail to generate character file due to lackness of character point or over-smallness of fingerprint image\r\n ");
        break;
    case FINGERPRINT_INVALIDIMAGE:
        printf("Fail to generate the image for the lackness of valid primary image\r\n ");
        break;
    }
    ack = createModel();
    switch (ack)
    {
    case FINGERPRINT_OK:
        printf("Operation success\r\n ");
        break;
    case FINGERPRINT_PACKETRECIEVER:
        printf("Error when receiving package\r\n ");
        return 0;
    case FINGERPRINT_ENROLLMISMATCH:
        ack = createModel();
        switch (ack)
        {
        case FINGERPRINT_OK:
            printf("Operation success\r\n ");
            break;
        case FINGERPRINT_PACKETRECIEVER:
            printf("Error when receiving package\r\n ");
            return 0;
        case FINGERPRINT_ENROLLMISMATCH:
            printf("Fail to combine the character files. That’s, the character files don’t belong to one finger\r\n ");
            return 0;
        }
    }
    ack = storeModel(pageId);
    switch (ack)
    {
    case FINGERPRINT_OK:
        printf("Storage success\r\n ");
        return 1;
    case FINGERPRINT_PACKETRECIEVER:
        printf("Error when receiving package\r\n ");
        return 0;
    case FINGERPRINT_BADLOCATION:
        printf("Addressing PageID is beyond the finger library\r\n ");
        return 0;
    case FINGERPRINT_FLASHERR:
        printf("Error when writing Flash\r\n ");
        return 0;
    }
}