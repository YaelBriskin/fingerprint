#include "../Inc/FP_find_finger.h"

char mydata[23] = {0};
extern uint8_t fingerID[2];


// Function to display the route message on the LCD
/**
 * @brief Displays a message on the LCD based on the input message.
 *
 * This function checks the input message and displays "sign in" or "sign out"
 * on the LCD based on whether the message is "Hello" or not.
 *
 * @param message The input message to be displayed.
 */
void display_route(const char *message) 
{
    if (strcmp(message, "Hello") == 0) 
	{
		lcd20x4_i2c_puts(3, 0, "sign in");
    } 
	else 
	{
		lcd20x4_i2c_puts(3, 0, "sign out");
    }
}
// Function to convert a string to an integer
/**
 * @brief Converts a string to an integer.
 *
 * This function takes a string representing a number and converts it to an integer.
 * If the string contains invalid characters, it logs an error and returns 0.
 *
 * @param str The input string to be converted.
 * @return int The converted integer, or 0 if the string contains invalid characters.
 */
int stringToInt(const char *str)
{
	int result = 0;
	int i = 0;
	// Convert each digit in the string to a number
	while (str[i] != '\0')
	{
		if (str[i] >= '0' && str[i] <= '9')
		{
			result = result * 10 + (str[i] - '0');
		}
		else
		{
			// Handle invalid characters
			LOG_MESSAGE(LOG_ERR, __func__, "format", "Invalid character in string: %c", str[i]);
			return 0;
		}
		i++;
	}
	return result;
}
// Function to find a fingerprint match
/**
 * @brief Tries to find a fingerprint match and returns the corresponding ID.
 *
 * This function attempts to find a fingerprint match by scanning for a finger within a specific time frame.
 * If a finger is detected, it generates a character file from the fingerprint image and stores it in the
 * character buffer. It then performs a fast search to identify the fingerprint and returns the corresponding ID.
 *
 * @param message The message to display during the fingerprint scanning process.
 * @return int The ID of the fingerprint match if found, or -1 if no match is found or an error occurs.
 */
int findFinger(const char *message)
{
	lcd20x4_i2c_clear();
	display_route(message);
	struct timespec start_time;
	const int max_execution_time = 10;
	struct timespec current_time;
	char num[2] = {0};
	int ack = -1;
	int previous_ack = -1;
	lcd20x4_i2c_puts(1, 0, "Waiting finger to enroll");
	sleep(SLEEP_DURATION);
	clock_gettime(CLOCK_MONOTONIC, &start_time);

	// Main loop for scanning the fingerprint
	while (ack != FINGERPRINT_OK)
	{
		clock_gettime(CLOCK_MONOTONIC, &current_time);
		long elapsed_time = (current_time.tv_sec - start_time.tv_sec) + (current_time.tv_nsec - start_time.tv_nsec) / 1000000000;
		if (elapsed_time >= max_execution_time)
		{
			lcd20x4_i2c_puts(1, 0, "Timeout: One minute has passed.");
			LOG_MESSAGE(LOG_ERR, __func__, "stderr", "Timeout: One minute has passed", NULL);
			sleep(SLEEP_LCD);
			lcd20x4_i2c_clear();
			return FAILED;
		}
		// detecting finger and store the detected finger image in ImageBuffer while returning successfull confirmation code;
		// If there is no finger, returned confirmation code would be cant detect finger.
		ack = (int)getImage();
		if (ack != previous_ack)
		{
			lcd20x4_i2c_clear();
			display_route(message);
			// Handle different response codes
			switch (ack)
			{
				// checks how the procedure went. FINGERPRINT_OK means good
			case FINGERPRINT_OK:
				LOG_MESSAGE(LOG_INFO, __func__, "OK", "Finger collection success", NULL);
				lcd20x4_i2c_puts(1, 0, "Finger collection success");
				break;
			case FINGERPRINT_PACKETRECIEVER:
				LOG_MESSAGE(LOG_ERR, __func__, "stderr", "Error when receiving package", NULL);
				lcd20x4_i2c_puts(1, 0, "Error when receiving package");
				break;
			case FINGERPRINT_NOFINGER:
				LOG_MESSAGE(LOG_ERR, __func__, "stderr", "Can't detect finger", NULL);
				lcd20x4_i2c_puts(1, 0, "Can't detect finger");
				break;
			case FINGERPRINT_IMAGEFAIL:
				LOG_MESSAGE(LOG_ERR, __func__, "stderr", "Fail to collect finger", NULL);
				lcd20x4_i2c_puts(1, 0, "Fail to collect finger");
				break;
			default:;
			}
			previous_ack = ack; // Update previous_ack for next iteration
		}
		usleep(DELAY);
	}
	// to generate character file from the original finger image in ImageBuffer and store the file in CharBuffer1 or CharBuffer2.
	sleep(SLEEP_LCD);
	lcd20x4_i2c_clear();
	display_route(message);
	// Convert image to template
	ack = image2Tz(1);
	// Handle different response codes
	switch (ack)
	{
	case FINGERPRINT_OK:
		LOG_MESSAGE(LOG_INFO, __func__, "OK", "Generate character file complete", NULL);
		break;
	case FINGERPRINT_PACKETRECIEVER:
		LOG_MESSAGE(LOG_ERR, __func__, "stderr", "Error when receiving package", NULL);
		break;
	case FINGERPRINT_IMAGEMESS:
		LOG_MESSAGE(LOG_ERR, __func__, "stderr", "Fail to generate character file due to the over-disorderly fingerprint image", NULL);
		break;
	case FINGERPRINT_FEATUREFAIL:
		LOG_MESSAGE(LOG_ERR, __func__, "stderr", "Fail to generate character file due to lackness of character point or over-smallness of fingerprint image", NULL);
		break;
	case FINGERPRINT_INVALIDIMAGE:
		LOG_MESSAGE(LOG_ERR, __func__, "stderr", "Fail to generate the image for the lackness of valid primary image", NULL);
		break;
	default:;
	}

	if (ack != FINGERPRINT_OK)
		return FAILED;
	// Search for fingerprint in database
	ack = fingerFastSearch();
	// Handle different response codes
	switch (ack)
	{
	// checks how the procedure went. FINGERPRINT_OK means good
	case FINGERPRINT_OK:
		sprintf(num, "%d", (fingerID[1] | (fingerID[0] << 8)));
		sprintf(mydata, "%s ID #%s", message, num);
		lcd20x4_i2c_puts(1, 0, mydata);
		LOG_MESSAGE(LOG_INFO, __func__, "OK", " ", mydata);
		return stringToInt(num); // Return fingerprint ID
	case FINGERPRINT_PACKETRECIEVER:
		lcd20x4_i2c_puts(1, 0, "Error when receiving package");
		LOG_MESSAGE(LOG_ERR, __func__, "stderr", "Error when receiving package", NULL);
		break;
	case FINGERPRINT_NOTFOUND:
		return ERROR;
	default:;
	}
	sleep(SLEEP_LCD);
	lcd20x4_i2c_clear();
	if (ack != FINGERPRINT_OK)
		return FAILED;
}