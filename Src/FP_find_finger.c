#include "../Inc/FP_find_finger.h"


char mydata[23] = { 0 };
extern uint8_t fingerID[2];

int stringToInt(const char* str) 
{
    int result = 0;
    int i = 0;
    // Convert each digit in the string to a number
    while (str[i] != '\0') 
    {
        if (str[i] >= '0' && str[i] <= '9') 
            result = result * 10 + (str[i] - '0');
        else 
        {
            // Handle invalid characters
            fprintf(stderr, "Invalid character in string: %c\n", str[i]);
            return 0; 
        }
        i++;
    }
    return result;
}
//tries to scan your fingerprint for only 1 minute
int findFinger(const char* message)
{
	char num[2] = { 0 };
	int ack = -1;
	lcd20x4_i2c_puts(1, 0,"Waiting finger to enroll");


	clock_t start_time = clock();
	const clock_t max_execution_time = 30 * CLOCKS_PER_SEC;

	while (ack != FINGERPRINT_OK && (clock() - start_time) <= max_execution_time)
	{
		//detecting finger and store the detected finger image in ImageBuffer while returning successfull confirmation code; If there is no finger, returned confirmation code would be cant detect finger.
		ack = (int) getImage();
		switch (ack)
		{
			//checks how the procedure went. FINGERPRINT_OK means good
		case FINGERPRINT_OK:
			lcd20x4_i2c_puts(1, 0,"Finger collection success");
			break;
		case FINGERPRINT_PACKETRECIEVER:
			lcd20x4_i2c_puts(1, 0,"Error when receiving package");
			break;
		case FINGERPRINT_NOFINGER:
			printf("Cant detect finger\r\n");
			lcd20x4_i2c_puts(1, 0,"Cant detect finger");
			break;
		case FINGERPRINT_IMAGEFAIL:
			lcd20x4_i2c_puts(1, 0,"Fail to collect finger");
			break;
		default:
			;
		}
		usleep(100);
	}
	if ((clock() - start_time) > max_execution_time) 
    {
        lcd20x4_i2c_puts(1, 0,"Timeout: One minute has passed.");
        return FINGERPRINT_TIMEOUT;
    }
	//to generate character file from the original finger image in ImageBuffer and store the file in CharBuffer1 or CharBuffer2.
	ack = image2Tz(1);
	switch (ack)
	{
	//checks how the procedure went. FINGERPRINT_OK means good
	case FINGERPRINT_OK:
		lcd20x4_i2c_puts(1, 0,"Generate character file complete");
		break;
	case FINGERPRINT_PACKETRECIEVER:
		lcd20x4_i2c_puts(1, 0,"Error when receiving package");
		break;
	case FINGERPRINT_IMAGEMESS:
		lcd20x4_i2c_puts(1, 0,"Fail to generate character file due to the over-disorderly fingerprint image");
		break;
	case FINGERPRINT_FEATUREFAIL:
		lcd20x4_i2c_puts(1, 0,"Fail to generate character file due to lackness of character point or over-smallness of fingerprint image");
		break;
	case FINGERPRINT_INVALIDIMAGE:
		lcd20x4_i2c_puts(1, 0,"Fail to generate the image for the lackness of valid primary image");
		break;
	default:
		;
	}
		if (ack != FINGERPRINT_OK)
			return 0;
		//
		ack = fingerFastSearch();
		switch (ack)
		{
		//checks how the procedure went. FINGERPRINT_OK means good
		case FINGERPRINT_OK:
			sprintf(num, "%d", (fingerID[1] | (fingerID[0] << 8)));
			sprintf(mydata,"%s ID #%s", message,num);
			//lcd20x4_i2c_2ndLine();
			//lcd20x4_i2c_printf(mydata);
			lcd20x4_i2c_puts(1, 0,mydata);
			printf("%s",mydata);
			return stringToInt(num);
		case FINGERPRINT_PACKETRECIEVER:
			lcd20x4_i2c_puts(1, 0,"Error when receiving package");
			break;
		case FINGERPRINT_NOTFOUND:
			lcd20x4_i2c_puts(1, 0,"No matching in the library");
			break;
		default:
			;
		}
		sleep(3);
		if (ack != FINGERPRINT_OK)
			return 0;
}