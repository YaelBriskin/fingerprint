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
int deleteModel(uint16_t id_N)
{
	lcd20x4_i2c_clear();
	uint8_t ack;
	ack = deleteTemplate(id_N);
	switch (ack)
	{
	case FINGERPRINT_OK:
		LOG_MESSAGE(LOG_ERR, __func__, "OK", "Delete success",NULL);
		return SUCCESS;
	case FINGERPRINT_PACKETRECIEVER:
		LOG_MESSAGE(LOG_ERR, __func__, "stderr", "Error when receiving package",NULL);
		return FAILED;
	case FINGERPRINT_DELETEFAIL:
		LOG_MESSAGE(LOG_ERR, __func__, "stderr", "Faile to delete templates",NULL);
		return FAILED;
	default:
		return FAILED;
	}
}