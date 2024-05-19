#include "../Inc/FP_delete.h"
#include <stdint.h>

void deleteModel(uint16_t id_N)
{
	lcd20x4_i2c_clear();
	uint8_t ack;
	ack = deleteTemplate(id_N);
	switch (ack)
	{
	case FINGERPRINT_OK:
		lcd20x4_i2c_puts(1, 0, "Delete success");
		break;
	case FINGERPRINT_PACKETRECIEVER:
		lcd20x4_i2c_puts(1, 0, "Error when receiving package");
		break;
	case FINGERPRINT_DELETEFAIL:
		lcd20x4_i2c_puts(1, 0, "Faile to delete templates");
		break;
	default:;
	}
}