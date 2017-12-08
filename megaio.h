
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


#define VERSION_BASE	(int)2
#define VERSION_MAJOR	(int)1
#define VERSION_MINOR	(int)2

typedef uint8_t u8;
typedef uint16_t u16;


int readReg16(int dev, int add);
int readReg24(int dev, int add);
int writeReg16(int dev, int add, int val);
int writeReg24(int dev, int add, int val);
void printbits(int v);

#endif //MEGAIO_H_
