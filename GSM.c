/*
 * GSM.c
 *
 *  Created on: 30 апр. 2017 г.
 *      Author: Dumitru
 */


/**************Imports***********************************/

#include "GSM.h"
#include "atcmd.h"
#include "ifgsm.h"
#include <string.h>

/**************Private Macro Definitions*****************/



/**************Private Type Definitions******************/



/**************Private Variable Definitions**************/

static GSM_STATE eGsmState = enIDLE;
static GSM_GPRS_STATE eGprsState = enPDPdeactivated;

/**************Public Variable Definitions***************/



/**************Private Function Definitions**************/


void Set_eGsmState(GSM_STATE state){
    eGsmState = state;
}

void Set_eGprsState(GSM_GPRS_STATE state){
    eGprsState = state;
}

void hardGsmReset(void){
    //Get value at STATUS pin
    if (Get_eGsmState() != enIDLE/* && STATUS_pin is high*/){
        GSM_PowerDown();
        gsmDelay_ms(4000);
    }
    gsmRstPullDown();
    gsmDelay_ms(2000);
    gsmRstPullUp();
    gsmDelay_ms(800);
    gsmPwrKeyOFF();
    gsmDelay_ms(1200);
    gsmPwrKeyON();
    gsmDelay_ms(2000);
    Set_eGsmState(enIDLE);
}


/*MAIN FUNCTION TO CONNECT TO SERVER */



/**************Public Function Definitions***************/


GSM_STATE Get_eGsmState(void){
    return eGsmState;
}

GSM_GPRS_STATE Get_eGPRSState(void){
    return eGprsState;
}

uint8_t GSM_ReceiveAtCommandResponse(uint16_t timeout, uint8_t* expectedResponse, uint8_t isStateChanging, uint8_t state, uint8_t isRspNeeded){
    uint64_t _timeout = 0;
    uint8_t * buffer = gsmGet_Response();

    _timeout = gsmPlaneTout(timeout);
    while(gsmCheckTimeout(_timeout)){
        if (gsmCompareResponseWith(expectedResponse)){
            if (isStateChanging == GSM_GSM_STATE_CHANGES){
                Set_eGsmState((GSM_STATE)state);
            }
            else{
                if (isStateChanging == GSM_GPRS_STATE_CHANGES){
                    Set_eGprsState((GSM_GPRS_STATE)state);
                }
            }
            if(!isRspNeeded){
                gsmResponseBufferReset();
            }
            return GSM_OK;
        }
        else{
            if(gsmCompareResponseWith("ERROR")){
                gsmResponseBufferReset();
                return GSM_ERROR;
            }
        }
    }
    if (!isEmpty(buffer)){
        gsmCopyUnexpectResponse(buffer);
        if(!isRspNeeded){
            gsmResponseBufferReset();
        }
    }
    return GSM_TIMEOUT;
}

uint8_t GSM_CheckInit(void){
    uint8_t tRsp;

    tRsp = GSM_ReceiveAtCommandResponse(500, "RDY", GSM_GSM_STATE_CHANGES, (uint8_t)enRDY, GSM_RESPONSE_SAVE_NEEDED);
    if (tRsp == GSM_OK)
    {
        tRsp = GSM_ReceiveAtCommandResponse(500, "+CPIN: READY", GSM_NO_STATE_CHANGES, GSM_NO_STATE_CHANGES, GSM_RESPONSE_SAVE_NEEDED);
        if (tRsp == GSM_OK){
            tRsp = GSM_ReceiveAtCommandResponse(500, "+CFUN: 1", GSM_NO_STATE_CHANGES, GSM_NO_STATE_CHANGES, GSM_NO_RESPONSE_SAVE_NEEDED);
        }
        else{
            if(tRsp != GSM_ERROR){
                if (gsmCompareUnexpectRspWith("+CPIN: SIM PIN")){
                    gsmUnexpectRspBufferReset();
                    tRsp = GSM_ENTER_PIN;
                }
                else{
                    if (gsmCompareUnexpectRspWith("+CPIN: NOT INSERTED"))
                    {
                        gsmUnexpectRspBufferReset();
                        tRsp = GSM_INSERT_SIM;
                    }
                }
            }
        }
    }
    return tRsp;
}

uint8_t GSM_ReceiveCallResponse(void){

    uint64_t timeout = 0;
    uint8_t * buffer = gsmGet_Response();
    uint8_t tRsp;


    timeout = gsmPlaneTout(2000);
    while(gsmCheckTimeout(timeout)){
        if (gsmCompareResponseWith("OK")){
            gsmResponseBufferReset();
            while(Get_eGsmState() == enCalling){
                if (gsmCompareResponseWith("NO DIALTONE")){
                    gsmResponseBufferReset();
                    Set_eGsmState(enCallReady);
                    return GSM_CALL_NO_DIALTONE;
                }
                else{
                    if (gsmCompareResponseWith("BUSY")){
                        gsmResponseBufferReset();
                        Set_eGsmState(enCallReady);
                        return GSM_CALL_BUSY;
                    }
                    else{
                        if (gsmCompareResponseWith("NO ANSWER")){
                            gsmResponseBufferReset();
                            Set_eGsmState(enCallReady);
                            return GSM_CALL_NO_ANSWER;
                        }
                        else{
                            if (gsmCompareResponseWith("NO CARRIER")){
                                gsmResponseBufferReset();
                                Set_eGsmState(enCallReady);
                                return GSM_CALL_NO_CARRIER;
                            }
                        }
                    }
                }
            }
            return GSM_CALL_OK;
        }
    }
    gsmResponseBufferReset();
    tRsp = GSM_TIMEOUT;

    return tRsp;
}



GsmHttp_Response GSM_GetHTTPResponseCode(uint8_t method){
    uint8_t tRsp;
    GsmHttp_Response httpRsp = GSM_HTTP_RESPONSE_NETWORK_ERROR;
    uint8_t _methodString[2] = "";
    uint8_t expectedRsp[16] = "+HTTPACTION:";
    uint8_t expectedRspIndex = 0;

    gsmIntToString((uint16_t)method, _methodString);

    strcat((char*)expectedRsp, (char*)_methodString);            /* The expected string is "+HTTPACTION:X,YYY,Z */

    uint8_t * buffer = gsmGet_Response();


    tRsp = GSM_ReceiveAtCommandResponse(HTTP_METHOD_TIMEOUT, expectedRsp, GSM_NO_STATE_CHANGES, GSM_NO_STATE_CHANGES, GSM_RESPONSE_SAVE_NEEDED);
    if(tRsp == GSM_OK){
        expectedRspIndex = (uint8_t)((uint8_t*)strstr((const char*)buffer, (const char*)expectedRsp) - buffer);
        while(!gsmCompleteResponseReceived(&buffer[expectedRspIndex]));
        httpRsp = gsmMessageToINT(&buffer[expectedRspIndex+14], 3);
        gsmResponseBufferReset();
    }

    return httpRsp;
}




uint8_t CheckGsmPersistance(void){

    gsmTransmit("AT\r\n");

    return GSM_ReceiveAtCommandResponse(50, "OK", GSM_NO_STATE_CHANGES, GSM_NO_STATE_CHANGES, GSM_NO_RESPONSE_SAVE_NEEDED);
}


uint8_t GSM_SetBaudRate(uint16_t baud){

    gsmAT_IPR(baud);

    return GSM_ReceiveAtCommandResponse(200, "OK", GSM_NO_STATE_CHANGES, GSM_NO_STATE_CHANGES, GSM_NO_RESPONSE_SAVE_NEEDED);
}


uint8_t GSM_PowerDown(void){

    gsmAT_CPOWD();

    return GSM_ReceiveAtCommandResponse(1000, "OK", GSM_GSM_STATE_CHANGES, (uint8_t)enIDLE, GSM_NO_RESPONSE_SAVE_NEEDED);
}


uint8_t GSM_EnterPinCode(uint16_t pinCode){

    gsmAT_CPIN(pinCode);

    return GSM_ReceiveAtCommandResponse(200, "OK", GSM_NO_STATE_CHANGES, GSM_NO_STATE_CHANGES, GSM_NO_RESPONSE_SAVE_NEEDED);
}


uint8_t GSM_CallNumber(uint8_t* phoneNumber){

    gsmATD(phoneNumber);

    Set_eGsmState(enCalling);

    return GSM_ReceiveCallResponse();
}


uint8_t GSM_AttachToGPRS(void){

    gsmAT_CGATT(1);

    return GSM_ReceiveAtCommandResponse(6000, "OK", GSM_GPRS_STATE_CHANGES, (uint8_t)enIPInitial, GSM_NO_RESPONSE_SAVE_NEEDED);
}


uint8_t GSM_DetachToGPRS(void){

    gsmAT_CGATT(0);

    return GSM_ReceiveAtCommandResponse(1000, "OK", GSM_GPRS_STATE_CHANGES, (uint8_t)enPDPdeactivated, GSM_NO_RESPONSE_SAVE_NEEDED);
}


uint8_t GSM_SetupAPN(uint8_t* apn){

    gsmAT_CSTT(apn);

    return GSM_ReceiveAtCommandResponse(1000, "OK", GSM_GPRS_STATE_CHANGES, (uint8_t)enIPstart, GSM_NO_RESPONSE_SAVE_NEEDED);
}


uint8_t GSM_GprsSingleConnectionMode(void){

    gsmAT_CIPMUX(0);

    return GSM_ReceiveAtCommandResponse(1000, "OK", GSM_NO_STATE_CHANGES, GSM_NO_STATE_CHANGES, GSM_NO_RESPONSE_SAVE_NEEDED);
}


uint8_t GSM_StartGPRS(void){

    gsmAT_CIICR();

    return GSM_ReceiveAtCommandResponse(6000, "OK", GSM_GPRS_STATE_CHANGES, (uint8_t)enIPConfig, GSM_NO_RESPONSE_SAVE_NEEDED);
}


uint8_t GSM_GetLocalIP(void){

    gsmAT_CIFSR();

    return GSM_ReceiveAtCommandResponse(5000, "10.12.", GSM_GPRS_STATE_CHANGES, (uint8_t)enIPStatus, GSM_NO_RESPONSE_SAVE_NEEDED);
}


uint8_t GSM_GprsShutDown(void){

    gsmAT_CIPSHUT();

    return GSM_ReceiveAtCommandResponse(1000, "SHUT OK", GSM_GPRS_STATE_CHANGES, (uint8_t)enPDPdeactivated, GSM_NO_RESPONSE_SAVE_NEEDED);
}


uint8_t GSM_ConnectToServer(uint8_t* host, uint8_t* port){

    gsmAT_CIPSTART(host, port);

    return GSM_ReceiveAtCommandResponse(4000, "CONNECT OK", GSM_GPRS_STATE_CHANGES, (uint8_t)enIPprocessing, GSM_NO_RESPONSE_SAVE_NEEDED);
}


uint8_t GSM_InitHTTP(void){

    gsmAT_HTTPINIT();

    return GSM_ReceiveAtCommandResponse(1000, "OK", GSM_NO_STATE_CHANGES, GSM_NO_STATE_CHANGES, GSM_NO_RESPONSE_SAVE_NEEDED);
}


uint8_t GSM_SetHttpParameters(uint8_t HTTPParamTag, uint8_t* HTTPParmValue){

    gsmAT_HTTPPARA(HTTPParamTag, HTTPParmValue);

    return GSM_ReceiveAtCommandResponse(1000, "OK", GSM_NO_STATE_CHANGES, GSM_NO_STATE_CHANGES, GSM_NO_RESPONSE_SAVE_NEEDED);
}


uint8_t GSM_SetHTTPDataToTransfer(uint8_t* postData, uint16_t nrOfBytes, uint16_t latencyTime){
    uint8_t tRsp = GSM_ERROR;

    gsmAT_HTTPDATA(nrOfBytes, latencyTime);

    tRsp = GSM_ReceiveAtCommandResponse(1000, "DOWNLOAD", GSM_NO_STATE_CHANGES, GSM_NO_STATE_CHANGES, GSM_NO_RESPONSE_SAVE_NEEDED);
    if(tRsp == GSM_OK){
        gsmHttpSendData(postData);
        tRsp = GSM_ReceiveAtCommandResponse(latencyTime, "OK", GSM_NO_STATE_CHANGES, GSM_NO_STATE_CHANGES, GSM_NO_RESPONSE_SAVE_NEEDED);
    }

    return tRsp;
}


uint8_t GSM_SetBearerParameters( uint8_t* ConParamTag, uint8_t* ConParamValue){

    gsmAT_SAPBR(3, 1, ConParamTag, ConParamValue);

    return GSM_ReceiveAtCommandResponse(500, "OK", GSM_NO_STATE_CHANGES, GSM_NO_STATE_CHANGES, GSM_NO_RESPONSE_SAVE_NEEDED);
}


uint8_t GSM_OpenBearer(void){
    gsmAT_SAPBR(1, 1, NULL, NULL);

    return GSM_ReceiveAtCommandResponse(6000, "OK", GSM_GPRS_STATE_CHANGES, (uint8_t)enIPStatus, GSM_NO_RESPONSE_SAVE_NEEDED);
}


uint8_t GSM_HttpMethodAction(uint8_t method){

    gsmAT_HTTPACTION(method);

    return GSM_GetHTTPResponseCode(method);
}


uint8_t GSM_HTTPTerminateService(void){

    gsmAT_HTTPTERM();

    return GSM_ReceiveAtCommandResponse(3000, "OK", GSM_NO_STATE_CHANGES, GSM_NO_STATE_CHANGES, GSM_NO_RESPONSE_SAVE_NEEDED);
}



void GSM_Init(void)
{
    uint8_t tries = 0;
    uint8_t response;
    uint8_t incorrectPIN = 0;

    while (Get_eGsmState() != enOK && Get_eGsmState() != enNoSIM && Get_eGsmState() != enUnreachable){

        if (tries == MAX_TRIES){
            Set_eGsmState(enUnreachable);
            break;
        }

        hardGsmReset();
        response = GSM_CheckInit();
        if (response == GSM_OK){
            response = GSM_ReceiveAtCommandResponse(8000, "Call Ready", GSM_GSM_STATE_CHANGES, (uint8_t)enCallReady, GSM_NO_RESPONSE_SAVE_NEEDED);
            if(response == GSM_OK){
                response = GSM_SetBaudRate(9600);
                if(response == GSM_OK){
                    Set_eGsmState(enOK);
                    GSM_ActivateGprs();
                }
            }
        }
        else{
            if(response == GSM_ENTER_PIN){
                if (incorrectPIN == 1){
                    break;
                }
                response = GSM_EnterPinCode(DEFAULT_PIN_CODE);
                if (response == GSM_ERROR){
                    incorrectPIN++;
                }
            }
            else{
                if (response == GSM_INSERT_SIM){
                    Set_eGsmState(enNoSIM);
                    break;
                }
            }
        }

        if (response == GSM_TIMEOUT){
            tries++;
        }
    }
}


void GSM_ServerSocketConnection(uint8_t* ip_addr, uint8_t* port){
    uint8_t resp = GSM_ERROR;

    if(enIPStatus != Get_eGPRSState()){
        (void)GSM_GprsShutDown();
        (void)GSM_DetachToGPRS();
        if (enPDPdeactivated == Get_eGPRSState()){
            resp = GSM_GprsSingleConnectionMode();
            if(resp == GSM_OK){
                resp = GSM_AttachToGPRS();
                if(enIPInitial == Get_eGPRSState()){
                    (void)GSM_SetupAPN("moldcell");
                    if(enIPstart == Get_eGPRSState()){
                        (void)GSM_StartGPRS();
                        if(enIPGprsAct == Get_eGPRSState()){
                            (void) GSM_GetLocalIP();
                            if(enIPStatus == Get_eGPRSState()){
                                (void)GSM_StartServerConnection(ip_addr, port);
                            }
                        }
                    }
                }
            }
        }
    }
    else{
        (void)GSM_StartServerConnection(ip_addr, port);
    }
}

GsmHttp_Response GSM_HttpPost(uint8_t* urlPath, uint8_t* postData){
    GsmHttp_Response tRsp;
    uint8_t responses;
    size_t postDataLen = strlen((const char*)postData);

    if (enIPStatus == Get_eGPRSState() || enIPprocessing == Get_eGPRSState())
    {
        responses = GSM_InitHTTP();
        if( GSM_OK == responses )
        {
            responses = GSM_SetHttpParameters(GSM_HTTP_PARAM_CID, "1");
            if( GSM_OK == responses )
            {
                responses = GSM_SetHttpParameters(GSM_HTTP_PARAM_URL, urlPath);
                if( GSM_OK == responses )
                {
                    responses = GSM_SetHTTPDataToTransfer(postData, postDataLen, 10000);
                    if (GSM_OK == responses)
                    {
                        tRsp = GSM_HttpMethodAction(GSM_HTTP_METHOD_POST);
                        GSM_HTTPTerminateService();
                    }
                }
            }
        }
    }
    return tRsp;
}

GsmHttp_Response GSM_HttpGet(uint8_t* urlPath){
    GsmHttp_Response tRsp;
    uint8_t responses;

    if (enIPStatus == Get_eGPRSState() || enIPprocessing == Get_eGPRSState())
    {
        responses = GSM_InitHTTP();
        if( GSM_OK == responses )
        {
            responses = GSM_SetHttpParameters(GSM_HTTP_PARAM_CID, "1");
            if( GSM_OK == responses )
            {
                responses = GSM_SetHttpParameters(GSM_HTTP_PARAM_URL, urlPath);
                if( GSM_OK == responses )
                {
                    tRsp = GSM_HttpMethodAction(GSM_HTTP_METHOD_GET);
                    GSM_HTTPTerminateService();
                }
            }
        }
    }
    return tRsp;
}

void GSM_ActivateGprs(void){
    uint8_t resp;
    uint8_t tries = 0;

    while (enIPStatus != Get_eGPRSState())
    {
        if (tries == MAX_TRIES){
            Set_eGprsState(enPDPdeactivated);
            if (resp == GSM_TIMEOUT){
                Set_eGsmState(enUnreachable);
            }
            break;
        }

        if(enIPInitial != Get_eGPRSState()){
            GSM_GprsShutDown();
            if (enPDPdeactivated == Get_eGPRSState())
            {
                resp = GSM_GprsSingleConnectionMode();
                if(resp == GSM_OK)
                {
                    resp = GSM_SetBearerParameters("Contype", "GPRS");
                    if(resp == GSM_OK)
                    {
                        resp = GSM_SetBearerParameters("APN", "moldcell");
                        if (resp == GSM_OK)
                        {
                            resp = GSM_AttachToGPRS();
                            if (enIPInitial == Get_eGPRSState())
                            {
                                resp = GSM_OpenBearer();
                            }
                            else{
                                if(resp == GSM_ERROR){
                                    resp = GSM_DetachToGPRS();
                                    if(resp == GSM_OK){
                                        gsmDelay_ms(3000);
                                        resp = GSM_AttachToGPRS();
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        else{
            resp = GSM_OpenBearer();
        }

        if (resp != GSM_OK){
            tries++;
        }
    }
}

uint8_t GSM_StartServerConnection(uint8_t* host, uint8_t* port){
    uint8_t resp = GSM_ERROR;
    uint8_t tries = 0;


    if(enIPStatus == Get_eGPRSState())
    {
        while ( enIPprocessing != Get_eGPRSState() )
        {
            if (tries == MAX_TRIES){
                Set_eGprsState(enPDPdeactivated);
                if (resp == GSM_TIMEOUT){
                    Set_eGsmState(enUnreachable);
                }
                break;
            }

            resp = GSM_ConnectToServer(host, port);

            if (resp != GSM_OK){
                tries++;
            }
        }
    }
    return resp;
}


void GSM_StopGprsConnection(void){
    if(enIPprocessing == Get_eGPRSState())
        GSM_GprsShutDown();
}

