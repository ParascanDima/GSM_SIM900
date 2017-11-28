/*
 * ifgsm.c
 *
 *  Created on: 11 окт. 2017 г.
 *      Author: Dumitru
 */


/**************Imports***********************************/

#include "ifgsm.h"
#include "custom_string.h"
#include "uart.h"
#include "Timer.h"
#include "io.h"

/**************Private Macro Definitions*****************/



/**************Private Type Definitions******************/



/**************Private Variable Definitions**************/

static uint8_t unexpectRespBuffer[40];


/**************Public Variable Definitions***************/



/**************Private Function Definitions**************/





/**************Public Function Definitions***************/

void gsmUnexpectRspBufferReset(void){
    CustomClrStr((char*)unexpectRespBuffer);
}

void gsmIntToString( uint16_t num, uint8_t* str){
    CustomIntToString(num, str);
}

uint16_t gsmMessageToINT(uint8_t* string, uint8_t nrofSimbolsToConvert){
    return CustomToINT(string, nrofSimbolsToConvert);
}

uint8_t isEmpty(uint8_t* buffer){
    return isBufferEmpty(buffer);
}



uint8_t gsmCompleteResponseReceived(uint8_t* responseBuffer){
    return FindSubStr(responseBuffer, strlen((const char*)responseBuffer), "\r\n", 2);
}

uint8_t gsmCompareResponseWith(uint8_t* str){
    return FindInBuffer(str);
}

uint8_t gsmCompareMessageWith(uint8_t* str){
    return FindInBuffer(str);
}

uint8_t gsmCompareUnexpectRspWith(uint8_t* str){
    return FindSubStr(unexpectRespBuffer, strlen((const char*)unexpectRespBuffer), str, strlen((const char*)str));
}

void gsmCopyUnexpectResponse(uint8_t* unexpectedResponse){
    gsmUnexpectRspBufferReset();
    (void)strcpy((char*)unexpectRespBuffer, (const char *)unexpectedResponse);
}

void gsmReadUnexpectResponseTo(uint8_t * dest){
    (void)strcpy((char*)dest, (const char *)unexpectRespBuffer);
    gsmUnexpectRspBufferReset();
}

/* --------------------------- GPIO dependent code ---------------------------  */

void gsmRstPullDown(void)
{
    GPIO_setOutputHighOnPin(GPIO_PORT_P1, GSM_RST);
}

void gsmRstPullUp(void)
{
    GPIO_setOutputLowOnPin(GPIO_PORT_P1, GSM_RST);
}

void gsmPwrKeyOFF(void)
{
    GPIO_setOutputHighOnPin(GPIO_PORT_P1, PWRKEY);
}

void gsmPwrKeyON(void)
{
    GPIO_setOutputLowOnPin(GPIO_PORT_P1, PWRKEY);
}

/* --------------------------- UART dependent code ---------------------------  */


void gsmTransmit(uint8_t* data){
    uartTransmit(data);
}


void gsmResponseBufferReset(void){
    uartRX_Reset();
}


uint8_t* gsmGet_Response(){
    return uartGet_Buffer();
}


/* --------------------------- TIMER dependent code ---------------------------  */

void gsmDelay_ms(uint16_t miliseconds){
    Delay_ms(miliseconds);
}


uint8_t gsmCheckTimeout( uint64_t planed_ms ){
    return Check_Timeout(planed_ms);
}


uint64_t gsmPlaneTout(uint16_t planed_ms){
    return timerPlane_Tout(planed_ms);
}
