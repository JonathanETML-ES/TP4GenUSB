#ifndef Mc32gestI2cSeeprom_H    /* Guard against multiple inclusion */
#define Mc32gestI2cSeeprom_H
/*--------------------------------------------------------*/
// Mc32gestI2cSeeprom.h
/*--------------------------------------------------------*/
//	Description :	Librairie d'accès à la mémoire EEPROM 
//
//	Auteur 		: 	J. Shifteh
//
//	Version		:	V1.1
//	Compilateur	:	XC32 V1.40 + Harmony 1.06
//
/*--------------------------------------------------------*/
#include "Mc32_I2cUtilCCS.h"
#include "DefMenuGen.h"

#include <Mc32Delays.h>
#include <stdint.h>
#include <stdbool.h>
/*--------------------------------------------------------*/
// Prototypes des fonctions 
/*--------------------------------------------------------*/
void I2C_InitMCP79411(void);
void I2C_Write_EEPROM(void *SrcData, uint16_t NbBytes);
void I2C_Read_EEPROM(void *DstData, uint16_t NbBytes);


#endif