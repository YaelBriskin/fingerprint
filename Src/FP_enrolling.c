
#include "../Inc/FP_enrolling.h"

void enrolling(uint16_t pageId)
{

    int ack = -1;
    printf("Waiting finger to enroll\r\n ");
    // ack=(int)getParameters();

    while (ack != FINGERPRINT_OK)
    {
        // ack=(int)communicate_link();
        ack = (int)getImage();
        switch (ack)
        {
        case FINGERPRINT_OK:
            printf("Finger collection success\r\n ");
            break;
        case FINGERPRINT_PACKETRECIEVER:
            printf("Error when receiving package\r\n ");
            break;
        case FINGERPRINT_NOFINGER:
            printf("Cant detect finger\r\n ");
            break;
        case FINGERPRINT_IMAGEFAIL:
            printf("Fail to collect finger\r\n ");
            break;
        default:;
        }
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

    while (getImage() != FINGERPRINT_NOFINGER)
        ;
    ack = FINGERPRINT_NOFINGER;
    printf("Waiting finger to enroll\r\n ");
    while (ack != FINGERPRINT_OK)
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
            printf("Can’t detect finger\r\n ");
            break;
        case FINGERPRINT_IMAGEFAIL:
            printf("Fail to collect finger\r\n ");
            break;
        default:;
        }
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
    {
    }
    else if (ack == FINGERPRINT_PACKETRECIEVER)
    {
    }
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
            break;
        case FINGERPRINT_ENROLLMISMATCH:
            printf("Fail to combine the character files. That’s, the character files don’t belong to one finger\r\n ");
            break;
        }

        ack = storeModel(pageId);
        switch (ack)
        {
        case FINGERPRINT_OK:
            printf("Storage success\r\n ");
            break;
        case FINGERPRINT_PACKETRECIEVER:
            printf("Error when receiving package\r\n ");
            break;
        case FINGERPRINT_BADLOCATION:
            printf("Addressing PageID is beyond the finger library\r\n ");
            break;
        case FINGERPRINT_FLASHERR:
            printf("Error when writing Flash\r\n ");
            break;
        }
    }
}