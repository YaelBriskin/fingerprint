#include "find_finger.h"


char mydata[23] = { 0 };
extern uint8_t fingerID[2];

void findFinger()
{
	char num[2] = { 0 };
	int ack = -1;
	lcd16x2_i2c_clear();
	lcd16x2_i2c_setCursor(0, 0);
	lcd16x2_i2c_printf("Waiting finger to");
	lcd16x2_i2c_setCursor(1,0);
	lcd16x2_i2c_printf("enroll");
	while (ack != FINGERPRINT_OK)
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
			return;
		//
		ack = fingerFastSearch();
		switch (ack)
		{
		//checks how the procedure went. FINGERPRINT_OK means good
		case FINGERPRINT_OK:
			sprintf(num, "%d", (fingerID[1] | (fingerID[0] << 8)));
			lcd16x2_i2c_clear();
			sprintf(mydata,"Found ID #%s",num);
			lcd16x2_i2c_printf(mydata);
			break;
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
}