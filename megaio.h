
#ifndef MEGAIO_H_
#define MEGAIO_H_



#define RETRY_TIMES	10


#define RELAY_MEM_ADD					(u8)0x00
#define RELAY_ON_MEM_ADD				(u8)0x01
#define RELAY_OFF_MEM_ADD				(u8)0x02
#define OPTO_IN_MEM_ADD					(u8)0x03
#define OC_OUT_VAL_MEM_ADD				(u8)0x04
#define OC_OUT_SET_MEM_ADD				(u8)0x05
#define OC_OUT_CLR_MEM_ADD				(u8)0x06

#define ADC_VAL_MEM_ADD					(u8)0x07
#define DAC_VAL_H_MEM_ADD				(u8)0x17
#define DAC_VAL_L_MEM_ADD				(u8)0x18
#define GPIO_VAL_MEM_ADD				(u8)0x19
#define GPIO_SET_MEM_ADD				(u8)0x1a
#define GPIO_CLR_MEM_ADD				(u8)0x1b
#define GPIO_DIR_MEM_ADD				(u8)0x1c
#define OPTO_IT_RISING_MEM_ADD			(u8)0x1d // 1B
#define OPTO_IT_FALLING_MEM_ADD			(u8)0x1e // 1B
#define GPIO_EXT_IT_RISING_MEM_ADD		(u8)0x1f // 1B
#define GPIO_EXT_IT_FALLING_MEM_ADD		(u8)0x20 // 1B
#define OPTO_IT_FLAGS_MEM_ADD			(u8)0x21 // 1B
#define GPIO_IT_FLAGS_MEM_ADD			(u8)0x22 // 1B

#define REVISION_HW_MAJOR_MEM_ADD		(u8)0x3c
#define REVISION_HW_MINOR_MEM_ADD		(u8)0x3d
#define REVISION_MAJOR_MEM_ADD			(u8)0x3e
#define REVISION_MINOR_MEM_ADD			(u8)0x3f

#define GPIO_PIN_NUMBER					(u8)6


#define ERROR	-1
#define FAIL	0
#define OK		1

#define CHANNEL_NR_MIN		1
#define RELAY_CH_NR_MAX		8
#define ADC_CH_NR_MAX		8
#define OPTO_CH_NR_MAX		8
#define OC_CH_NR_MAX		4
#define GPIO_CH_NR_MAX		6

#define ANALOG_VAL_MIN		0
#define ANALOG_VAL_MAX		4095
#define ANALOG_ERR_THRESHOLD	100

#define	COUNT_KEY	0
#define YES		1
#define NO		2

#define MEGAIO_HW_I2C_BASE_ADD	0x31


typedef uint8_t u8;
typedef uint16_t u16;

typedef enum
{
	OFF = 0,
	ON
} OutStateEnumType;

enum
{
	INT_DISABLE = 0,
	INT_RISING,
	INT_FALLING,
	INT_BOTH
};
	

void startThread(void);
int checkThreadResult(void);
int readReg16(int dev, int add);
int readReg24(int dev, int add);
int writeReg16(int dev, int add, int val);
int writeReg24(int dev, int add, int val);
void printbits(int v);
int writeReg8(int dev, int add, int val);
int readReg8(int dev, int add);
int doBoardInit(int hwAdd);
int getLedVal(int chip);
int setLedVal(int chip, int val);
void busyWait(int ms);

#endif //MEGAIO_H_
