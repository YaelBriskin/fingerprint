#include "../Inc/FP_delete.h"
#include <stdint.h>

/**
 * @brief Deletes a fingerprint template with the specified ID.
 *
 * This function deletes a fingerprint template stored in the fingerprint module
 * with the given ID.
 *
 * @param id_N The ID of the fingerprint template to be deleted.
 */
void deleteModel(uint16_t id_N)
{
	lcd20x4_i2c_clear();
	uint8_t ack;
	ack = deleteTemplate(id_N);
	switch (ack)
	{
	case FINGERPRINT_OK:
		printf("Delete success\r\n");
		break;
	case FINGERPRINT_PACKETRECIEVER:
		printf("Error when receiving package\r\n");
		break;
	case FINGERPRINT_DELETEFAIL:
		printf("Faile to delete templates\r\n");
		break;
	default:;
	}
}