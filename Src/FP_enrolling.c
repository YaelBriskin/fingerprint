
#include "../Inc/FP_enrolling.h"

int enrolling(uint16_t pageId)
{

    int ack = -1;
    showMessageOnDisplay("Waiting finger to enroll",-1);
    sleep(1);

    clock_t start_time;
	const clock_t max_execution_time = 30 * CLOCKS_PER_SEC;

    start_time = clock();
	while (ack != FINGERPRINT_OK && (clock() - start_time) <= max_execution_time)
	{
        ack = (int)getImage();
        switch (ack)
        {
        case FINGERPRINT_OK:
            showMessageOnDisplay("Finger collection success",1,0);
            break;
        case FINGERPRINT_PACKETRECIEVER:
            showMessageOnDisplay("Error when receiving package",1,0);
            break;
        case FINGERPRINT_NOFINGER:
            showMessageOnDisplay("Can't detect finger",1,0);
            break;
        case FINGERPRINT_IMAGEFAIL:
            showMessageOnDisplay("Fail to collect finger",1,0);
            break;
        default:;
        }
    }
    if ((clock() - start_time) > max_execution_time) 
    {
        showMessageOnDisplay("Timeout: One minute has passed.",1,0);
        return FINGERPRINT_TIMEOUT;
    }
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
    printf("Clear sensor\r\n ");
    sleep(2);

    while (getImage() != FINGERPRINT_NOFINGER)
        ;
    ack = FINGERPRINT_NOFINGER;
    printf("Waiting finger to enroll\r\n ");
    
    start_time = clock();
	while (ack != FINGERPRINT_OK && (clock() - start_time) <= max_execution_time)
    {
        ack = getImage();
        switch (ack)
        {
        case FINGERPRINT_OK:
            printf("Finger collection success\r\n ");
            break;
        case FINGERPRINT_PACKETRECIEVER:
            printf("Error when receiving package\r\n ");
            break;
        case FINGERPRINT_NOFINGER:
            printf("Can't detect finger\r\n ");
            break;
        case FINGERPRINT_IMAGEFAIL:
            printf("Fail to collect finger\r\n ");
            break;
        default:;
        }
    }
    if ((clock() - start_time) > max_execution_time) 
    {
        printf("Timeout: One minute has passed.\r\n");
        return FINGERPRINT_TIMEOUT;
    }
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
    if (ack == FINGERPRINT_OK)
        return 1;
    else if (ack == FINGERPRINT_PACKETRECIEVER)
        return 0;
    else if (ack == FINGERPRINT_ENROLLMISMATCH)
    {
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

        ack = storeModel(pageId);
        switch (ack)
        {
        case FINGERPRINT_OK:
            printf("Storage success\r\n ");
            break;
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
    return 1; 
}