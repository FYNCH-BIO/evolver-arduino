#include "avr/io.h"
#include <stdlib.h>
// F_CPU must be defined before including util/delay.h
#define F_CPU 1000000UL
#include <util/delay.h>
#include <string.h>

#define BAUD 9600UL
#define Q_DEL _delay_loop_2(3)
#define H_DEL _delay_loop_2(5)

// Configuration bytes
#define SLAVE_WRITE 0xD0
#define SLAVE_READ 0xD1
// Differ only in the address bits to select C1 or C2
#define CONFIG_C1 0x88
#define CONFIG_C2 0xA8


// I/O Port Definitions
#define SCLPORT PORTC //TAKE PORTC as SCL OUTPUT WRITE
#define SCLDDR  DDRC  //TAKE DDRC as SCL INPUT/OUTPUT configure

#define SDAPORT PORTB //TAKE PORTB as SDA OUTPUT WRITE
#define SDADDR  DDRB  //TAKE PORTB as SDA INPUT configure

#define SDAPIN  PINB  //TAKE PORTB TO READ DATA
#define SCLPIN  PINC  //TAKE PORTC TO READ DATA

#define SCL PC1   //PORTC.1 PIN AS SCL PIN
#define SDA PB1   //PORTB.1 PIN AS SDA PIN

#define SOFT_I2C_SDA_LOW  SDADDR|=((1<<SDA))
#define SOFT_I2C_SDA_HIGH SDADDR&=(~(1<<SDA))

#define SOFT_I2C_SCL_LOW  SCLDDR|=((1<<SCL))
#define SOFT_I2C_SCL_HIGH SCLDDR&=(~(1<<SCL))

void initSerial();
void sendMessage(char *);
uint8_t writeADC(uint8_t, uint8_t);
unsigned long readADC(uint8_t);
void SoftI2CInit();
void SoftI2CStart();
void SoftI2CStop();
uint8_t SoftI2CWriteByte(uint8_t);
uint8_t SoftI2CReadByte(uint8_t);

int main(void)
{
    char buffer[16];

    initSerial();

    // This is for the serial bus - enables output on porta0
    DDRA |= (1 << 0);

    // Set the serial buffer for output
    PORTA &= (0 << 0);

    SoftI2CInit();

    for( ; ; ) {
        unsigned long value;

        // Read from channel 1
        value = readADC(CONFIG_C1);
        itoa(value, buffer, 10);
        sendMessage(buffer);
        sendMessage("|");

        // Read from channel 2 (blank)
        value = readADC(CONFIG_C2);
        itoa(value, buffer, 10);
        sendMessage(buffer);
        sendMessage("!");

        SoftI2CStop();
    }
    return 0;
}

// Serial comms functions
void initSerial() {
    unsigned long br = ((F_CPU)/(BAUD*8UL)-1);
    UBRR0H = (br >> 8);
    UBRR0L = br;

    UCSR0B = ((1 << RXCIE0) | (1 << RXEN0) | (1 << TXEN0));
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);  //8 bit data format
    UCSR0A |= (1 << U2X0);
}

void sendMessage(char* message) {
    for (int i = 0; i < strlen(message); i++){
        while (( UCSR0A &(1<<UDRE0))  == 0){};
        UDR0 = message[i];
    }
}

uint8_t writeADC(uint8_t address, uint8_t data) {
    SoftI2CStart();
    uint8_t response = SoftI2CWriteByte(address);
    if(response > 0) {
        response = SoftI2CWriteByte(data);
    }
    SoftI2CStop();
    return response;
}

unsigned long readADC(uint8_t config) {
    SoftI2CStart();
    unsigned long value = 1UL;
    uint8_t response = writeADC(SLAVE_WRITE, config);
    uint8_t dataResponse0;
    uint8_t dataResponse1;
    uint8_t dataResponse2;

    if (response == 0) {
        sendMessage("Failed on write 1!");
        return value;
    }

    int ready = 0;

    while (!ready) {
        SoftI2CStart();
        response = SoftI2CWriteByte(SLAVE_READ);
        if (response == 0) {
            SoftI2CStop();
            sendMessage("failed on write 2!");
            return value;
        }

        // ADC returns 3 bytes
        dataResponse0 = SoftI2CReadByte(1);
        dataResponse1 = SoftI2CReadByte(1);
        dataResponse2 = SoftI2CReadByte(0);
        ready = ((dataResponse2 >> 7) & 0x01) == 0;
    }

    //char buffer[16];

    // RDY bit needs to be 0! Otherwise conversion data is not ready.
    if (((dataResponse2 >> 7) & 0x01) == 0) {
        // convert to a base 10
        value = 0;
        // D15 is the sign bit, so skip the 0th bit
        //itoa(((dataResponse0 >> 7) & 0x01), buffer, 10);
        //sendMessage(buffer);
        for (int i = 1; i < 16; i++) {
            value *= 2;
            if (i < 8) {
                value += ((dataResponse0 >> 7 - (i % 8)) & 0x01);
                //itoa((dataResponse0 >> 7 - (i % 8)) & 0x01, buffer, 10);
                //sendMessage(buffer);
            }

            else {
                value += ((dataResponse1 >> 7 - (i % 8)) & 0x01);
                //itoa((dataResponse1 >> 7 - (i % 8)) & 0x01, buffer, 10);
                //sendMessage(buffer);
            }
        }
    }
    //sendMessage("!");

    if (((dataResponse0 >> 7) & 0x01) == 1) {
        value = value * -1;
    }
    return value;
}

void SoftI2CInit()
{
    SDAPORT&=(1<<SDA);
    SCLPORT&=(1<<SCL);

    SOFT_I2C_SDA_HIGH;
    SOFT_I2C_SCL_HIGH;
    H_DEL;

}
void SoftI2CStart()
{
    SOFT_I2C_SCL_HIGH;
    H_DEL;

    SOFT_I2C_SDA_LOW;
    H_DEL;
}

void SoftI2CStop()
{
     SOFT_I2C_SDA_LOW;
     H_DEL;
     SOFT_I2C_SCL_HIGH;
     Q_DEL;
     SOFT_I2C_SDA_HIGH;
     H_DEL;
}

uint8_t SoftI2CWriteByte(uint8_t data)
{

     uint8_t i;

     for(i=0;i<8;i++)
     {
        SOFT_I2C_SCL_LOW;
        Q_DEL;

        if(data & 0x80)
            SOFT_I2C_SDA_HIGH;
        else
            SOFT_I2C_SDA_LOW;

        H_DEL;

        SOFT_I2C_SCL_HIGH;
        H_DEL;

        while((SCLPIN & (1<<SCL))==0);

        data=data<<1;
    }

    //The 9th clock (ACK Phase)
    SOFT_I2C_SCL_LOW;
    Q_DEL;

    SOFT_I2C_SDA_HIGH;
    H_DEL;

    SOFT_I2C_SCL_HIGH;
    H_DEL;

    uint8_t ack=!(SDAPIN & (1<<SDA));

    SOFT_I2C_SCL_LOW;
    H_DEL;

    return ack;

}


uint8_t SoftI2CReadByte(uint8_t ack)
{
    uint8_t data=0x00;
    uint8_t i;

    for(i=0;i<8;i++)
    {

        SOFT_I2C_SCL_LOW;
        H_DEL;
        SOFT_I2C_SCL_HIGH;
        H_DEL;

        while((SCLPIN & (1<<SCL))==0);

        if(SDAPIN &(1<<SDA))
            data|=(0x80>>i);

    }

    SOFT_I2C_SCL_LOW;
    Q_DEL;

    //Soft_I2C_Put_Ack

    if(ack)
    {
        SOFT_I2C_SDA_LOW;
    }
    else
    {
        SOFT_I2C_SDA_HIGH;
    }
    H_DEL;

    SOFT_I2C_SCL_HIGH;
    H_DEL;

    SOFT_I2C_SCL_LOW;
    H_DEL;

    SOFT_I2C_SDA_HIGH;
    H_DEL;

    return data;
}
