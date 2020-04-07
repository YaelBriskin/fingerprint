#include "packet.h"
/*
protocol[][][]...
	Header   |  Adder   |  Package     |  Package  |  Package content             |  Checksum
						   identifier  |  length   |  (instuction/data/Parameter） |

	2 bytes  |  4 bytes |  1 byte      |  2 bytes  |     -----------------        |  2 bytes


 */
uint8_t pData[SIZE]={0};
uint8_t RX_UART2[SIZE_RX2];
uint8_t getTempCount[SIZE_RX2] = "getT\n";
extern int uart_fd;

//Gets the command packet
#define GET_CMD_PACKET(...) \
	uint8_t Data[] = {__VA_ARGS__}; \
	fingerprintPacket packet;\
	uint8_t ack=0;\
	int length=0;\
	int chkSum=0;\
	packet.start_code=0xEF01;\
	packet.address[0] = 0xFF;\
	packet.address[1] = 0xFF;\
	packet.address[2] = 0xFF;\
	packet.address[3] = 0xFF;\
	packet.type = FINGERPRINT_COMMANDPACKET;\
	length= sizeof(Data);\
	if(sizeof(Data)<SIZE)\
	{\
		memcpy(packet.data, Data ,length);\
		for(int i=length;i<=SIZE;packet.data[i++]=0);\
	}\
	else\
	{\
		length=SIZE;\
		memcpy(packet.data, Data, SIZE);\
	}\
	packet.length=length+2;\
	chkSum=packet.type+packet.length;\
	for(int i=0;i<packet.length-2;i++)\
		chkSum+=packet.data[i];\
	packet.Checksum=chkSum;\
	ack=GetFromUart(&packet);\
	if(ack!=FINGERPRINT_OK)return ack;\

//Sends the command packet
#define SEND_CMD_PACKET(...) GET_CMD_PACKET(__VA_ARGS__);  return packet.data[0];\

//Confirm that communicate is connect between  module  and  upper monitor
uint8_t communicate_link(void)
{
	SEND_CMD_PACKET(FINGERPRINT_HANDSHAKE, FINGERPRINT_CONTROLCODE);
}
/*!
    @brief   Ask the sensor to take an image of the finger pressed on surface
    @returns <code>FINGERPRINT_OK</code> on success
    @returns <code>FINGERPRINT_NOFINGER</code> if no finger detected
    @returns <code>FINGERPRINT_PACKETRECIEVEERR</code> on communication error
    @returns <code>FINGERPRINT_IMAGEFAIL</code> on imaging error
*/
uint8_t getImage(void)
{
	SEND_CMD_PACKET(FINGERPRINT_GETIMAGE);
}
/*!
    @brief   Ask the sensor to convert image to feature template
    @param slot Location to place feature template (put one in 1 and another in
   2 for verification to create model)
    @returns <code>FINGERPRINT_OK</code> on success
    @returns <code>FINGERPRINT_IMAGEMESS</code> if image is too messy
    @returns <code>FINGERPRINT_PACKETRECIEVEERR</code> on communication error
    @returns <code>FINGERPRINT_FEATUREFAIL</code> on failure to identify
   fingerprint features
    @returns <code>FINGERPRINT_INVALIDIMAGE</code> on failure to identify
   fingerprint features
*/
uint8_t image2Tz(uint8_t slot)
{
	SEND_CMD_PACKET(FINGERPRINT_IMAGE2TZ, slot);
}
/*!
    @brief   Ask the sensor to take two print feature template and create a
   model
    @returns <code>FINGERPRINT_OK</code> on success
    @returns <code>FINGERPRINT_PACKETRECIEVEERR</code> on communication error
    @returns <code>FINGERPRINT_ENROLLMISMATCH</code> on mismatch of fingerprints
*/
uint8_t createModel(void)
{
	SEND_CMD_PACKET(FINGERPRINT_REGMODEL)
}
/*!
    @brief   Ask the sensor to store the calculated model for later matching
    @param   location The model location #
    @returns <code>FINGERPRINT_OK</code> on success
    @returns <code>FINGERPRINT_BADLOCATION</code> if the location is invalid
    @returns <code>FINGERPRINT_FLASHERR</code> if the model couldn't be written
   to flash memory
    @returns <code>FINGERPRINT_PACKETRECIEVEERR</code> on communication error
*/
uint8_t storeModel(uint16_t location) {
	SEND_CMD_PACKET(FINGERPRINT_STORE, 0x01, (uint8_t)(location >> 8),(uint8_t)(location & 0xFF));
}
/*!
    @brief   Ask the sensor to load a fingerprint model from flash into buffer 1
    @param   location The model location #
    @returns <code>FINGERPRINT_OK</code> on success
    @returns <code>FINGERPRINT_BADLOCATION</code> if the location is invalid
    @returns <code>FINGERPRINT_PACKETRECIEVEERR</code> on communication error
*/
uint8_t loadModel(uint16_t location)
{
	SEND_CMD_PACKET(FINGERPRINT_LOAD, 0x01, (uint8_t)(location >> 8),(uint8_t)(location & 0xFF));
}
/*!
    @brief   Ask the sensor to transfer 256-byte fingerprint template from the
   buffer to the UART
    @returns <code>FINGERPRINT_OK</code> on success
    @returns <code>FINGERPRINT_PACKETRECIEVEERR</code> on communication error
*/
uint8_t getModel(void)
{
	SEND_CMD_PACKET(FINGERPRINT_UPLOAD, 0x01);
}
/*!
    @brief   Ask the sensor to delete a model in memory
    @param   location The model location #
    @returns <code>FINGERPRINT_OK</code> on success
    @returns <code>FINGERPRINT_BADLOCATION</code> if the location is invalid
    @returns <code>FINGERPRINT_FLASHERR</code> if the model couldn't be written
   to flash memory
    @returns <code>FINGERPRINT_PACKETRECIEVEERR</code> on communication error
*/
uint8_t deleteTemplate(uint16_t location)
{
	SEND_CMD_PACKET(FINGERPRINT_DELETE, (uint8_t)(location >> 8),(uint8_t)(location & 0xFF), 0x00, 0x01);
}
/*!
    @brief   Ask the sensor to delete ALL models in memory
    @returns <code>FINGERPRINT_OK</code> on success
    @returns <code>FINGERPRINT_BADLOCATION</code> if the location is invalid
    @returns <code>FINGERPRINT_FLASHERR</code> if the model couldn't be written
   to flash memory
    @returns <code>FINGERPRINT_PACKETRECIEVEERR</code> on communication error
*/
uint8_t emptyDatabase(void)
{
	SEND_CMD_PACKET(FINGERPRINT_EMPTY);
}
/*!
    @brief   Ask the sensor to search the current slot 1 fingerprint features to
   match saved templates. The matching location is stored in <b>fingerID</b> and
   the matching confidence in <b>confidence</b>
    @returns <code>FINGERPRINT_OK</code> on fingerprint match success
    @returns <code>FINGERPRINT_NOTFOUND</code> no match made
    @returns <code>FINGERPRINT_PACKETRECIEVEERR</code> on communication error
*/
uint8_t fingerFastSearch(void)
{
	GET_CMD_PACKET(FINGERPRINT_SEARCH, 0x01, 0x00, 0x00, 0x00, 0xA3)

	confidence = 0xFFFF;
	fingerID[0] = packet.data[1];
	fingerID[1] = packet.data[2];
	confidence = packet.data[3];
	confidence <<= 8;
	confidence |= packet.data[4];

	return packet.data[0];
}
/*!
    @brief   Ask the sensor for the number of templates stored in memory. The
   number is stored in <b>templateCount</b> on success.
    @returns <code>FINGERPRINT_OK</code> on success
    @returns <code>FINGERPRINT_PACKETRECIEVEERR</code> on communication error
*/
uint8_t getTemplateCount(void) {
	GET_CMD_PACKET(FINGERPRINT_TEMPLATECOUNT);

	templateCount = packet.data[1];
	templateCount <<= 8;
	templateCount |= packet.data[2];

	return packet.data[0];
}
//Get packet from FPM
uint8_t GetFromUart(fingerprintPacket *packet)
{
	int count_received_data=0;
	uint8_t idx = 0;
	uint16_t length = 0;
	int chkSum;
	SendToUart(packet);
	if(UART_read(uart_fd, (uint16_t) pData,MIN_SIZE_PACKET)==0)
	{
		return FINGERPRINT_TIMEOUT;
	}
	length = (uint16_t) pData[7] << 8;
	length = (uint16_t) pData[8];
	if(UART_read(uart_fd, (uint16_t) pData,length)==0)
	{
		return FINGERPRINT_TIMEOUT;
	}
	if (pData[idx] == (FINGERPRINT_STARTCODE >> 8))
	{
		packet->start_code = (uint16_t) pData[idx++] << 8;
		packet->start_code |= pData[idx++];
	}
	if (packet->start_code != FINGERPRINT_STARTCODE)
		return FINGERPRINT_BADPACKET;

	packet->address[0] = pData[idx++];
	packet->address[1] = pData[idx++];
	packet->address[2] = pData[idx++];
	packet->address[3] = pData[idx++];

	if(pData[idx]!=FINGERPRINT_ACKPACKET)
		return FINGERPRINT_PACKETRECIEVER;
	packet->type = pData[idx++];
	packet->length = length;
	idx += length;
	memset(packet->data, 0, SIZE);
	memcpy(packet->data, pData + MIN_SIZE_PACKET, length - 2);

	packet->Checksum = (uint16_t) pData[idx++] << 8;
	packet->Checksum |= (uint16_t) pData[idx];
	chkSum = packet->length + packet->type;
	for (int i = 0; i < packet->length - 2; i++)
		chkSum += packet->data[i];
	if (chkSum != packet->Checksum )
		return FINGERPRINT_BADPACKET;
	return packet->data[0];
}

void GetFromServer()
{
	uint8_t ch = 0;
	uint16_t byte = 0;
	int dec = 0;
	ch = RX_UART2[0];

	switch(ch){
	case 'e' :
		//to enroll a fingerprint and save ID in Flash memory
		if(isdigit(RX_UART2[1]))
		{
			dec = atoi((char*) &RX_UART2[1]);
			byte = (uint16_t) dec;
			enrolling(byte);
		}
		// to delete all the templates in the Flash library
		if (strcmp((char*) emptyDB, (char*) RX_UART2) == 0)
		{
			emptyDatabase();
		}
		break;
	case 'd':
		//delete a specific template in the Flash library
		if(isdigit(RX_UART2[1]))
		{
			dec = atoi((char*) &RX_UART2[1]);
			byte = (uint16_t) dec;
			deleteModel(byte);
		}
		break;
	default:
		;
	}
	//read the current valid template number of the Module
	if (strcmp((char*) getTempCount, (char*) RX_UART2) == 0)
	{
		GetTemplateCount();
	}
	memset(RX_UART2, 0, SIZE_Eth);
}
