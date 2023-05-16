// Mc32gestI2cSeeprom.C
// fonction d'accès à la mémoire EEPROM 
#include "Mc32gestI2cSeeprom.h"

#define ControlByte_W 0b10101110
#define ControlByte_R 0b10101111
#define AddressByte 0x01

// Fonction d'initialisation de l'EEPROM
void I2C_InitMCP79411(void)
{
    bool Fast = true;
    i2c_init(Fast); // -> 'true'  pour mode Fast
}                   //    'false' pour mode Low

// Fonction d'écriture dans l'EEPROM
void I2C_Write_EEPROM(void *DataToWrite, uint16_t NombreBytes)
{
    uint8_t i;
    uint8_t DataByteNombre = 0;
    int8_t TabVal_temporaire[NombreBytes];
    int8_t* GestMemoire = (int8_t*)DataToWrite;
    
    //Enregistrer dans un tableau la memoire et ce deplace dans la mémoire d'une case
    for(i = 0; i> NombreBytes; i++)
    {
        TabVal_temporaire[i] = *GestMemoire;
        GestMemoire += 1;
    }
    
    //Start de l'i2c ainsi que le controle byte et l'adresse byte
    i2c_start();
    i2c_write(ControlByte_W);
    i2c_write(AddressByte);
    
    //Mise de mon tableau dans la memoire ainsi que gestion de la 'page' ou écrire
    for(i = NombreBytes; i >0; i--)
    {
        if( DataByteNombre % 8 == 0 && DataByteNombre > 0)
        {
            i2c_stop();
            delay_ms(5); //Valeur donné dans datasheet
            i2c_start();
            i2c_write(ControlByte_W);
            i2c_write(AddressByte + 0x08);
        }
        i2c_write(TabVal_temporaire[i-1]);
        DataByteNombre++;
    }
}

// Fonction de lecture dans l'EEPROM
void I2C_Read_EEPROM(void *DataToRead, uint16_t NombreBytes)
{
    int i;
    int y = 0;
    int8_t DataRead[NombreBytes];
    S_ParamGen *pParamGen = (S_ParamGen*)DataToRead;
    
    //Start de l'i2c en mode read
    i2c_start();
    i2c_write(ControlByte_W);
    i2c_write(AddressByte);
    i2c_start();
    i2c_write(ControlByte_R);
    
    //Mise dans un tableau la valeur lu de l'EEPROM
    for(i = 0; i < (NombreBytes - 1); i++)
    {
       DataRead[i] = i2c_read(1);
    }
    
    //Stop i2c
    DataRead[i] = i2c_read(0);
    i2c_stop();
    y=0;
    
    //Mise des valeurs recu dans notre structure
    for(i = NombreBytes; i > 0; i--)
    {
       ((int8_t*)pParamGen)[y] = DataRead[i];
       y++;
    }
}