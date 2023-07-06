/**
  I2C Generated Driver File

  @Company
    Microchip Technology Inc.

  @File Name
    i2c_slave.c

  @Summary
    This is the generated driver implementation file for the I2C driver using PIC10 / PIC12 / PIC16 / PIC18 MCUs

  @Description
    This header file provides implementations for driver APIs for I2C.
    Generation Information :
        Product Revision  :  PIC10 / PIC12 / PIC16 / PIC18 MCUs - 1.81.8
        Device            :  PIC16LF1936
        Driver Version    :  2.0.1
    The generated drivers are tested against the following:
        Compiler          :  XC8 2.36 and above or later
        MPLAB             :  MPLAB X 6.00
*/

/*
    (c) 2018 Microchip Technology Inc. and its subsidiaries. 
    
    Subject to your compliance with these terms, you may use Microchip software and any 
    derivatives exclusively with Microchip products. It is your responsibility to comply with third party 
    license terms applicable to your use of third party software (including open source software) that 
    may accompany Microchip software.
    
    THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES, WHETHER 
    EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY 
    IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS 
    FOR A PARTICULAR PURPOSE.
    
    IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE, 
    INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND 
    WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP 
    HAS BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE. TO 
    THE FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL 
    CLAIMS IN ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT 
    OF FEES, IF ANY, THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS 
    SOFTWARE.
*/

#include "i2c_slave.h"
#include <xc.h>

#define I2C_SLAVE_ADDRESS      8
#define I2C_SLAVE_MASK         127

typedef enum
{
    I2C_IDLE,
    I2C_ADDR_TX,
    I2C_ADDR_RX,
    I2C_DATA_TX,
    I2C_DATA_RX
} i2c_slave_state_t;

/**
 Section: Global Variables
 */
volatile uint8_t i2cWrData;
volatile uint8_t i2cRdData;
volatile uint8_t i2cSlaveAddr;
static volatile i2c_slave_state_t i2cSlaveState = I2C_IDLE;

/**
 Section: Functions declaration
 */
static void I2C_Isr(void);
static void I2C_SlaveDefRdInterruptHandler(void);
static void I2C_SlaveDefWrInterruptHandler(void);
static void I2C_SlaveDefAddrInterruptHandler(void);
static void I2C_SlaveDefWrColInterruptHandler(void);
static void I2C_SlaveDefBusColInterruptHandler(void);

static void I2C_SlaveRdCallBack(void);
static void I2C_SlaveWrCallBack(void);
static void I2C_SlaveAddrCallBack(void);
static void I2C_SlaveWrColCallBack(void);
static void I2C_SlaveBusColCallBack(void);

static inline bool I2C_SlaveOpen();
static inline void I2C_SlaveClose();
static inline void I2C_SlaveSetSlaveAddr(uint8_t slaveAddr);
static inline void I2C_SlaveSetSlaveMask(uint8_t maskAddr);
static inline void I2C_SlaveEnableIrq(void);
static inline bool I2C_SlaveIsAddr(void);
static inline bool I2C_SlaveIsRead(void);
static inline void I2C_SlaveClearBuff(void);
static inline void I2C_SlaveClearIrq(void);
static inline void I2C_SlaveReleaseClock(void);
static inline bool I2C_SlaveIsWriteCollision(void);
static inline bool I2C_SlaveIsTxBufEmpty(void);
static inline bool I2C_SlaveIsData(void);
static inline void I2C_SlaveRestart(void);
static inline bool I2C_SlaveIsRxBufFull(void);
static inline void I2C_SlaveSendTxData(uint8_t data);
static inline uint8_t I2C_SlaveGetRxData(void);
static inline uint8_t I2C_SlaveGetAddr(void);
static inline void I2C_SlaveSendAck(void);
static inline void I2C_SlaveSendNack(void);
static inline bool I2C_SlaveIsOverFlow(void);

void I2C_Initialize()
{
    SSPSTAT  = 0x00;
    SSPCON1 |= 0x06;
    SSPCON2  = 0x00;
    SSPCON1bits.SSPEN = 0;
}

void I2C_Open() 
{
    I2C_SlaveOpen();
    I2C_SlaveSetSlaveAddr(I2C_SLAVE_ADDRESS);
    I2C_SlaveSetSlaveMask(I2C_SLAVE_MASK);
    I2C_SlaveSetIsrHandler(I2C_Isr);
    I2C_SlaveSetBusColIntHandler(I2C_SlaveDefBusColInterruptHandler);
    I2C_SlaveSetWriteIntHandler(I2C_SlaveDefWrInterruptHandler);
    I2C_SlaveSetReadIntHandler(I2C_SlaveDefRdInterruptHandler);
    I2C_SlaveSetAddrIntHandler(I2C_SlaveDefAddrInterruptHandler);
    I2C_SlaveSetWrColIntHandler(I2C_SlaveDefWrColInterruptHandler);
    I2C_SlaveEnableIrq();    
}

void I2C_Close() 
{
    I2C_SlaveClose();
}

uint8_t I2C_Read()
{
   return I2C_SlaveGetRxData();
}

void I2C_Write(uint8_t data)
{
    I2C_SlaveSendTxData(data);
}

bool I2C_IsRead()
{
    return I2C_SlaveIsRead();
}

void I2C_Enable()
{
    I2C_Initialize();
}

void I2C_SendAck()
{
    I2C_SlaveSendAck();
}

void I2C_SendNack()
{
    I2C_SlaveSendNack();
}

static void I2C_Isr() 
{ 
    I2C_SlaveClearIrq();

    if(I2C_SlaveIsAddr())
    {
        if(I2C_SlaveIsRead())
        {
            i2cSlaveState = I2C_ADDR_TX;
        }
        else
        {
            i2cSlaveState = I2C_ADDR_RX;
        }
    }
    else
    {
        if(I2C_SlaveIsRead())
        {
            i2cSlaveState = I2C_DATA_TX;
        }
        else
        {
            i2cSlaveState = I2C_DATA_RX;
        }
    }

    switch(i2cSlaveState)
    {
        case I2C_ADDR_TX:
            I2C_SlaveAddrCallBack();
            if(I2C_SlaveIsTxBufEmpty())
            {
                I2C_SlaveWrCallBack();
            }
            break;
        case I2C_ADDR_RX:
            I2C_SlaveAddrCallBack();
            break;
        case I2C_DATA_TX:
            if(I2C_SlaveIsTxBufEmpty())
            {
                I2C_SlaveWrCallBack();
            }
            break;
        case I2C_DATA_RX:
            if(I2C_SlaveIsRxBufFull())
            {
                I2C_SlaveRdCallBack();
            }
            break;
        default:
            break;
    }
    I2C_SlaveReleaseClock();
}

// Common Event Interrupt Handlers
void I2C_SlaveSetIsrHandler(i2cInterruptHandler handler)
{
    MSSP_InterruptHandler = handler;
}

// Read Event Interrupt Handlers
void I2C_SlaveSetReadIntHandler(i2cInterruptHandler handler) {
    I2C_SlaveRdInterruptHandler = handler;
}

static void I2C_SlaveRdCallBack() {
    // Add your custom callback code here
    if (I2C_SlaveRdInterruptHandler) 
    {
        I2C_SlaveRdInterruptHandler();
    }
}

static void I2C_SlaveDefRdInterruptHandler() {
    i2cRdData = I2C_SlaveGetRxData();
}

// Write Event Interrupt Handlers
void I2C_SlaveSetWriteIntHandler(i2cInterruptHandler handler) {
    I2C_SlaveWrInterruptHandler = handler;
}

static void I2C_SlaveWrCallBack() {
    // Add your custom callback code here
    if (I2C_SlaveWrInterruptHandler) 
    {
        I2C_SlaveWrInterruptHandler();
    }
}

static void I2C_SlaveDefWrInterruptHandler() {
    I2C_SlaveSendTxData(i2cWrData);
}

// ADDRESS Event Interrupt Handlers
void I2C_SlaveSetAddrIntHandler(i2cInterruptHandler handler){
    I2C_SlaveAddrInterruptHandler = handler;
}

static void I2C_SlaveAddrCallBack() {
    // Add your custom callback code here
    if (I2C_SlaveAddrInterruptHandler) {
        I2C_SlaveAddrInterruptHandler();
    }
}

static void I2C_SlaveDefAddrInterruptHandler() {
    i2cSlaveAddr = I2C_SlaveGetRxData();
}

// Write Collision Event Interrupt Handlers
void I2C_SlaveSetWrColIntHandler(i2cInterruptHandler handler){
    I2C_SlaveWrColInterruptHandler = handler;
}

static void  I2C_SlaveWrColCallBack() {
    // Add your custom callback code here
    if ( I2C_SlaveWrColInterruptHandler) 
    {
         I2C_SlaveWrColInterruptHandler();
    }
}

static void I2C_SlaveDefWrColInterruptHandler() {
}

// Bus Collision Event Interrupt Handlers
void I2C_SlaveSetBusColIntHandler(i2cInterruptHandler handler){
    I2C_SlaveBusColInterruptHandler = handler;
}

static void  I2C_SlaveBusColCallBack() {
    // Add your custom callback code here
    if ( I2C_SlaveBusColInterruptHandler) 
    {
         I2C_SlaveBusColInterruptHandler();
    }
}

static void I2C_SlaveDefBusColInterruptHandler() {
}

static inline bool I2C_SlaveOpen()
{
    if(!SSPCON1bits.SSPEN)
    {      
        SSPSTAT  = 0x00;
        SSPCON1 |= 0x06;
        SSPCON2  = 0x00;
        SSPCON1bits.SSPEN = 1;
        return true;
    }
    return false;
}

static inline void I2C_SlaveClose()
{
    SSPSTAT  = 0x00;
    SSPCON1 |= 0x06;
    SSPCON2  = 0x00;
    SSPCON1bits.SSPEN = 0;
}

static inline void I2C_SlaveSetSlaveAddr(uint8_t slaveAddr)
{
    SSPADD = (uint8_t) (slaveAddr << 1);
}

static inline void I2C_SlaveSetSlaveMask(uint8_t maskAddr)
{
    SSPMSK = (uint8_t) (maskAddr << 1);
}

static inline void I2C_SlaveEnableIrq()
{
    PIE1bits.SSPIE = 1;
}

static inline bool I2C_SlaveIsAddr()
{
    return !(SSPSTATbits.D_nA);
}

static inline bool I2C_SlaveIsRead()
{
    return (SSPSTATbits.R_nW);
}

static inline void I2C_SlaveClearIrq()
{
    PIR1bits.SSPIF = 0;
}

static inline void I2C_SlaveReleaseClock()
{
    SSPCON1bits.CKP = 1;
}

static inline bool I2C_SlaveIsWriteCollision()
{
    return SSPCON1bits.WCOL;
}

static inline bool I2C_SlaveIsData()
{
    return SSPSTATbits.D_nA;
}

static inline void I2C_SlaveRestart(void)
{
    SSPCON2bits.RSEN = 1;
}

static inline bool I2C_SlaveIsTxBufEmpty()
{
    return !SSPSTATbits.BF;
}

static inline bool I2C_SlaveIsRxBufFull()
{
    return SSPSTATbits.BF;
}

static inline void I2C_SlaveSendTxData(uint8_t data)
{
    SSPBUF = data;
}

static inline uint8_t I2C_SlaveGetRxData()
{
    return SSPBUF;
}

static inline uint8_t I2C_SlaveGetAddr()
{
    return SSPADD;
}

static inline void I2C_SlaveSendAck()
{
    SSPCON2bits.ACKDT = 0;
    SSPCON2bits.ACKEN = 1;
}

static inline void I2C_SlaveSendNack()
{
    SSPCON2bits.ACKDT = 1;
    SSPCON2bits.ACKEN = 1;
}

static inline bool I2C_SlaveIsOverFlow()
{
    return SSPCON1bits.SSPOV;
}