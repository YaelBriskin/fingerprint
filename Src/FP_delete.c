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
		syslog_log(LOG_ERR, __func__, "OK", "Delete success");
		break;
	case FINGERPRINT_PACKETRECIEVER:
		syslog_log(LOG_ERR, __func__, "stderr", "Error when receiving package");
		break;
	case FINGERPRINT_DELETEFAIL:
		syslog_log(LOG_ERR, __func__, "stderr", "Faile to delete templates");
		break;
	default:;
	}
}