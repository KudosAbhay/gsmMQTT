/*
  MQTT.h - Library for GSM MQTT Client.
  Created by Nithin K. Kurian, Dhanish Vijayan, Elementz Engineers Guild Pvt. Ltd, July 2, 2016.
  Released into the public domain.
*/

#include "GSM_MQTT.h"
#include "Arduino.h"
#include <SoftwareSerial.h>
#include <avr/pgmspace.h>
extern uint8_t GSM_Response;

//extern SoftwareSerial Serial1;
extern String MQTT_HOST;
extern String MQTT_PORT;

extern GSM_MQTT MQTT;
uint8_t GSM_Response = 0;
unsigned long previousMillis = 0;
//char inputString[UART_BUFFER_LENGTH];         // a string to hold incoming data
boolean stringComplete = false;  // whether the string is complete
void serialEvent();

void serialEvent1();
uint8_t count = 1;



GSM_MQTT::GSM_MQTT(unsigned long KeepAlive)
{
  _KeepAliveTimeOut = KeepAlive;
}

void GSM_MQTT::begin(void)
{
  Serial1.begin(9600);
  Serial.begin(9600);
  Serial.write("AT\r\n");
  delay(1000);
  _tcpInit();
}




char GSM_MQTT::_sendAT(char *command, unsigned long waitms)
{

  unsigned long PrevMillis = millis();
  strcpy(reply, "none");
  GSM_Response = 0;
  Serial.println(command);
  
  Serial1.println(command);
  
  unsigned long currentMillis = millis();
  //  Serial1.println(PrevMillis);
  //  Serial1.println(currentMillis);
  while ( (GSM_Response == 0) && ((currentMillis - PrevMillis) < waitms) )
  {
    serialEvent();
    currentMillis = millis();
  }
  return GSM_Response;
}






char GSM_MQTT::sendATreply(char *command, char *replystr, unsigned long waitms)
{
  strcpy(reply, replystr);
  unsigned long PrevMillis = millis();
  GSM_ReplyFlag = 0;
  Serial.println(command);
  
  Serial1.println(command);
  
  unsigned long currentMillis = millis();

  //  Serial1.println(PrevMillis);
  //  Serial1.println(currentMillis);
  while ( (GSM_ReplyFlag == 0) && ((currentMillis - PrevMillis) < waitms) )
  {
    serialEvent();
    currentMillis = millis();
  }
  return GSM_ReplyFlag;
}



void GSM_MQTT::_tcpInit(void)
{
  switch (modemStatus)
  {
    case 0:
      {
        delay(1000);
        Serial.print("\n----Starting------\n");
        delay(500);
        if (_sendAT("AT\r\n", 5000) == 1)
        {
          modemStatus = 1;
        }
        else
        {
          modemStatus = 0;
          break;
        }
      }
    case 1:
      {
        if (_sendAT("AT+CIPSHUT\r\n", 2000) == 1)
        {
          modemStatus = 2;          
        }
        else
        {
          modemStatus = 1;
          break;
        }
      }
    case 2:
      {
        if (sendATreply("AT+CREG?\r\n", "0,1", 5000) == 1)
        {
          _sendAT("AT+CIPMUX=0\r\n", 2000);
          _sendAT("AT+CIPMODE=1\r\n", 2000);
          if (sendATreply("AT+CGATT?\r\n", ": 1", 4000) != 1)
          {
            _sendAT("AT+CGATT=1\r\n", 2000);
          }
//..............................................................................................
          if(sendATreply("AT+CSTT=\"airtelgprs.com\"\r\n","OK", 5000) == 1)
          {
            if(sendATreply("AT+CIPSTATUS\r\n","STATE: IP START",4000) == 1)
            {
              Serial.println("\n OKAY \n");
            }
            else
            {
              Serial.println("\n NOT OKAY \n");
            }
            Serial.println((char)_tcpStatus);
          }

          
          if(sendATreply("AT+CIICR\r\n","OK", 5000) == 1)
          {
            if(sendATreply("AT+CIPSTATUS\r\n","STATE: IP GPRSACT",4000) == 1)
            {
              Serial.println("\n OKAY \n");
            }
            else
            {
              Serial.println("\n NOT OKAY \n");
            }
            Serial.println((char)_tcpStatus);
          }

                    
          if(sendATreply("AT+CIFSR\r\n",".", 5000) == 1)
          {
            _tcpStatus= sendATreply("AT+CIPSTATUS\r\n","STATE",4000);
            Serial.println((char)_tcpStatus);          
          }

//..............................................................................................          
          modemStatus = 3;
          _tcpStatus = 5;
        }
        else
        {
          modemStatus = 2;
          break;
        }
      }
    case 3:
      {
        Serial.println("\n in Case 3\n");
        if (GSM_ReplyFlag != 7)
        {
          Serial.println("\n GSM_ReplyFlag is not 7 \n"); 
          
          //_tcpStatus = sendATreply("AT+CIPSTATUS\r\n", "STATE" , 4000);                             //This is commented
           Serial.print("\n count is:\n");
           Serial.println(count);
           count = count+1;
        
        /* if(count > 0)
         {
          _tcpStatus = count + 1;
         }*/
         
          /* if (sendATreply("AT+CIPSTATUS\r\n","STATE: IP INITIAL" , 4000))
            {
              _tcpStatus = 2;
            }
            else if(sendATreply("AT+CIPSTATUS\r\n","STATE: TCP CLOSED" , 4000))
            {
              _tcpStatus = 2;
            }
            else if(sendATreply("AT+CIPSTATUS\r\n","STATE: IP START" , 4000))
            {
              _tcpStatus = 3;
            }
            else if (sendATreply("AT+CIPSTATUS\r\n","STATE: IP GPRSACT" , 4000))
            {
              _tcpStatus = 4;
            }
            else if (sendATreply("AT+CIPSTATUS\r\n","STATE: IP STATUS" , 4000))
            {
              _tcpStatus = 5;
            }
            else if (sendATreply("AT+CIPSTATUS\r\n","ERROR" , 4000))
            {
              Serial.println("\n Getting ERROR \n");
            }
            else
            {
              Serial.println("\n Did not match any condition for CIPSTATUS \n");
              _tcpStatus = sendATreply("AT+CIPSTATUS\r\n", "STATE" , 4000);
              Serial.println(_tcpStatus);
            }
          */


          if (_tcpStatusPrev == _tcpStatus)
          {
            Serial.println("\n _tcpStatusPrev == _tcpStatus \n");
            tcpATerrorcount++;
            if (tcpATerrorcount >= 10)
            {
              Serial.println("\n tcpATerrorcount > = 10\n");
              tcpATerrorcount = 0;
              _tcpStatus = 7;
            }
          }
          else
          {
            Serial.println("\n ecpATerrorcount < 10 \n");
            _tcpStatusPrev = _tcpStatus;
            tcpATerrorcount = 0;
          }
        }
        
        _tcpStatusPrev = _tcpStatus;
        //Serial1.print(_tcpStatus);
        
        switch (_tcpStatus)
        {
          case 2:
            {
              Serial.println("\n In Nested Loop 2");
              _sendAT("AT+CSTT=\"airtelgprs.com\"\r\n", 5000);
              break;
            }
          case 3:
            {
              Serial.println("\n In Nested Loop 3");
              _sendAT("AT+CIICR\r\n", 5000);
              break;
            }
          case 4:
            {
              Serial.println("\n In Nested Loop 4");
              sendATreply("AT+CIFSR\r\n", ".", 4000);
              break;
            }
          case 5:
            {
              Serial.println("\n In Nested Loop 5");
              Serial.print("AT+CIPSTART=\"TCP\",\"");
              Serial.print(MQTT_HOST);
              Serial.print("\",\"");
              Serial.print(MQTT_PORT);
              if (_sendAT("\"\r\n", 5000) == 1)
              {
                Serial.println("\n CONNECTED from CASE \n");
                unsigned long PrevMillis = millis();
                unsigned long currentMillis = millis();
                
                Serial.print("\n GSM_Response is:\t");
                Serial.println(GSM_Response);
                
                while ( (GSM_Response != 4) && ((currentMillis - PrevMillis) < 20000))
                {
                  //GSM_Response = 4;                                              //This is temporarily SET (When GSM_Response = 4, it indicates either it is CONNECTED / DISCONNECTED
                  Serial.print("\n GSM_Response is:\t");
                  Serial.println(GSM_Response);
                  Serial.println("\nEntering Serial Event from Nested Loop 5\n");
                  serialEvent();              
                  //serialEvent1();
                  currentMillis = millis();
                }
               Serial.println("\n After while() \n");
              //count = count + 1;
             // _tcpStatus = 6;
              }
              else
              {
                Serial.println("\n Trying to SHUT the Connection\n");
                _sendAT("AT+CIPSHUT\r\n", 2000);
              }
              Serial.println("\n Exiting Nested Loop 5\n");
              break;
            }
          case 6:
            {
              Serial.println("\n In Nested Loop 6");
              unsigned long PrevMillis = millis();
              unsigned long currentMillis = millis();
              while ( (GSM_Response != 4) && ((currentMillis - PrevMillis) < 20000) )
              {
                //    delay(1);
                serialEvent();
                currentMillis = millis();
              }
              count = count + 1;
              break;
            }
          case 7:
            {
              Serial.println("\n In Nested Loop 7");
              sendATreply("AT+CIPSHUT\r\n", "OK", 4000) ;
              modemStatus = 0;
              _tcpStatus = 2;

              break;
            }
        }
      }
  }

}

void GSM_MQTT::_ping(void)
{

  if (pingFlag == true)
  {
    unsigned long currentMillis = millis();
    if ((currentMillis - _PingPrevMillis ) >= _KeepAliveTimeOut * 1000)
    {
      // save the last time you blinked the LED
      _PingPrevMillis = currentMillis;
      Serial.print(char(PINGREQ * 16));
      _sendLength(0);
    }
  }
}



void GSM_MQTT::_sendUTFString(char *string)
{
  int localLength = strlen(string);
  Serial.print(char(localLength / 256));
  Serial.print(char(localLength % 256));
  Serial.print(string);


  Serial1.print(char(localLength / 256));
  Serial1.print(char(localLength % 256));
  Serial1.print(string);
}




void GSM_MQTT::_sendLength(int len)
{
  bool  length_flag = false;
  while (length_flag == false)
  {
    if ((len / 128) > 0)
    {

      Serial1.print(char(len % 128 + 128));
      
      Serial.print(char(len % 128 + 128));
      len /= 128;
    }
    else
    {
      length_flag = true;
 
      Serial1.print(char(len));
      
      Serial.print(char(len));
    }
  }
}





void GSM_MQTT::connect(char *ClientIdentifier, char UserNameFlag, char PasswordFlag, char *UserName, char *Password, char CleanSession, char WillFlag, char WillQoS, char WillRetain, char *WillTopic, char *WillMessage)
{

  Serial.println("\n ATTEMPT TO CONNECT \n");
  
  ConnectionAcknowledgement = NO_ACKNOWLEDGEMENT ;
  Serial.print(char(CONNECT * 16 ));
  
  char ProtocolName[7] = "MQTT";
  //char ProtocolName[7] = "MQIsdp";
  
  int localLength = (2 + strlen(ProtocolName)) + 1 + 3 + (2 + strlen(ClientIdentifier));
  if (WillFlag != 0)
  {
    localLength = localLength + 2 + strlen(WillTopic) + 2 + strlen(WillMessage);
  }
  if (UserNameFlag != 0)
  {
    localLength = localLength + 2 + strlen(UserName);

    if (PasswordFlag != 0)
    {
      localLength = localLength + 2 + strlen(Password);
    }
  }
  _sendLength(localLength);
  _sendUTFString(ProtocolName);
  Serial.print(char(_ProtocolVersion));
  Serial.print(char(UserNameFlag * User_Name_Flag_Mask + PasswordFlag * Password_Flag_Mask + WillRetain * Will_Retain_Mask + WillQoS * Will_QoS_Scale + WillFlag * Will_Flag_Mask + CleanSession * Clean_Session_Mask));
  Serial.print(char(_KeepAliveTimeOut / 256));
  Serial.print(char(_KeepAliveTimeOut % 256));

  Serial1.print(char(_ProtocolVersion));
  Serial1.print(char(UserNameFlag * User_Name_Flag_Mask + PasswordFlag * Password_Flag_Mask + WillRetain * Will_Retain_Mask + WillQoS * Will_QoS_Scale + WillFlag * Will_Flag_Mask + CleanSession * Clean_Session_Mask));
  Serial1.print(char(_KeepAliveTimeOut / 256));
  Serial1.print(char(_KeepAliveTimeOut % 256));

  
  _sendUTFString(ClientIdentifier);
  if (WillFlag != 0)
  {
    _sendUTFString(WillTopic);
    _sendUTFString(WillMessage);
  }
  if (UserNameFlag != 0)
  {
    _sendUTFString(UserName);
    if (PasswordFlag != 0)
    {
      _sendUTFString(Password);
    }
  }
}


void GSM_MQTT::publish(char DUP, char Qos, char RETAIN, unsigned int MessageID, char *Topic, char *Message)                           //....PUBLISH
{
 
  Serial1.println(char(PUBLISH * 16 + DUP * DUP_Mask + Qos * QoS_Scale + RETAIN));

  Serial.print(char(PUBLISH * 16 + DUP * DUP_Mask + Qos * QoS_Scale + RETAIN));
  int localLength = (2 + strlen(Topic));
  if (Qos > 0)
  {
    localLength += 2;
  }
  localLength += strlen(Message);
  _sendLength(localLength);
  _sendUTFString(Topic);
  if (Qos > 0)
  {
    Serial.print(char(MessageID / 256));
    Serial.print(char(MessageID % 256));

    
    Serial1.print(char(MessageID / 256));
    Serial1.print(char(MessageID % 256));

    
  }
  Serial.print(Message);  
}



void GSM_MQTT::publishACK(unsigned int MessageID)                                              //.............Publish Acknowledgement
{
  Serial.print(char(PUBACK * 16));
  _sendLength(2);
  Serial.print(char(MessageID / 256));
  Serial.print(char(MessageID % 256));
}



void GSM_MQTT::publishREC(unsigned int MessageID)                                             //.............Publish Received
{
  Serial.print(char(PUBREC * 16));
  _sendLength(2);
  Serial.print(char(MessageID / 256));
  Serial.print(char(MessageID % 256));
}



void GSM_MQTT::publishREL(char DUP, unsigned int MessageID)                                   //...............Publish Release
{
  Serial.print(char(PUBREL * 16 + DUP * DUP_Mask + 1 * QoS_Scale));
  _sendLength(2);
  Serial.print(char(MessageID / 256));
  Serial.print(char(MessageID % 256));
}

void GSM_MQTT::publishCOMP(unsigned int MessageID)                                              //..............Publish Complete
{
  Serial.print(char(PUBCOMP * 16));
  _sendLength(2);
  Serial.print(char(MessageID / 256));
  Serial.print(char(MessageID % 256));
}



void GSM_MQTT::subscribe(char DUP, unsigned int MessageID, char *SubTopic, char SubQoS)
{
  Serial.print(char(SUBSCRIBE * 16 + DUP * DUP_Mask + 1 * QoS_Scale));
  int localLength = 2 + (2 + strlen(SubTopic)) + 1;
  _sendLength(localLength);
  Serial.print(char(MessageID / 256));
  Serial.print(char(MessageID % 256));
  _sendUTFString(SubTopic);
  Serial.print(SubQoS);
}



void GSM_MQTT::unsubscribe(char DUP, unsigned int MessageID, char *SubTopic)
{
  Serial.print(char(UNSUBSCRIBE * 16 + DUP * DUP_Mask + 1 * QoS_Scale));
  int localLength = (2 + strlen(SubTopic)) + 2;
  _sendLength(localLength);

  Serial.print(char(MessageID / 256));
  Serial.print(char(MessageID % 256));

  _sendUTFString(SubTopic);
}
void GSM_MQTT::disconnect(void)
{
  Serial.print(char(DISCONNECT * 16));
  _sendLength(0);
  pingFlag = false;
}
//Messages
const char CONNECTMessage[] PROGMEM  = {"Client request to connect to Server\r\n"};
const char CONNACKMessage[] PROGMEM  = {"Connect Acknowledgment\r\n"};
const char PUBLISHMessage[] PROGMEM  = {"Publish message\r\n"};
const char PUBACKMessage[] PROGMEM  = {"Publish Acknowledgment\r\n"};
const char PUBRECMessage[] PROGMEM  = {"Publish Received (assured delivery part 1)\r\n"};
const char PUBRELMessage[] PROGMEM  = {"Publish Release (assured delivery part 2)\r\n"};
const char PUBCOMPMessage[] PROGMEM  = {"Publish Complete (assured delivery part 3)\r\n"};
const char SUBSCRIBEMessage[] PROGMEM  = {"Client Subscribe request\r\n"};
const char SUBACKMessage[] PROGMEM  = {"Subscribe Acknowledgment\r\n"};
const char UNSUBSCRIBEMessage[] PROGMEM  = {"Client Unsubscribe request\r\n"};
const char UNSUBACKMessage[] PROGMEM  = {"Unsubscribe Acknowledgment\r\n"};
const char PINGREQMessage[] PROGMEM  = {"PING Request\r\n"};
const char PINGRESPMessage[] PROGMEM  = {"PING Response\r\n"};
const char DISCONNECTMessage[] PROGMEM  = {"Client is Disconnecting\r\n"};

void GSM_MQTT::printMessageType(uint8_t Message)
{
  switch (Message)
  {
    case CONNECT:
      {
        int k, len = strlen_P(CONNECTMessage);
        char myChar;
        for (k = 0; k < len; k++)
        {
          myChar =  pgm_read_byte_near(CONNECTMessage + k);
          Serial1.print(myChar);
        }
        break;
      }
    case CONNACK:
      {
        int k, len = strlen_P(CONNACKMessage);
        char myChar;
        for (k = 0; k < len; k++)
        {
          myChar =  pgm_read_byte_near(CONNACKMessage + k);
          Serial1.print(myChar);
        }
        break;
      }
    case PUBLISH:
      {
        int k, len = strlen_P(PUBLISHMessage);
        char myChar;
        for (k = 0; k < len; k++)
        {
          myChar =  pgm_read_byte_near(PUBLISHMessage + k);
          Serial1.print(myChar);
        }
        break;
      }
    case PUBACK:
      {
        int k, len = strlen_P(PUBACKMessage);
        char myChar;
        for (k = 0; k < len; k++)
        {
          myChar =  pgm_read_byte_near(PUBACKMessage + k);
          Serial1.print(myChar);
        }
        break;
      }
    case  PUBREC:
      {
        int k, len = strlen_P(PUBRECMessage);
        char myChar;
        for (k = 0; k < len; k++)
        {
          myChar =  pgm_read_byte_near(PUBRECMessage + k);
          Serial1.print(myChar);
        }
        break;
      }
    case PUBREL:
      {
        int k, len = strlen_P(PUBRELMessage);
        char myChar;
        for (k = 0; k < len; k++)
        {
          myChar =  pgm_read_byte_near(PUBRELMessage + k);
          Serial1.print(myChar);
        }
        break;
      }
    case PUBCOMP:
      {
        int k, len = strlen_P(PUBCOMPMessage );
        char myChar;
        for (k = 0; k < len; k++)
        {
          myChar =  pgm_read_byte_near(PUBCOMPMessage  + k);
          Serial1.print(myChar);
        }
        break;
      }
    case SUBSCRIBE:
      {
        int k, len = strlen_P(SUBSCRIBEMessage );
        char myChar;
        for (k = 0; k < len; k++)
        {
          myChar =  pgm_read_byte_near(SUBSCRIBEMessage  + k);
          Serial1.print(myChar);
        }
        break;
      }
    case SUBACK:
      {
        int k, len = strlen_P(SUBACKMessage );
        char myChar;
        for (k = 0; k < len; k++)
        {
          myChar =  pgm_read_byte_near(SUBACKMessage  + k);
          Serial1.print(myChar);
        }
        break;
      }
    case UNSUBSCRIBE:
      {
        int k, len = strlen_P(UNSUBSCRIBEMessage );
        char myChar;
        for (k = 0; k < len; k++)
        {
          myChar =  pgm_read_byte_near(UNSUBSCRIBEMessage  + k);
          Serial1.print(myChar);
        }
        break;
      }
    case UNSUBACK:
      {
        int k, len = strlen_P(UNSUBACKMessage );
        char myChar;
        for (k = 0; k < len; k++)
        {
          myChar =  pgm_read_byte_near(UNSUBACKMessage  + k);
          Serial1.print(myChar);
        }
        break;
      }
    case PINGREQ:
      {
        int k, len = strlen_P(PINGREQMessage);
        char myChar;
        for (k = 0; k < len; k++)
        {
          myChar =  pgm_read_byte_near(PINGREQMessage + k);
          Serial1.print(myChar);
        }
        break;
      }
    case PINGRESP:
      {
        int k, len = strlen_P(PINGRESPMessage);
        char myChar;
        for (k = 0; k < len; k++)
        {
          myChar =  pgm_read_byte_near(PINGRESPMessage + k);
          Serial1.print(myChar);
        }
        break;
      }
    case DISCONNECT:
      {
        int k, len = strlen_P(DISCONNECTMessage);
        char myChar;
        for (k = 0; k < len; k++)
        {
          myChar =  pgm_read_byte_near(DISCONNECTMessage + k);
          Serial1.print(myChar);
        }
        break;
      }
  }
}

//Connect Ack
const char ConnectAck0[] PROGMEM  = {"Connection Accepted\r\n"};
const char ConnectAck1[] PROGMEM  = {"Connection Refused: unacceptable protocol version\r\n"};
const char ConnectAck2[] PROGMEM  = {"Connection Refused: identifier rejected\r\n"};
const char ConnectAck3[] PROGMEM  = {"Connection Refused: server unavailable\r\n"};
const char ConnectAck4[] PROGMEM  = {"Connection Refused: bad user name or password\r\n"};
const char ConnectAck5[] PROGMEM  = {"Connection Refused: not authorized\r\n"};

void GSM_MQTT::printConnectAck(uint8_t Ack)
{
  switch (Ack)
  {
    case 0:
      {
        int k, len = strlen_P(ConnectAck0);
        char myChar;
        for (k = 0; k < len; k++)
        {
          myChar =  pgm_read_byte_near(ConnectAck0 + k);
          Serial1.print(myChar);
        }
        break;
      }
    case 1:
      {
        int k, len = strlen_P(ConnectAck1);
        char myChar;
        for (k = 0; k < len; k++)
        {
          myChar =  pgm_read_byte_near(ConnectAck1 + k);
          Serial1.print(myChar);
        }
        break;
      }
    case 2:
      {
        int k, len = strlen_P(ConnectAck2);
        char myChar;
        for (k = 0; k < len; k++)
        {
          myChar =  pgm_read_byte_near(ConnectAck2 + k);
          Serial1.print(myChar);
        }
        break;
      }
    case 3:
      {
        int k, len = strlen_P(ConnectAck3);
        char myChar;
        for (k = 0; k < len; k++)
        {
          myChar =  pgm_read_byte_near(ConnectAck3 + k);
          Serial1.print(myChar);
        }
        break;
      }
    case 4:
      {
        int k, len = strlen_P(ConnectAck4);
        char myChar;
        for (k = 0; k < len; k++)
        {
          myChar =  pgm_read_byte_near(ConnectAck4 + k);
          Serial1.print(myChar);
        }
        break;
      }
    case 5:
      {
        int k, len = strlen_P(ConnectAck5);
        char myChar;
        for (k = 0; k < len; k++)
        {
          myChar =  pgm_read_byte_near(ConnectAck5 + k);
          Serial1.print(myChar);
        }
        break;
      }
  }
}
unsigned int GSM_MQTT::_generateMessageID(void)
{
  if (_LastMessaseID < 65535)
  {
    return ++_LastMessaseID;
  }
  else
  {
    _LastMessaseID = 0;
    return _LastMessaseID;
  }
}




void GSM_MQTT::processing(void)
{
  if (TCP_Flag == false)
  {
    MQTT_Flag = false;
    _tcpInit();
  }
  _ping();
}







bool GSM_MQTT::available(void)
{
  return MQTT_Flag;
}






void serialEvent()
{

  while (Serial1.available()) 
  {
    char inChar = (char)Serial1.read();

    
    
    if (MQTT.TCP_Flag == false)
    {
      if (MQTT.index < 200)
      {
        MQTT.inputString[MQTT.index++] = inChar;
      }
      if (inChar == '\n')
      {
        MQTT.inputString[MQTT.index] = 0;
        stringComplete = true;
        Serial1.println(MQTT.inputString);

        //Serial.println(MQTT.inputString);
        
        if (strstr(MQTT.inputString, MQTT.reply) != NULL)
        {
          MQTT.GSM_ReplyFlag = 1;
          if (strstr(MQTT.inputString, " INITIAL") != 0)
          {
            Serial.println("\n FOUND INITIAL \n");
            MQTT.GSM_ReplyFlag = 2; //
          }
          else if (strstr(MQTT.inputString, " START") != 0)
          {
            MQTT.GSM_ReplyFlag = 3; //
          }
          else if (strstr(MQTT.inputString, "IP CONFIG") != 0)
          {
            _delay_us(10);
            MQTT.GSM_ReplyFlag = 4;
          }
          else if (strstr(MQTT.inputString, " GPRSACT") != 0)
          {
            Serial.println("\n GPRSACT \n");
            MQTT.GSM_ReplyFlag = 4; //
          }
          else if ((strstr(MQTT.inputString, " STATUS") != 0) || (strstr(MQTT.inputString, "TCP CLOSED") != 0))
          {
            Serial.println("\n TCP CLOSED \n");
            MQTT.GSM_ReplyFlag = 5; //
          }
          else if (strstr(MQTT.inputString, " TCP CONNECTING") != 0)
          {
            Serial.println("\n TCP CONNECTING\n");
            MQTT.GSM_ReplyFlag = 6; //
          }
          else if ((strstr(MQTT.inputString, " CONNECT OK") != 0) || (strstr(MQTT.inputString, "CONNECT FAIL") != NULL) || (strstr(MQTT.inputString, "PDP DEACT") != 0))
          {
            Serial.println("\nDisCONNECTED\n");
            MQTT.GSM_ReplyFlag = 7;
          }
        }
        else if (strstr(MQTT.inputString, "OK") != NULL)
        {
          GSM_Response = 1;
        }
        else if (strstr(MQTT.inputString, "ERROR") != NULL)
        {
          Serial.println("\n ERROR \n");
          GSM_Response = 2;
        }
        else if (strstr(MQTT.inputString, ".") != NULL)
        {
          GSM_Response = 3;
        }
        else if (strstr(MQTT.inputString, "CONNECT FAIL") != NULL)
        {
          Serial.println("\n CONNECTION FAILED \n");
          GSM_Response = 5;
        }
        else if (strstr(MQTT.inputString, "CONNECT") != NULL)
        {
          Serial.println("\n CONNECTION SUCCESSFUL");
          GSM_Response = 4;
          MQTT.TCP_Flag = true;
          Serial1.println("MQTT.TCP_Flag = True");
          MQTT.AutoConnect();
          MQTT.pingFlag = true;
          MQTT.tcpATerrorcount = 0;
        }
        else if (strstr(MQTT.inputString, "CLOSED") != NULL)
        {
          Serial.println("\n CONNECTION CLOSED \n");
          GSM_Response = 4;
          MQTT.TCP_Flag = false;
          MQTT.MQTT_Flag = false;
        }
        MQTT.index = 0;
        MQTT.inputString[0] = 0;
      }
    }
    else
    {
      uint8_t ReceivedMessageType = (inChar / 16) & 0x0F;
      uint8_t DUP = (inChar & DUP_Mask) / DUP_Mask;
      uint8_t QoS = (inChar & QoS_Mask) / QoS_Scale;
      uint8_t RETAIN = (inChar & RETAIN_Mask);
      if ((ReceivedMessageType >= CONNECT) && (ReceivedMessageType <= DISCONNECT))
      {
        bool NextLengthByte = true;
        MQTT.length = 0;
        MQTT.lengthLocal = 0;
        uint32_t multiplier=1;
        delay(2);
        char Cchar = inChar;
        while ( (NextLengthByte == true) && (MQTT.TCP_Flag == true))
        {
          if (Serial.available())
          {
            inChar = (char)Serial.read();
  /*          if (Serial.available())
          {
            inChar = (char)Serial.read();
    */        Serial1.println(inChar, DEC);
            if ((((Cchar & 0xFF) == 'C') && ((inChar & 0xFF) == 'L') && (MQTT.length == 0)) || (((Cchar & 0xFF) == '+') && ((inChar & 0xFF) == 'P') && (MQTT.length == 0)))
            {
              MQTT.index = 0;
              MQTT.inputString[MQTT.index++] = Cchar;
              MQTT.inputString[MQTT.index++] = inChar;
              MQTT.TCP_Flag = false;
              MQTT.MQTT_Flag = false;
              MQTT.pingFlag = false;
              Serial1.println("Disconnecting");
            }
            else
            {
              if ((inChar & 128) == 128)
              {
                MQTT.length += (inChar & 127) *  multiplier;
                multiplier *= 128;
                Serial1.println("More");
              }
              else
              {
                NextLengthByte = false;
                MQTT.length += (inChar & 127) *  multiplier;
                multiplier *= 128;
              }
            }
          }
        }
        MQTT.lengthLocal = MQTT.length;
        Serial1.println(MQTT.length);
        if (MQTT.TCP_Flag == true)
        {
          MQTT.printMessageType(ReceivedMessageType);
          MQTT.index = 0L;
          uint32_t a = 0;
          while ((MQTT.length-- > 0) && (Serial.available()))
          {
            MQTT.inputString[uint32_t(MQTT.index++)] = (char)Serial.read();

            delay(1);

          }
          Serial1.println(" ");
          if (ReceivedMessageType == CONNACK)
          {

            Serial.println("\nAcknowledgement Received\n");
            
            MQTT.ConnectionAcknowledgement = MQTT.inputString[0] * 256 + MQTT.inputString[1];
            if (MQTT.ConnectionAcknowledgement == 0)
            {
              MQTT.MQTT_Flag = true;
              MQTT.OnConnect();

            }

            MQTT.printConnectAck(MQTT.ConnectionAcknowledgement);
            // MQTT.OnConnect();
          }
          else if (ReceivedMessageType == PUBLISH)
          {
            uint32_t TopicLength = (MQTT.inputString[0]) * 256 + (MQTT.inputString[1]);
            Serial1.print("Topic : '");
            MQTT.PublishIndex = 0;
            for (uint32_t iter = 2; iter < TopicLength + 2; iter++)
            {
              Serial1.print(MQTT.inputString[iter]);
              MQTT.Topic[MQTT.PublishIndex++] = MQTT.inputString[iter];
            }
            MQTT.Topic[MQTT.PublishIndex] = 0;
            Serial1.print("' Message :'");
            MQTT.TopicLength = MQTT.PublishIndex;

            MQTT.PublishIndex = 0;
            uint32_t MessageSTART = TopicLength + 2UL;
            int MessageID = 0;
            if (QoS != 0)
            {
              MessageSTART += 2;
              MessageID = MQTT.inputString[TopicLength + 2UL] * 256 + MQTT.inputString[TopicLength + 3UL];
            }
            for (uint32_t iter = (MessageSTART); iter < (MQTT.lengthLocal); iter++)
            {
              Serial1.print(MQTT.inputString[iter]);
              MQTT.Message[MQTT.PublishIndex++] = MQTT.inputString[iter];
            }
            MQTT.Message[MQTT.PublishIndex] = 0;
            Serial1.println("'");
            MQTT.MessageLength = MQTT.PublishIndex;
            if (QoS == 1)
            {
              MQTT.publishACK(MessageID);
            }
            else if (QoS == 2)
            {
              MQTT.publishREC(MessageID);
            }
            MQTT.OnMessage(MQTT.Topic, MQTT.TopicLength, MQTT.Message, MQTT.MessageLength);
            MQTT.MessageFlag = true;
          }
          else if (ReceivedMessageType == PUBREC)
          {
            Serial1.print("Message ID :");
            MQTT.publishREL(0, MQTT.inputString[0] * 256 + MQTT.inputString[1]) ;
            Serial1.println(MQTT.inputString[0] * 256 + MQTT.inputString[1]) ;

          }
          else if (ReceivedMessageType == PUBREL)
          {
            Serial1.print("Message ID :");
            MQTT.publishCOMP(MQTT.inputString[0] * 256 + MQTT.inputString[1]) ;
            Serial1.println(MQTT.inputString[0] * 256 + MQTT.inputString[1]) ;

          }
          else if ((ReceivedMessageType == PUBACK) || (ReceivedMessageType == PUBCOMP) || (ReceivedMessageType == SUBACK) || (ReceivedMessageType == UNSUBACK))
          {
            Serial1.print("Message ID :");
            Serial1.println(MQTT.inputString[0] * 256 + MQTT.inputString[1]) ;
          }
          else if (ReceivedMessageType == PINGREQ)
          {
            MQTT.TCP_Flag = false;
            MQTT.pingFlag = false;
            Serial1.println("Disconnecting");
            MQTT.sendATreply("AT+CIPSHUT\r\n", ".", 4000) ;
            MQTT.modemStatus = 0;
          }
        }
      }
      else if ((inChar = 13) || (inChar == 10))
      {
      }
      else
      {
        Serial1.print("Received :Unknown Message Type :");
        Serial1.println(inChar);
      }
    }
  }
}//serialEvent()
















void serialEvent1()
{

  while (Serial1.available())
  {
  Serial.println("\nIn serialEvent1 \n");

  if(GSM_Response == 4)
  {
          Serial.println("\n CONNECTION SUCCESSFUL");
          GSM_Response = 4;
          MQTT.TCP_Flag = true;
          Serial1.println("MQTT.TCP_Flag = True");
          MQTT.AutoConnect();
          MQTT.pingFlag = true;
          MQTT.tcpATerrorcount = 0;
   }

   char inChar;
    /* if(GSM_Response == 4))
        {
          Serial.println("\n CONNECTION CLOSED \n");
          GSM_Response = 4;
          MQTT.TCP_Flag = false;
          MQTT.MQTT_Flag = false;
        }
        MQTT.index = 0;
        MQTT.inputString[0] = 0;
      }
    } */
    
    if(MQTT.TCP_Flag = true)
    {
      uint8_t ReceivedMessageType = (inChar / 16) & 0x0F;
      uint8_t DUP = (inChar & DUP_Mask) / DUP_Mask;
      uint8_t QoS = (inChar & QoS_Mask) / QoS_Scale;
      uint8_t RETAIN = (inChar & RETAIN_Mask);
      if ((ReceivedMessageType >= CONNECT) && (ReceivedMessageType <= DISCONNECT))
      {
        bool NextLengthByte = true;
        MQTT.length = 0;
        MQTT.lengthLocal = 0;
        uint32_t multiplier=1;
        delay(2);
        char Cchar = inChar;
        while ( (NextLengthByte == true) && (MQTT.TCP_Flag == true))
        {
          if (Serial1.available())
          {
            inChar = (char)Serial1.read();

            //Serial1.println(inChar, DEC);
            if ((((Cchar & 0xFF) == 'C') && ((inChar & 0xFF) == 'L') && (MQTT.length == 0)) || (((Cchar & 0xFF) == '+') && ((inChar & 0xFF) == 'P') && (MQTT.length == 0)))
            {
              MQTT.index = 0;
              MQTT.inputString[MQTT.index++] = Cchar;
              MQTT.inputString[MQTT.index++] = inChar;
              MQTT.TCP_Flag = false;
              MQTT.MQTT_Flag = false;
              MQTT.pingFlag = false;
              Serial.println("Disconnecting");
            }
            else
            {
              if ((inChar & 128) == 128)
              {
                MQTT.length += (inChar & 127) *  multiplier;
                multiplier *= 128;
                Serial.println("More");
              }
              else
              {
                NextLengthByte = false;
                MQTT.length += (inChar & 127) *  multiplier;
                multiplier *= 128;
              }
            }
          }
        }
        MQTT.lengthLocal = MQTT.length;
        Serial1.println(MQTT.length);
        if (MQTT.TCP_Flag == true)
        {
          MQTT.printMessageType(ReceivedMessageType);
          MQTT.index = 0L;
          uint32_t a = 0;
          while ((MQTT.length-- > 0) && (Serial1.available()))
          {
            MQTT.inputString[uint32_t(MQTT.index++)] = (char)Serial1.read();

            delay(1);

          }
          Serial.println(" ");
          if (ReceivedMessageType == CONNACK)
          {

            Serial.println("\nAcknowledgement Received\n");
            
            MQTT.ConnectionAcknowledgement = MQTT.inputString[0] * 256 + MQTT.inputString[1];
            if (MQTT.ConnectionAcknowledgement == 0)
            {
              MQTT.MQTT_Flag = true;
              MQTT.OnConnect();

            }

            MQTT.printConnectAck(MQTT.ConnectionAcknowledgement);
            // MQTT.OnConnect();
          }
          else if (ReceivedMessageType == PUBLISH)
          {
            uint32_t TopicLength = (MQTT.inputString[0]) * 256 + (MQTT.inputString[1]);
            Serial1.print("Topic : '");
            MQTT.PublishIndex = 0;
            for (uint32_t iter = 2; iter < TopicLength + 2; iter++)
            {
              Serial1.print(MQTT.inputString[iter]);
              MQTT.Topic[MQTT.PublishIndex++] = MQTT.inputString[iter];
            }
            MQTT.Topic[MQTT.PublishIndex] = 0;
            Serial1.print("' Message :'");
            MQTT.TopicLength = MQTT.PublishIndex;

            MQTT.PublishIndex = 0;
            uint32_t MessageSTART = TopicLength + 2UL;
            int MessageID = 0;
            if (QoS != 0)
            {
              MessageSTART += 2;
              MessageID = MQTT.inputString[TopicLength + 2UL] * 256 + MQTT.inputString[TopicLength + 3UL];
            }
            for (uint32_t iter = (MessageSTART); iter < (MQTT.lengthLocal); iter++)
            {
              Serial1.print(MQTT.inputString[iter]);
              MQTT.Message[MQTT.PublishIndex++] = MQTT.inputString[iter];
            }
            MQTT.Message[MQTT.PublishIndex] = 0;
            Serial1.println("'");
            MQTT.MessageLength = MQTT.PublishIndex;
            if (QoS == 1)
            {
              MQTT.publishACK(MessageID);
            }
            else if (QoS == 2)
            {
              MQTT.publishREC(MessageID);
            }
            MQTT.OnMessage(MQTT.Topic, MQTT.TopicLength, MQTT.Message, MQTT.MessageLength);
            MQTT.MessageFlag = true;
          }
          else if (ReceivedMessageType == PUBREC)
          {
            Serial1.print("Message ID :");
            MQTT.publishREL(0, MQTT.inputString[0] * 256 + MQTT.inputString[1]) ;
            Serial1.println(MQTT.inputString[0] * 256 + MQTT.inputString[1]) ;

          }
          else if (ReceivedMessageType == PUBREL)
          {
            Serial1.print("Message ID :");
            MQTT.publishCOMP(MQTT.inputString[0] * 256 + MQTT.inputString[1]) ;
            Serial1.println(MQTT.inputString[0] * 256 + MQTT.inputString[1]) ;

          }
          else if ((ReceivedMessageType == PUBACK) || (ReceivedMessageType == PUBCOMP) || (ReceivedMessageType == SUBACK) || (ReceivedMessageType == UNSUBACK))
          {
            Serial1.print("Message ID :");
            Serial1.println(MQTT.inputString[0] * 256 + MQTT.inputString[1]) ;
          }
          else if (ReceivedMessageType == PINGREQ)
          {
            MQTT.TCP_Flag = false;
            MQTT.pingFlag = false;
            Serial1.println("Disconnecting");
            MQTT.sendATreply("AT+CIPSHUT\r\n", ".", 4000) ;
            MQTT.modemStatus = 0;
          }
        }
      }
      else if ((inChar = 13) || (inChar == 10))
      {
      }
      else
      {
        Serial1.print("Received :Unknown Message Type :");
        Serial1.println(inChar);
      }
    }
  }
}//serialEvent1()





