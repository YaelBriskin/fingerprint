#ifndef PACKET_H
#define PACKET_H

#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include "UART.h"
#include "FP_delete.h"
#include "FP_enrolling.h"

#define MAX_RETRIES 10

// confirmation codes
#define FINGERPRINT_OK 0x00                 // выполнение команды завершено успешно
#define FINGERPRINT_PACKETRECIEVER 0x01     // ошибка при получении пакета данных
#define FINGERPRINT_NOFINGER 0x02           // нет пальца на датчике
#define FINGERPRINT_IMAGEFAIL 0x03          // не удается зарегистрировать палец
#define FINGERPRINT_IMAGEMESS 0x06          // не удалось сгенерировать файл символов из-за чрезмерно беспорядочного изображения отпечатка пальца
#define FINGERPRINT_FEATUREFAIL 0x07        // не удается сгенерировать файл символов из-за отсутствия точки символа или слишком маленького размера отпечатка пальца
#define FINGERPRINT_NOMATCH 0x08            // палец не совпадает
#define FINGERPRINT_NOTFOUND 0x09           // не удается найти соответствующий палец
#define FINGERPRINT_ENROLLMISMATCH 0x0A     // не удается объединить файлы символов
#define FINGERPRINT_BADLOCATION 0x0B        // адресация PageID находится за пределами библиотеки finger
#define FINGERPRINT_DBRANGEFAIL 0x0C        // ошибка при чтении шаблона из библиотеки или неверного шаблона
#define FINGERPRINT_UPLOADFEATUREFAIL 0x0D  // ошибка при загрузке шаблона
#define FINGERPRINT_PACKETRESPONSEFAIL 0x0E // модуль не может получить следующие пакеты данных
#define FINGERPRINT_UPLOADFAIL 0x0F         // ошибка при загрузке изображения
#define FINGERPRINT_DELETEFAIL 0x10         // не удалось удалить шаблон
#define FINGERPRINT_DBCLEARFAIL 0x11        // не удается очистить библиотеку пальцев
#define FINGERPRINT_PASSFAIL 0x13           //
#define FINGERPRINT_INVALIDIMAGE 0x15       // не удалось сгенерировать изображение из-за отсутствия действительного основного изображения
#define FINGERPRINT_FLASHERR 0x18           // ошибка при записи флеш
#define FINGERPRINT_INVALIDREG 0x1A         // неверный номер регистра
#define FINGERPRINT_ADDRCODE 0x20           //
#define FINGERPRINT_PASSVERIFY 0x21         //

// signature and packet ids
#define FINGERPRINT_STARTCODE 0xEF01

#define FINGERPRINT_CONTROLCODE 0x0
#define FINGERPRINT_COMMANDPACKET 0x1
#define FINGERPRINT_DATAPACKET 0x2
#define FINGERPRINT_ACKPACKET 0x7
#define FINGERPRINT_ENDDATAPACKET 0x8

#define FINGERPRINT_TIMEOUT 0xFF
#define FINGERPRINT_BADPACKET 0xFE

// COMMANDS
#define FINGERPRINT_GETIMAGE 0x01       // обнаружение пальца и сохранение обнаруженного изображения в ImageBuffer
#define FINGERPRINT_IMAGE2TZ 0x02       // преобразовать изображение ImageBuffer в шаблон объекта в CharBuffer1\CharBuffer2
#define FINGERPRINT_MATCH 0x03          // сопостановлени-е шаблонов из CharBuffer1 и CharBuffer2 обеспечения соответствия результатов
#define FINGERPRINT_SEARCH 0x04         // поиск "пальца" во всей библиотеке соответствующий шаблону CharBuffer1\CharBuffer2
#define FINGERPRINT_REGMODEL 0x05       // объединяет инфор. из CharBuffer1 и CharBuffer2 и создает шаблон в CharBuffer1 и CharBuffer2
#define FINGERPRINT_STORE 0x06          // сохранить шаблон из Buffer1/Buffer2 в указанное место
#define FINGERPRINT_LOAD 0x07           // загрузить шаблон в указанное место(PadeId) библиотеки FLASH в буфер шаблонов CharBuffer1\CharBuffer2
#define FINGERPRINT_UPLOAD 0x08         // загружает файл симвоолов или шаблона CharBuffer1\CharBuffer2 в верхний комп.
#define FINGERPRINT_DOWNCHAR 0x09       //
#define FINGERPRINT_IMGUPLOAD 0x0A      //
#define FINGERPRINT_DELETE 0x0C         //
#define FINGERPRINT_EMPTY 0x0D          // удалить все шаблоны в библиотеке
#define FINGERPRINT_SETSYSPARAM 0x0E    // настройка параметров работы
#define FINGERPRINT_READSYSPARAM 0x0F   // считать регистр состояния модуля и основные параметры конфигурации системы
#define FINGERPRINT_SETPASSWORD 0x12    //
#define FINGERPRINT_VERIFYPASSWORD 0x13 //
#define FINGERPRINT_GETRANDOM 0x14      // дать команду модулю сгенерировать случайное число и вернуть его в верхний комп.
#define FINGERPRINT_HISPEEDSEARCH 0x1B  //
#define FINGERPRINT_TEMPLATECOUNT 0x1D  // прочитать текущий действующий номер шаблона модуля
#define FINGERPRINT_HANDSHAKE 0x17

#define DEFAULTTIMEOUT 1000 /// Время ожидания чтения UART в миллисекундах (= 1 секунда)

#define SIZE 64
#define MIN_SIZE_PACKET 9
#define SIZE_Eth 7
#define TIMEOUT 3000

/// Соответствующее местоположение, которое установлено fingerFastSearch ()
uint8_t fingerID[2];
/// Достоверность соответствия fingerFastSearch (), более высокие числа - больше уверенности
uint16_t confidence;
/// Количество хранимых шаблонов в датчике, установленное getTemplateCount ()
uint16_t templateCount;

///! Вспомогательный класс для создания пакетов UART
typedef struct
{
   uint16_t start_code; ///< "Wakeup" code for packet detection
   uint8_t address[4];  ///< 32-bit Fingerprint sensor address
   uint8_t type;        ///< Type of packet
   uint16_t length;     ///< Length of packet
   uint8_t data[SIZE];
   uint16_t Checksum; ///< Необработанный буфер для полезной нагрузки пакета////////changed!!!!!!!!!!!!!!!!!
} fingerprintPacket;

// ReadSysPara
typedef struct
{
   uint16_t status_reg;     // регистр состояния
   uint16_t system_id;      // код id системы
   uint16_t capacity;       // размер библиотеки пальца
   uint16_t security_level; // уровень безопасности
   uint32_t device_addr;    // адрес устройства
   uint16_t packet_len;     // размер пакета данных
   uint16_t baud_rate;      // настройки в бодах
} ReadSysPara;

// формат пакета данных
typedef enum
{
   FPM_STATE_READ_HEADER,
   FPM_STATE_READ_ADDRESS,
   FPM_STATE_READ_PID,
   FPM_STATE_READ_LENGTH,
   FPM_STATE_READ_CONTENTS,
   FPM_STATE_READ_CHECKSUM
} FINGERPRINT_STATE;
enum
{
   FPM_SETPARAM_BAUD_RATE = 4,  // управление скоростью передачи данных
   FPM_SETPARAM_SECURITY_LEVEL, //=5 //уровень безопасности
   FPM_SETPARAM_PACKET_LEN      //=6 //длина пакета данных
};
// управление скоростью передачи данных
enum
{
   FPM_BAUD_9600 = 1,
   FPM_BAUD_19200,
   FPM_BAUD_28800,
   FPM_BAUD_38400,
   FPM_BAUD_48000,
   FPM_BAUD_57600,
   FPM_BAUD_67200,
   FPM_BAUD_76800,
   FPM_BAUD_86400,
   FPM_BAUD_96000,
   FPM_BAUD_105600,
   FPM_BAUD_115200
};
uint8_t getImage(void);
uint8_t image2Tz(uint8_t slot);
void receive_data(void);
uint8_t createModel(void);
uint8_t emptyDatabase(void);
uint8_t storeModel(uint16_t id);
uint8_t loadModel(uint16_t id);
uint8_t getModel(void);
uint8_t deleteTemplate(uint16_t id);
uint8_t fingerFastSearch(void);
uint8_t getTemplateCount(void);
uint8_t getParameters(void);
void SendToUart(fingerprintPacket *packet);
uint8_t GetFromUart(fingerprintPacket *packet);
uint8_t communicate_link(void);
void GetFromServer(char *request);
#endif // PACKET_H
