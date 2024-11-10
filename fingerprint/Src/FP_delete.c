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
	printf("ID to delete: %u\n",id_N);
	if (id_N == 0)
    {
        lcd16x2_i2c_clear();
        lcd16x2_i2c_print(0, 0, "No ID entered");
        return FAILED;
    }
	uint8_t ack;
	ack = deleteTemplate(id_N);
	switch (ack)
	{
	case FINGERPRINT_OK:
		displayMessage( __func__,"Delete success");
		return SUCCESS;
	case FINGERPRINT_PACKETRECIEVER:
		displayMessage( __func__,"Error receive package");
		return FAILED;
	case FINGERPRINT_DELETEFAIL:
		displayMessage( __func__,"Delete failed");
		return FAILED;
	default:
		return FAILED;
	}
}