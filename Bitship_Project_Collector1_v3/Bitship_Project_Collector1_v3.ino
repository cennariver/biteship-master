// include the library code:
#include <LiquidCrystal.h>
#include <ArduinoJson.h>
#include "WiFiEsp.h"


//Delay
unsigned long m_ulStartMillis;  //some global variables available anywhere in the program
unsigned long m_ulCurrentMillis;
const unsigned long PERIOD_TIME = 6000;


//WIFI
//const char m_chaSsid[] = "tamsos blok HG 25A";            // your network m_chaSsid (name)
//const char m_chaPass[] = "qwerty123456";        // your network m_chPassword
//const char m_chaSsid[] = "c7";            // your network m_chaSsid (name)
//const char m_chaPass[] = "da645591";        // your network m_chPassword
const char m_chaSsid[] = "Warehouse Biteship";            // your network m_chaSsid (name)
const char m_chaPass[] = "Bismilahlancar";        // your network m_chPassword
//const char m_chaSsid[] = "Diskum_723";            // your network m_chaSsid (name)
//const char m_chaPass[] = "19283746abcd";


int m_iStatus = WL_IDLE_STATUS;     // the Wifi radio's status
//char m_chaServer[] = "061e-182-2-170-99.ngrok.io";
//String m_strHost = "061e-182-2-170-99.ngrok.io";
char m_chaServer[] = "192.168.0.6";
String m_strHost = "192.168.0.6";
String m_strAtResponse;
unsigned long m_ulLastConnectionTime = 0;         // last time you connected to the server, in milliseconds
const unsigned long POSTING_INTERVAL = 10000L; // delay between updates, in milliseconds
const unsigned long CONNECT_TIMEOUT = 3000;

// Initialize the Ethernet client object
WiFiEspClient m_oClient;

//LCD Intialization
const int RTU_TOTAL = 11; // {null, RTU1, RTU2, RTU3, RTU4, RTU5, RTU6, RTU7, RTU8, RTU9, RTU10}
const int PIN_D4 = 25;
const int PIN_D5 = 24;
const int PIN_D6 = 23;
const int PIN_D7 = 22;
//Pin : rs,en,PIN_D4,PIN_D5,PIN_D6,PIN_D7
LiquidCrystal m_oRtuLcd1(27, 26, PIN_D4, PIN_D5, PIN_D6, PIN_D7);
LiquidCrystal m_oRtuLcd2(29, 28, PIN_D4, PIN_D5, PIN_D6, PIN_D7);
LiquidCrystal m_oRtuLcd3(31, 30, PIN_D4, PIN_D5, PIN_D6, PIN_D7);
LiquidCrystal m_oRtuLcd4(33, 32, PIN_D4, PIN_D5, PIN_D6, PIN_D7);
LiquidCrystal m_oRtuLcd5(35, 34, PIN_D4, PIN_D5, PIN_D6, PIN_D7);
LiquidCrystal m_oRtuLcd6(37, 36, PIN_D4, PIN_D5, PIN_D6, PIN_D7);
LiquidCrystal m_oRtuLcd7(39, 38, PIN_D4, PIN_D5, PIN_D6, PIN_D7);
LiquidCrystal m_oRtuLcd8(41, 40, PIN_D4, PIN_D5, PIN_D6, PIN_D7);
LiquidCrystal m_oRtuLcd9(43, 42, PIN_D4, PIN_D5, PIN_D6, PIN_D7);
LiquidCrystal m_oRtuLcd10(45, 44, PIN_D4, PIN_D5, PIN_D6, PIN_D7);
//Pin for LED (Pin LED RU1, Pin LED RU2,...)
const int PIN_LED[RTU_TOTAL] = {0, 47, 46, 50, 52, 17, 16, 15, 14, 48, 49};
//Pin for Push Button
const int PIN_BUTTON[RTU_TOTAL] = {0, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};
//Button Virtual Latch
bool m_zaButtonState[RTU_TOTAL] = {false, false, false, false, false, false, false, false, false, false, false};
//API List
const String API_GET_CURRENT_DATA = "/get-current-data-by-collector/1";
const String API_GET_TRANSACTION = "/get-transaction-by-collector/1";
const String API_TRANSACTION_CONFIRM = "/confirm-on-process-by-collector/1";
const String API_PICKING_CONFIRM = "/confirm-done?m_straBinId=";
//RU Var Init
String m_straSkuName[RTU_TOTAL];
int m_iaSkuQty[RTU_TOTAL];
//RU Transaction Data
bool m_zTransactionStatus = false;
String m_strTransactionId;
String m_strTransactionExecution;
bool m_zaBinStatus[RTU_TOTAL]; // transaction status exist or not
String m_straBinId[RTU_TOTAL];
String m_zaActionCode[RTU_TOTAL];
int m_iaActionQty[RTU_TOTAL];
bool m_zTransactionUpdated = false;
bool m_zConfirmationStatus = false;

//API Data
String m_strRawData;
String m_strRawStockData;
String m_strRawTransactionData;
String m_strRawTransactionConfirmation;
String m_strRawPickingsConfirmation;
//Receive Serial
boolean m_zNewData = false;
bool m_zFirstRun = false;
const int RECEIVED_CHAR_LENGTH = 1500;
char m_chaReceivedChars[RECEIVED_CHAR_LENGTH];
/* m_zaWaitSerial[0] = get current stock data,
 * m_zaWaitSerial[1] = transaction data,
 * m_zaWaitSerialCommand[2] = transaction confirmation data
 */
bool m_zaWaitSerial[10];
/*
 *  m_zaWaitSerialCommand[1] = transaction cmd,
 *  m_zaWaitSerialCommand[2] = transaction confirmation cmd
 */
bool m_zaWaitSerialCommand[10];



void setup() {

  //Setting UP Pin Mode
  for (int i = 1; i <= 10; i++) {
    //Activate Pull UP for Push Button, default: high, press: low
    pinMode(PIN_BUTTON[i], INPUT_PULLUP);
    Serial.println(PIN_BUTTON[i]);
    //Setting Output for LED Relay
    pinMode(PIN_LED[i], OUTPUT);
  }
  m_ulStartMillis = millis();

  //Initialize LCD
  m_oRtuLcd1.begin(16, 2);
  m_oRtuLcd2.begin(16, 2);
  m_oRtuLcd3.begin(16, 2);
  m_oRtuLcd4.begin(16, 2);
  m_oRtuLcd5.begin(16, 2);
  m_oRtuLcd6.begin(16, 2);
  m_oRtuLcd7.begin(16, 2);
  m_oRtuLcd8.begin(16, 2);
  m_oRtuLcd9.begin(16, 2);
  m_oRtuLcd10.begin(16, 2);

  //Wait For Running Code
  delay(100);
  LCDprint(m_oRtuLcd1, "Intialization", "RU-1 Please Wait");
  LCDprint(m_oRtuLcd2, "Intialization", "RU-2 Please Wait");
  LCDprint(m_oRtuLcd3, "Intialization", "RU-3 Please Wait");
  LCDprint(m_oRtuLcd4, "Intialization", "RU-4 Please Wait");
  LCDprint(m_oRtuLcd5, "Intialization", "RU-5 Please Wait");
  LCDprint(m_oRtuLcd6, "Intialization", "RU-6 Please Wait");
  LCDprint(m_oRtuLcd7, "Intialization", "RU-7 Please Wait");
  LCDprint(m_oRtuLcd8, "Intialization", "RU-8 Please Wait");
  LCDprint(m_oRtuLcd9, "Intialization", "RU-9 Please Wait");
  LCDprint(m_oRtuLcd10, "Intialization", "RU-10 Please Wait");

  //Initate Serial Communication

  //For ESP01
  Serial1.begin(115200);
  Serial1.setTimeout(5000);
  WiFi.init(&Serial1);

  //For Arduino Serial
  Serial.begin(115200);

  //Clear Buffer
  clearBuffer();

  //Initiate ESP8266 Startup
  connectWifi();

  //Wait For Running Code
  delay(1000);
}

void loop() {
  m_ulCurrentMillis = millis();

  //Run 1st Command
  if (m_zFirstRun == false) {
    httpGetRequest(m_strHost, API_GET_CURRENT_DATA);
    m_zaWaitSerial[0] = true;
    m_zFirstRun = true;
  }

  //Wait For 1st Initial Data
  if (m_zaWaitSerial[0] == true) {
    m_strRawStockData = recvWithStartEndMarkers('<', '>');

    if (m_strRawStockData != "") {
      getInitialData();
      Serial.println(m_strRawStockData);
      m_zaWaitSerial[0] = false;
      m_zaWaitSerialCommand[1] = true;
      clearBuffer();
      for (int i = 0; i <= RECEIVED_CHAR_LENGTH; i++) {
        m_chaReceivedChars[i] = NULL;
      }
    }
  }

  // TODO try with else only
  if (m_zaWaitSerial[0] == false) {
    //Listen For New Transaction
    listenTransaction();
    //Check Transaction Status
    if (m_zTransactionStatus == true) {
      //Check Push Button For Deactivate Transaction
      checkPushButton();
      //Deactive Transaction Status when PBs are pressed
      deactivateTransaction();
    }
  }

  //Loop Sampling Time
  delay(1);
}

String recvWithStartEndMarkers(char p_chStartMarker, char p_chEndMarker) {
  static boolean l_zRecvInProgress = false;
  static int l_iNdx = 0;
  char l_chRc;

  while (m_oClient.available() > 0 && m_zNewData == false) {
    l_chRc = m_oClient.read();

    if (l_zRecvInProgress == true) {
      if (l_chRc != p_chEndMarker) {
        m_chaReceivedChars[l_iNdx] = l_chRc;
        l_iNdx++;
        if (l_iNdx >= RECEIVED_CHAR_LENGTH) {
          l_iNdx = RECEIVED_CHAR_LENGTH - 1;
        }
      }
      else {
        m_chaReceivedChars[l_iNdx] = '\0'; // terminate the string
        l_zRecvInProgress = false;
        l_iNdx = 0;
        m_zNewData = true;
      }
    }

    else if (l_chRc == p_chStartMarker) {
        l_zRecvInProgress = true;
      }
    }

    if (m_zNewData == true) {
    Serial.println(m_chaReceivedChars);
    m_zNewData = false;
  }
  return m_chaReceivedChars;
}

//Clear Serial Buffer
void clearBuffer() {
  //Clear Buffer
  Serial1.flush();
  Serial.flush();

  while (Serial1.available() || Serial.available()) {
    Serial.read();
    Serial1.read();
  }
}

//Connect to your wifi network
void connectWifi() {
  // attempt to connect to WiFi network
  while ( m_iStatus != WL_CONNECTED) {
    Serial.print("Attempting to connect to WPA SSID: ");
    Serial.println(m_chaSsid);

    // Connect to WPA/WPA2 network
    m_iStatus = WiFi.begin(m_chaSsid, m_chaPass);
  }

  Serial.println("You're connected to the network");
}

String httpGetRequest (String p_strHostAddress, String p_strQuery) {
  //Close All Socket
  m_oClient.stop();

  // if there's a successful connection
  if (m_oClient.connect(m_chaServer, CONNECT_TIMEOUT)) {
    Serial.println("Connecting...");
    // send the HTTP GET request
    m_oClient.println(("GET " + p_strQuery + " HTTP/1.1"));
    Serial.println(p_strQuery);
    //m_oClient.println(F("GET /get-current-data-by-collector/1 HTTP/1.1"));
    //m_oClient.println(F("Host: 83f3-182-253-87-219.ngrok.io"));
    m_oClient.println(("Host: " + p_strHostAddress));
    m_oClient.println("Connection: close");
    m_oClient.println();
  }
  else {
    // if you couldn't make a connection
    Serial.println("Connection failed");
  }

  return m_strRawData;
}

//Parsing CurrentData
void JsonParsing_CurrentData() {
  StaticJsonDocument<600> l_oDoc;
  DeserializationError l_oError = deserializeJson(l_oDoc, m_strRawStockData);

  if (l_oError) {
    Serial.println("Error Parsing Json Current Data");
  }

  //Status Data
  bool l_zStatus = l_oDoc["success"];

  //Function Code
  String l_strMessage = l_oDoc["message"];

  //Data Object (10 RU , SKU Name, Bin ID,
  JsonObject l_oData = l_oDoc["data"];
  JsonArray l_oArrayData = l_oDoc["data"].as<JsonArray>();

  //Parsing Array Data For 10 SKU
  for (int i = 0; i <= RTU_TOTAL; i++) {
    JsonObject l_oSku = l_oArrayData[i];
    String l_strDeviceId = (const char*)l_oSku["device_id"];
    int l_iBinId = l_strDeviceId.substring(3).toInt();
    String l_strSkuName = (const char*)l_oSku["sku_name"];
    int l_iSkuQty = (int)l_oSku["quantity"];
    m_straSkuName[l_iBinId] = l_strSkuName;
    m_iaSkuQty[l_iBinId] = l_iSkuQty;
  }
}

//Parsing CurrentData
bool JsonParsing_TransactionData() {
  bool l_zStatus = false;
  StaticJsonDocument<600> l_oParsedData;
  DeserializationError l_oError = deserializeJson(l_oParsedData, m_strRawTransactionData);

  if (l_oError) {
    Serial.println("Error Parsing Json Transaction Data");
  }
  else { //Parsing the Data And Move to Global Var
    //Transaction_Status
    l_zStatus = l_oParsedData["success"];

    if (l_zStatus == true) {
      Serial.print("trans Stat : "); Serial.println(l_zStatus);
      //Transaction_id
      m_strTransactionId = (const char*)l_oParsedData["m_strTransactionId"];
      //Data Array ( x Transaction Bin )
      JsonArray l_oaArrayTransaction = l_oParsedData["data"].as<JsonArray>();
      //Parsing Array Transaction Data For x SKU

      for (int i = 0; i <= RTU_TOTAL; i++) {
        JsonObject l_oJsonTransactionData = l_oaArrayTransaction[i];
        //Get Local Var
        String l_strBinId = (const char*)l_oJsonTransactionData["device_id"];
        String l_action_code = (const char*)l_oJsonTransactionData["action"];
        int l_iActionQty = (int)l_oJsonTransactionData["quantity"];
        //Decode bin ID
        int l_iBinId2 = l_strBinId.substring(3).toInt();
        // int ll_bin_id = l_oaJsonTransactionData["m_straBinId"];
        //Move Global Array
        m_zaBinStatus[l_iBinId2] = true;
        m_straBinId[l_iBinId2] = l_strBinId;
        m_zaActionCode[l_iBinId2] = l_action_code;
        m_iaActionQty[l_iBinId2] = l_iActionQty;
      }
    }
    return l_zStatus;
  }
}

//Parsing CurrentData
void JsonParsing_TransactionConfirmation() {
  StaticJsonDocument<300> l_oParsedData;
  DeserializationError l_oError = deserializeJson(l_oParsedData, m_strRawTransactionConfirmation);

  if (l_oError) {
    Serial.println("Error Parsing Transaction Confirmation");
  }
  //Parsing the Data And Move to Global Var
  else {
    //Transaction_Status
    JsonObject l_oJsonTransactionData = l_oParsedData["data"];
    m_strTransactionExecution = (const char*)l_oJsonTransactionData["status"];
  }
}

void JsonParsing_PickingsConfirmation(int p_iBinIds) {
  StaticJsonDocument<400> l_oParsedData;
  DeserializationError l_oError = deserializeJson(l_oParsedData, m_strRawPickingsConfirmation);

  if (l_oError) {
    Serial.println("Error Parsing Pickings Confirmation");
    m_zConfirmationStatus = false;
  }
  else { //Parsing the Data And Move to Global Var
    //Transaction_Status
    bool transStatus = l_oParsedData["success"];
    m_zConfirmationStatus = transStatus;
    if (transStatus == true) {
      JsonObject l_oJsonTransactionData = l_oParsedData["data"];
      m_zaBinStatus[p_iBinIds] = false;
      //        m_straSkuName[p_iBinIds]=(const char*)l_oaJsonTransactionData["sku_name"];
      m_iaSkuQty[p_iBinIds] = (int)l_oJsonTransactionData["quantity"];
      Serial.println(m_zaBinStatus[p_iBinIds]);
      //        Serial.println(m_straSkuName[p_iBinIds]);
    }
    else {
      //Do Nothing
    }
  }
}

//Function Print Single LCD
void LCDprint(LiquidCrystal p_oLcd, String p_strFirstRow, String p_strSecondRow) {
  // TODO try lcd.begin
  p_oLcd.clear();
  p_oLcd.setCursor(0, 0);
  p_oLcd.print(p_strFirstRow);
  p_oLcd.setCursor(0, 1);
  p_oLcd.print(p_strSecondRow);
}

//Confirmatin Status
void LCDTransactionUpdate (LiquidCrystal p_oLcd, String p_strAction, String p_strQuantity) {
  // (note: line 1 is the second row, since counting begins with 0):
  // TODO try lcd.begin
  p_oLcd.clear();
  p_oLcd.setCursor(0, 0);
  p_oLcd.print("Action : " + p_strAction);
  p_oLcd.setCursor(0, 1);
  p_oLcd.print("Quantity : " + p_strQuantity);
}

//LCDIdle
void LCDTransactionStockUpdate (LiquidCrystal p_oLcd, String p_strSku, String p_strQuantity) {
  // (note: line 1 is the second row, since counting begins with 0):
  // TODO try lcd.begin
  p_oLcd.clear();
  p_oLcd.setCursor(0, 0);
  p_oLcd.print("SKU : " + p_strSku);
  p_oLcd.setCursor(0, 1);
  p_oLcd.print("Quantity : " + p_strQuantity);
}

//Update All RU LCD Function Custom Text
void LCDAllUpdatesCustom(String p_strLine1, String p_strLine2) {
  //Print to 10 RU
  LCDprint(m_oRtuLcd1, p_strLine1, p_strLine2);
  LCDprint(m_oRtuLcd2, p_strLine1, p_strLine2);
  LCDprint(m_oRtuLcd3, p_strLine1, p_strLine2);
  LCDprint(m_oRtuLcd4, p_strLine1, p_strLine2);
  LCDprint(m_oRtuLcd5, p_strLine1, p_strLine2);
  LCDprint(m_oRtuLcd6, p_strLine1, p_strLine2);
  LCDprint(m_oRtuLcd7, p_strLine1, p_strLine2);
  LCDprint(m_oRtuLcd8, p_strLine1, p_strLine2);
  LCDprint(m_oRtuLcd9, p_strLine1, p_strLine2);
  LCDprint(m_oRtuLcd10, p_strLine1, p_strLine2);
}

//Function LCD Update ALL SKU & Stock Data
void LCDAllUpdatesStock() {
  //Print Stock SKU to 10 RU
  LCDTransactionStockUpdate(m_oRtuLcd1, m_straSkuName[1], (String)m_iaSkuQty[1]);
  LCDTransactionStockUpdate(m_oRtuLcd2, m_straSkuName[2], (String)m_iaSkuQty[2]);
  LCDTransactionStockUpdate(m_oRtuLcd3, m_straSkuName[3], (String)m_iaSkuQty[3]);
  LCDTransactionStockUpdate(m_oRtuLcd4, m_straSkuName[4], (String)m_iaSkuQty[4]);
  LCDTransactionStockUpdate(m_oRtuLcd5, m_straSkuName[5], (String)m_iaSkuQty[5]);
  LCDTransactionStockUpdate(m_oRtuLcd6, m_straSkuName[6], (String)m_iaSkuQty[6]);
  LCDTransactionStockUpdate(m_oRtuLcd7, m_straSkuName[7], (String)m_iaSkuQty[7]);
  LCDTransactionStockUpdate(m_oRtuLcd8, m_straSkuName[8], (String)m_iaSkuQty[8]);
  LCDTransactionStockUpdate(m_oRtuLcd9, m_straSkuName[9], (String)m_iaSkuQty[9]);
  LCDTransactionStockUpdate(m_oRtuLcd10, m_straSkuName[10], (String)m_iaSkuQty[10]);
}

//Function Pickings
void activateTransaction() {
  //Turn on LED
  for (int i = 1; i <= 10; i++) {
    if (m_zaBinStatus[i] == true) {
      //LCD Update
      if (i == 1) {
        LCDTransactionUpdate(m_oRtuLcd1, m_zaActionCode[i], String(m_iaActionQty[i]));
      }
      if (i == 2) {
        LCDTransactionUpdate(m_oRtuLcd2, m_zaActionCode[i], String(m_iaActionQty[i]));
      }
      if (i == 3) {
        LCDTransactionUpdate(m_oRtuLcd3, m_zaActionCode[i], String(m_iaActionQty[i]));
      }
      if (i == 4) {
        LCDTransactionUpdate(m_oRtuLcd4, m_zaActionCode[i], String(m_iaActionQty[i]));
      }
      if (i == 5) {
        LCDTransactionUpdate(m_oRtuLcd5, m_zaActionCode[i], String(m_iaActionQty[i]));
      }
      if (i == 6) {
        LCDTransactionUpdate(m_oRtuLcd6, m_zaActionCode[i], String(m_iaActionQty[i]));
      }
      if (i == 7) {
        LCDTransactionUpdate(m_oRtuLcd7, m_zaActionCode[i], String(m_iaActionQty[i]));
      }
      if (i == 8) {
        LCDTransactionUpdate(m_oRtuLcd8, m_zaActionCode[i], String(m_iaActionQty[i]));
      }
      if (i == 9) {
        LCDTransactionUpdate(m_oRtuLcd9, m_zaActionCode[i], String(m_iaActionQty[i]));
      }
      if (i == 10) {
        LCDTransactionUpdate(m_oRtuLcd10, m_zaActionCode[i], String(m_iaActionQty[i]));
      }
      delay(50);
      digitalWrite(PIN_LED[i], HIGH); //LED ON

    }
    else {
      digitalWrite(PIN_LED[i], LOW); //LED OFF
    }
  }
}

void deactivateTransaction() {
  for (int i = 1; i <= 10; i++) {
    if (m_zaButtonState[i] == true) {
      // Serial.println("HIT BUTTON : " + String(i));
      if (m_zaWaitSerial[3] != true) {
        m_zaWaitSerialCommand[3] = true;
      }

      if (m_zaWaitSerialCommand[3] == true) {
        clearBuffer();
        for (int i = 0; i <= RECEIVED_CHAR_LENGTH; i++) {
          m_chaReceivedChars[i] = NULL;
        }
        //Hit API
        // httpGetRequest(m_strHost, "/confirm-done?m_straBinId="+String(i)+"&m_strTransactionId="+m_strTransactionId);
        httpGetRequest(m_strHost, "/confirm-done?m_strTransactionId=" + m_strTransactionId + "&device_id=" + m_straBinId[i]);

        m_zaWaitSerial[3] = true;
        m_zaWaitSerialCommand[3] = false;
      }

      if (m_zaWaitSerial[3] == true) {
        //Get Serial Data
        m_strRawPickingsConfirmation = recvWithStartEndMarkers('<', '>');
        if (m_strRawPickingsConfirmation != "") {
          //Parse API
          JsonParsing_PickingsConfirmation(i);
          //Turn Off LED & Update LCD
          if (m_zaBinStatus[i] == false) {
            //Turn Off LED

            //delay(50);
            switch (i) {
              case 1:
                LCDTransactionStockUpdate(m_oRtuLcd1, m_straSkuName[i], String(m_iaSkuQty[i]));
                break;
              case 2:
                LCDTransactionStockUpdate(m_oRtuLcd2, m_straSkuName[i], String(m_iaSkuQty[i]));
                break;
              case 3:
                LCDTransactionStockUpdate(m_oRtuLcd3, m_straSkuName[i], String(m_iaSkuQty[i]));
                break;
              case 4:
                LCDTransactionStockUpdate(m_oRtuLcd4, m_straSkuName[i], String(m_iaSkuQty[i]));
                break;
              case 5:
                LCDTransactionStockUpdate(m_oRtuLcd5, m_straSkuName[i], String(m_iaSkuQty[i]));
                break;
              case 6:
                LCDTransactionStockUpdate(m_oRtuLcd6, m_straSkuName[i], String(m_iaSkuQty[i]));
                break;
              case 7:
                LCDTransactionStockUpdate(m_oRtuLcd7, m_straSkuName[i], String(m_iaSkuQty[i]));
                break;
              case 8:
                LCDTransactionStockUpdate(m_oRtuLcd8, m_straSkuName[i], String(m_iaSkuQty[i]));
                break;
              case 9:
                LCDTransactionStockUpdate(m_oRtuLcd9, m_straSkuName[i], String(m_iaSkuQty[i]));
                break;
              case 10:
                LCDTransactionStockUpdate(m_oRtuLcd10, m_straSkuName[i], String(m_iaSkuQty[i]));
                break;
              default:
                // statements
                break;
            }
            delay(50);
            digitalWrite(PIN_LED[i], LOW);
          }
          m_zaButtonState[i] = false;
          m_zaWaitSerial[3] = false;
        }
      }
      i = RTU_TOTAL;
    }
  }
}

//Read Push Button
bool buttonWasPressed(int p_iPinButton) {
  bool l_zStatusButton = false;

  if (digitalRead(p_iPinButton) == LOW) {
    l_zStatusButton = true;
  } else {
    l_zStatusButton = false;
  }

  return l_zStatusButton;
}

//Complete Transaction Request Function
void checkPushButton() {
  //Serial.println("Button Pressed");
  for (int i = 1; i <= 10; i++) {
  //Check Button are Pressed or Not
    if (buttonWasPressed(PIN_BUTTON[i]) && (m_zaBinStatus[i] == true)) {
      m_zaButtonState[i] = true;
      Serial.println("Button Was Pressed : " + String(i));
      Serial.println(m_zaButtonState[i]);
      // digitalWrite(PIN_LED[i],!digitalRead(PIN_BUTTON[i]));
    }
  }
}

//listen Transaction Reuqest Function
void listenTransaction() {
  //Check Active transaction
  int l_iTransactionTotal = 0;

  for (int i = 1; i <= 10; i++) {
    // check transaction for each RU
    if (m_zaBinStatus[i] == true) {
      l_iTransactionTotal++;
    }

    // if no transaction or transaction updated for all RUs then reset all values
    if (i == 10 && l_iTransactionTotal == 0 && m_zTransactionUpdated == true) {
      clearBuffer();
      for (int i = 0; i <= RECEIVED_CHAR_LENGTH; i++) {
        m_chaReceivedChars[i] = NULL;
      }
      m_strRawData = "";
      m_strRawStockData = "";
      m_strRawTransactionData = "";
      m_strRawTransactionConfirmation = "";
      m_strRawPickingsConfirmation = "";
      m_strTransactionExecution = "";
      m_zTransactionStatus = false;
      m_strTransactionId = "";
      m_zTransactionUpdated = false;
      m_zaWaitSerialCommand[1] = true;
      m_zaWaitSerialCommand[2] = false;
      m_zaWaitSerialCommand[3] = false;
      m_zaWaitSerial[1] = false;
      m_zaWaitSerial[2] = false;
      m_zaWaitSerial[3] = false;
    }
  }

  //If Already Transaction, Confirm the transaction
  if (m_zTransactionStatus == false) {

    //Request Transaction Data
    if (m_zaWaitSerialCommand[1] == true && (m_ulCurrentMillis - m_ulStartMillis >= PERIOD_TIME)) {
      Serial.println(API_GET_TRANSACTION);
      httpGetRequest(m_strHost, API_GET_TRANSACTION);
      m_zaWaitSerialCommand[1] = false;
      m_zaWaitSerial[1] = true;
      m_ulStartMillis = m_ulCurrentMillis;
    }

    if (m_zaWaitSerial[1] == true) {
      m_strRawTransactionData = recvWithStartEndMarkers('<', '>');

      if (m_strRawTransactionData != "") {
        Serial.println(m_strRawTransactionData);
        //Parsing raw Transaction Json Data
        m_zTransactionStatus = JsonParsing_TransactionData();
        if (m_zTransactionStatus == false) {
          m_zaWaitSerialCommand[1] = true;
          m_zaWaitSerial[1] = false;
        } else {
          m_zaWaitSerial[1] = false;
          m_zaWaitSerialCommand[2] = true;
          Serial.print("stat :"); Serial.println(m_zTransactionStatus);
          Serial.print("id :"); Serial.println(m_strTransactionId);
        }
      }

      //Clear
      clearBuffer();
      for (int i = 0; i <= RECEIVED_CHAR_LENGTH; i++) {
        m_chaReceivedChars[i] = NULL;
      }
    }
  }
  //Req Pull Data From Raspi if no transaction
  else {
    if (m_strTransactionExecution != "execute") {
      //Sent Confirmation Transaction to Raspi
      if (m_zaWaitSerialCommand[2] == true) {
        Serial.println(API_TRANSACTION_CONFIRM);
        httpGetRequest(m_strHost, API_TRANSACTION_CONFIRM);
        m_zaWaitSerialCommand[2] = false;
        m_zaWaitSerial[2] = true;
      }

      if (m_zaWaitSerial[2] == true) {
        m_strRawTransactionConfirmation = recvWithStartEndMarkers('<', '>');
        if (m_strRawTransactionConfirmation != "") {
          //Parsing raw Transaction Confirm Json Data
          JsonParsing_TransactionConfirmation();
          Serial.println(m_strRawTransactionConfirmation);
          m_zaWaitSerial[2] = false;
          Serial.println(m_strTransactionId);
          Serial.println(m_zTransactionUpdated);
          Serial.println(m_strTransactionExecution);
          Serial.print(m_zaBinStatus[1]);
          Serial.print(m_zaBinStatus[2]);
          Serial.print(m_zaBinStatus[3]);
          Serial.print(m_zaBinStatus[4]);
          Serial.print(m_zaBinStatus[5]);
          Serial.print(m_zaBinStatus[6]);
          Serial.print(m_zaBinStatus[7]);
          Serial.print(m_zaBinStatus[8]);
          Serial.print(m_zaBinStatus[9]);
          Serial.print(m_zaBinStatus[10]);
          //m_zaWaitSerialCommand[3]=true;
        }
      }
    }
    else {
      if (m_zTransactionUpdated == false) {
        Serial.println("sampai");
        //Update LCD & Turn on LED
        activateTransaction();
        m_zTransactionUpdated = true;
      }
      else {
        //Do Nothing Wait Until All Bin Transaction Completed
      }
    }
  }
}

//Get Initial SKU & Stock Data Function
void getInitialData() {
  //Get Data
//  m_strRawStockData = httpGetRequest(m_strHost,API_GET_CURRENT_DATA);
  //Parsing Stock Data
  JsonParsing_CurrentData();
  //Add If Here
  //Print to RU LCD
  LCDAllUpdatesStock();
}
