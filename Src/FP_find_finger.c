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
	lcd16x2_i2c_clear();
	lcd16x2_i2c_setCursor(0, 0);
	lcd16x2_i2c_printf("Waiting finger to");
	lcd16x2_i2c_setCursor(1,0);
	lcd16x2_i2c_printf("enroll");

	clock_t start_time = clock();
	const clock_t max_execution_time = 60 * CLOCKS_PER_SEC;

	while (ack != FINGERPRINT_OK && (clock() - start_time) <= max_execution_time)
	{
		//detecting finger and store the detected finger image in ImageBuffer while returning successfull confirmation code; If there is no finger, returned confirmation code would be cant detect finger.
		ack = (int) getImage();
		switch (ack)
		{
			//checks how the procedure went. FINGERPRINT_OK means good
		case FINGERPRINT_OK:
			lcd16x2_i2c_clear();
			lcd16x2_i2c_setCursor(0, 0);
			lcd16x2_i2c_printf("Finger collection");
			lcd16x2_i2c_setCursor(1,0);
			lcd16x2_i2c_printf("success");
			break;
		case FINGERPRINT_PACKETRECIEVER:
			lcd16x2_i2c_clear();
			lcd16x2_i2c_setCursor(0, 0);
			lcd16x2_i2c_printf("Error when receiving");
			lcd16x2_i2c_setCursor(1,0);
			lcd16x2_i2c_printf("package");
			break;
		case FINGERPRINT_NOFINGER:
			lcd16x2_i2c_clear();
			lcd16x2_i2c_printf("Cant detect finger");
			break;
		case FINGERPRINT_IMAGEFAIL://
			lcd16x2_i2c_clear();
			lcd16x2_i2c_setCursor(0, 4);
			lcd16x2_i2c_printf("Fail to collect");
			lcd16x2_i2c_setCursor(1,0);
			lcd16x2_i2c_printf("finger");
			break;
		default:
			;
		}
	}
	//to generate character file from the original finger image in ImageBuffer and store the file in CharBuffer1 or CharBuffer2.
	ack = image2Tz(1);
	switch (ack)
	{
	//checks how the procedure went. FINGERPRINT_OK means good
	case FINGERPRINT_OK:
		lcd16x2_i2c_clear();
		lcd16x2_i2c_setCursor(0, 0);
		lcd16x2_i2c_printf("Generate character");
		lcd16x2_i2c_setCursor(1, 0);
		lcd16x2_i2c_printf("file complete");
		break;
	case FINGERPRINT_PACKETRECIEVER:
		lcd16x2_i2c_clear();
		lcd16x2_i2c_printf("Error when receiving package");
		break;
	case FINGERPRINT_IMAGEMESS:
		lcd16x2_i2c_clear();
		lcd16x2_i2c_printf("Fail to generate character file due to the over-disorderly fingerprint image");
		break;
	case FINGERPRINT_FEATUREFAIL:
		lcd16x2_i2c_clear();
		lcd16x2_i2c_printf("Fail to generate character file due to lackness of character point or over-smallness of fingerprint image");
		break;
	case FINGERPRINT_INVALIDIMAGE:
		lcd16x2_i2c_clear();
		lcd16x2_i2c_printf("Fail to generate the image for the lackness of valid primary image");
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
			lcd16x2_i2c_clear();
			sprintf(mydata,"%s ID #%s", message,num);
			lcd16x2_i2c_printf(mydata);
			return stringToInt(num);
		case FINGERPRINT_PACKETRECIEVER:
			lcd16x2_i2c_clear();
			lcd16x2_i2c_printf("Error when receiving package");
			break;
		case FINGERPRINT_NOTFOUND:
			lcd16x2_i2c_clear();
			lcd16x2_i2c_printf("No matching in the library");
			break;
		default:
			;
		}
		if (ack != FINGERPRINT_OK)
			return 0;
			
}