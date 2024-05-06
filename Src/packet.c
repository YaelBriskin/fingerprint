#include "../Inc/packet.h"
/*
protocol[][][]...
	Header   |  Adder   |  Package     |  Package  |  Package content             |  Checksum
						   identifier  |  length   |  (instuction/data/Parameter） |

	2 bytes  |  4 bytes |  1 byte      |  2 bytes  |     -----------------        |  2 bytes


 */
uint8_t getTempCount[7] = "getT\n";
ReadSysPara parameters;
extern int uart2_fd;

//Gets the command packet
#define GET_CMD_PACKET(...) \
	uint8_t Data[] = {__VA_ARGS__}; \
	fingerprintPacket packet;\
	uint8_t ack=0;\
	int length=0;\
	packet.start_code=FINGERPRINT_STARTCODE;\
	packet.address[0] = 0xFF;\
	packet.address[1] = 0xFF;\
	packet.address[2] = 0xFF;\
	packet.address[3] = 0xFF;\
	packet.type = FINGERPRINT_COMMANDPACKET;\
	length= sizeof(Data);\
	if(sizeof(Data)<SIZE)\
	{\
		memcpy(packet.data, Data ,length);\
		memset(packet.data + length, 0, SIZE - length);\
	}\
	else\
	{\
		memcpy(packet.data, Data, SIZE);\
	}\
	packet.length=length+2;\
	ack=GetFromUart(&packet);\
	if(ack!=FINGERPRINT_OK)return ack;\

//Sends the command packet 
#define SEND_CMD_PACKET(...)           \
  GET_CMD_PACKET(__VA_ARGS__);         \
  return packet.data[0];
//Confirm that communicate is connect between  module  and  upper monitor
uint8_t communicate_link(void)
{
	SEND_CMD_PACKET(FINGERPRINT_HANDSHAKE, FINGERPRINT_CONTROLCODE);
}

uint8_t getParameters(void) 
{
  GET_CMD_PACKET(FINGERPRINT_READSYSPARAM);

  parameters.status_reg = ((uint16_t)packet.data[1] << 8) | packet.data[2];
  parameters.system_id = ((uint16_t)packet.data[3] << 8) | packet.data[4];
  parameters.capacity = ((uint16_t)packet.data[5] << 8) | packet.data[6];
  parameters.security_level = ((uint16_t)packet.data[7] << 8) | packet.data[8];
  parameters.device_addr = ((uint32_t)packet.data[9] << 24) |
                ((uint32_t)packet.data[10] << 16) |
                ((uint32_t)packet.data[11] << 8) | (uint32_t)packet.data[12];
  parameters.packet_len = ((uint16_t)packet.data[13] << 8) | packet.data[14];
  if (parameters.packet_len == 0) {
    parameters.packet_len = 32;
  } else if (parameters.packet_len == 1) {
    parameters.packet_len = 64;
  } else if (parameters.packet_len == 2) {
    parameters.packet_len = 128;
  } else if (parameters.packet_len == 3) {
    parameters.packet_len = 256;
  }
  parameters.baud_rate = (((uint16_t)packet.data[15] << 8) | packet.data[16]) * 9600;

  return packet.data[0];
}
uint8_t writeRegister(uint8_t regAdd, uint8_t value) 
{
  	SEND_CMD_PACKET(FINGERPRINT_WRITE_REG, regAdd, value);
}
uint8_t setSecurityLevel(uint8_t level) 
{
  return (writeRegister(FINGERPRINT_SECURITY_REG_ADDR, level));
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
uint8_t storeModel(uint16_t location) 
{
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
void SendToUart(fingerprintPacket *packet)
{
	uint16_t Size= MIN_SIZE_PACKET + (packet->length);
	uint16_t i=0;
	uint8_t packetData[Size];

	packetData[i++]=(uint8_t)((packet->start_code)>>8);
	packetData[i++]= (uint8_t)((packet->start_code) & 0xFF);

	packetData[i++]=packet->address[0];
	packetData[i++]=packet->address[1];
	packetData[i++]=packet->address[2];
	packetData[i++]=packet->address[3];

	packetData[i++]=packet->type;

	packetData[i++]= (uint8_t)((packet->length)>>8);
	packetData[i++]= (uint8_t)((packet->length) & 0xFF);
	uint16_t Sum = ((packet->length) >> 8) + ((packet->length) & 0xFF) + packet->type;

	for (int j = 0; j <packet->length-2; j++)
	{
		packetData[i++]=(packet->data[j]);
		Sum += packet->data[j];
	}
	packetData[i++]=(uint8_t)(Sum>>8);
	packetData[i]=(uint8_t)(Sum & 0xFF);
	UART_write(uart2_fd,packetData, Size);
}

//Get packet from FPM
uint8_t GetFromUart(fingerprintPacket *packet)
{
	uint8_t pData[SIZE]={0};
	int count_received_data=0;
	uint8_t idx = 0;
	uint16_t length = 0;
	int chkSum;
	SendToUart(packet);
	usleep(100000);
	// Check the first data read
	if(UART_read(uart2_fd,pData,MIN_SIZE_PACKET)==0)
		return FINGERPRINT_TIMEOUT;
	// shift a byte 8 bits to the left and then combine it with the next byte
	length = ((uint16_t)pData[7] << 8) | pData[8];
	if (length > SIZE - MIN_SIZE_PACKET)
    	// Packet length exceeds buffer size
    	return -1;

	// Check the second data read
	if(UART_read(uart2_fd,pData+MIN_SIZE_PACKET,length)==0)
		return FINGERPRINT_TIMEOUT;

	if ((pData[idx] != (FINGERPRINT_STARTCODE >> 8)) || ((pData[idx+1] != (FINGERPRINT_STARTCODE & 0xFF)))) 
    	return FINGERPRINT_BADPACKET;

	packet->start_code = (uint16_t) pData[idx++] << 8;
	packet->start_code |= pData[idx++];

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

	// shift the byte 8 bits to the left and then concatenate it with the low byte
	packet->Checksum = ((uint16_t)pData[idx++] << 8) | pData[idx];

	chkSum = packet->length + packet->type;
	for (int i = 0; i < packet->length - 2; i++)
		chkSum += packet->data[i];
	if (chkSum != packet->Checksum )
		return FINGERPRINT_BADPACKET;
	printf("%s", packet->data);
	return packet->data[0];
}
void printParameters() 
{
     printf("Device parameters:\n");
     printf("Status register: 0x%04X\n", parameters.status_reg);
     printf("System ID code: 0x%04X\n", parameters.system_id);
     printf("Finger library size: %d\n", parameters.capacity);
     printf("Security level: %d\n", parameters.security_level);
     printf("Device address: 0x%08X\n", parameters.device_addr);
     printf("Data packet size: %d\n", parameters.packet_len);
     printf("Baud rate: %d\n", parameters.baud_rate);
}