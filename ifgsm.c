/*
 * ifgsm.c
 *
 *  Created on: 11 окт. 2017 г.
 *      Author: Dumitru
 */


/**************Imports***********************************/

#include "ifgsm.h"
#include <string.h>
#include "custom_string.h"    /* You can use the custom module to work with strings */
//#include "uart.h"             /* Include the UART module */
//#include "Timer.h"            /* Include the TIMER module */
//#include "io.h"               /* Include the IO module */

/**************Private Macro Definitions*****************/

#define UNEXPECTED_RSP_MSG_LENGTH   40

/**************Private Type Definitions******************/



/**************Private Variable Definitions**************/

static uint8_t unexpectRespBuffer[UNEXPECTED_RSP_MSG_LENGTH];


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
    /* Set the pin connected to RST of SIM900 to LOW ("0") */
}

void gsmRstPullUp(void)
{
    /* Set the pin connected to RST of SIM900 to HIGH ("1") */
}

void gsmPwrKeyOFF(void)
{
    /* Set the pin connected to PWRKEY of SIM900 to LOW ("0") */
}

void gsmPwrKeyON(void)
{
    /* Set the pin connected to PWRKEY of SIM900 to HIGH ("1") */
}

/* --------------------------- UART dependent code ---------------------------  */


void gsmTransmit(uint8_t* data){
    /* UART transmit interface */
}


void gsmResponseBufferReset(void){
    /* Clear the UART buffer interface */
}


uint8_t* gsmGet_Response(){
    /* Get the UART buffer interface */
}

uint8_t gsmCompareResponseWith(uint8_t* str){
    /* Verify if "str" string is in UART buffer */
}



/* --------------------------- TIMER dependent code ---------------------------  */

void gsmDelay_ms(uint16_t miliseconds){
    /* The delay function implemented by using timer */
}


uint8_t gsmCheckTimeout( uint64_t planed_ms ){
    /* This function check if the planed time reached */

    /* Example:

    if (planed_ms < time_quant_count){
        tRsp = FALSE;
    }
    else{
        tRsp = TRUE;
    }

     * */
}


uint64_t gsmPlaneTout(uint16_t planed_ms){
    /* This function get the counter value of type uint64 that is incremented every 1 ms and add to this
     * "planed_ms" */
}
