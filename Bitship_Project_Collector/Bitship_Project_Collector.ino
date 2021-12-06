// include the library code:
#include <LiquidCrystal.h>
#include <ArduinoJson.h>
#include "WiFiEsp.h"


/** Arduino PIN Setting **/
//PIN for LCD
const char    RTU_NUMBER_ONE    = 0;
const char    RTU_NUMBER_TWO    = RTU_NUMBER_ONE + 1;
const char    RTU_NUMBER_THREE  = RTU_NUMBER_TWO + 1;
const char    RTU_NUMBER_FOUR   = RTU_NUMBER_THREE + 1;
const char    RTU_NUMBER_FIVE   = RTU_NUMBER_FOUR + 1;
const char    RTU_NUMBER_SIX    = RTU_NUMBER_FIVE + 1;
const char    RTU_NUMBER_SEVEN  = RTU_NUMBER_SIX + 1;
const char    RTU_NUMBER_EIGHT  = RTU_NUMBER_SEVEN + 1;
const char    RTU_NUMBER_NINE   = RTU_NUMBER_EIGHT + 1;
const char    RTU_NUMBER_TEN    = RTU_NUMBER_NINE + 1;
const char    RTU_TOTAL         = RTU_NUMBER_TEN + 1;
// TODO try char data type to digital pin
const int     PIN_D4        = 25;
const int     PIN_D5        = 24;
const int     PIN_D6        = 23;
const int     PIN_D7        = 22;
LiquidCrystal m_oaRtu[RTU_TOTAL] = {
    LiquidCrystal (27, 26, PIN_D4, PIN_D5, PIN_D6, PIN_D7), // RTU_NUMBER_ONE
    LiquidCrystal (29, 28, PIN_D4, PIN_D5, PIN_D6, PIN_D7), // RTU_NUMBER_TWO
    LiquidCrystal (31, 30, PIN_D4, PIN_D5, PIN_D6, PIN_D7), // RTU_NUMBER_THREE
    LiquidCrystal (33, 32, PIN_D4, PIN_D5, PIN_D6, PIN_D7), // RTU_NUMBER_FOUR
    LiquidCrystal (35, 34, PIN_D4, PIN_D5, PIN_D6, PIN_D7), // RTU_NUMBER_FIVE
    LiquidCrystal (37, 36, PIN_D4, PIN_D5, PIN_D6, PIN_D7), // RTU_NUMBER_SIX
    LiquidCrystal (39, 38, PIN_D4, PIN_D5, PIN_D6, PIN_D7), // RTU_NUMBER_SEVEN
    LiquidCrystal (41, 40, PIN_D4, PIN_D5, PIN_D6, PIN_D7), // RTU_NUMBER_EIGHT
    LiquidCrystal (43, 42, PIN_D4, PIN_D5, PIN_D6, PIN_D7), // RTU_NUMBER_NINE
    LiquidCrystal (45, 44, PIN_D4, PIN_D5, PIN_D6, PIN_D7)  // RTU_NUMBER_TEN
};
//Pin for LED
const int PIN_LED[RTU_TOTAL]      = {47, 46, 50, 52, 17, 16, 15, 14, 48, 49};
//Pin for Push Button
const int PIN_BUTTON[RTU_TOTAL]   = {2, 3, 4, 5, 6, 7, 8, 9, 10, 11};
bool m_zaIsButtonPressed[RTU_TOTAL] = {false, false, false, false, false, false, false, false, false, false};
//Pin for RTU Detector
const int PIN_DETECT_RU[RTU_TOTAL]  = {A15, A14, A13, A12, A11, A10, A9, A8, A7, A6};
bool m_zaIsRtuConnected[RTU_TOTAL]  = {false, false, false, false, false, false, false, false, false, false};

/** Wifi **/
//const char    WIFI_SSID[]         = "tamsos blok HG 25A";            // your network m_chaSsid (name)
//const char    WIFI_PASSWORD[]     = "qwerty123456";        // your network m_chPassword
//const char    WIFI_SSID[]         = "c7";            // your network m_chaSsid (name)
//const char    WIFI_PASSWORD[]     = "da645591";        // your network m_chPassword
const char    WIFI_SSID[]         = "Warehouse Biteship";            // your network m_chaSsid (name)
const char    WIFI_PASSWORD[]     = "Bismilahlancar";        // your network m_chPassword
//const char    WIFI_SSID[]         = "Diskum_723";            // your network m_chaSsid (name)
//const char    WIFI_PASSWORD[]     = "19283746abcd";
int m_iStatus = WL_IDLE_STATUS;

/** Client **/
WiFiEspClient m_oClient;
const char            COLLECTOR_IDENTIFIER          = 1;
const unsigned long   CLIENT_CONNECTING_TIMEOUT     = 3000;
// TODO try to remove String in COLLECTOR_IDENTIFIER changed data type
const String      API_GET_CURRENT_DATA      = "/get-current-data-by-collector/" + String(COLLECTOR_IDENTIFIER);
const String      API_GET_TRANSACTION       = "/get-transaction-by-collector/" + String(COLLECTOR_IDENTIFIER);
const String      API_TRANSACTION_CONFIRMATION   = "/confirm-on-process-by-collector/" + String(COLLECTOR_IDENTIFIER);
const char        HOST_ADDRESS[]         = "192.168.0.6";
//first layer
const char      RTU_STATE_INITIALIZED    = 0;
const char      RTU_STATE_IDLE            = RTU_STATE_INITIALIZED + 1; // waiting for transaction, always sending get request if transaction is exist
const char      RTU_STATE_TRANSACTION     = RTU_STATE_IDLE + 1; // active -> processing -> confirming -> done (back to idle)
//second layer
const char      RTU_STATE_TRANSACTION_ONGOING       = 0;
const char      RTU_STATE_TRANSACTION_CONFIRMATION  = RTU_STATE_TRANSACTION_ONGOING + 1;
//RTU layer
const char      RTU_STATE_FIRST_LAYER         = 0;
const char      RTU_STATE_SECOND_LAYER        = RTU_STATE_FIRST_LAYER + 1;
const char      RTU_STATE_LAYER_THICKNESS     = RTU_STATE_SECOND_LAYER + 1;
char l_baRtuState[RTU_STATE_LAYER_THICKNESS];
//Data processing
const int       MAX_RECEIVED_CHAR_LENGTH    = 1500; // TODO try with 1000
String m_strRawClientData;
//RU Var Init
String m_straSkuName[RTU_TOTAL];
int m_iaSkuQty[RTU_TOTAL];
//IDLE STATE
const unsigned long   GET_TRANSACTION_PERIOD_TIME           = 6000;
unsigned long m_ulCurrentMillis;
unsigned long m_ulLastHitApiGetTransaction = 0;  //some global variables available anywhere in the program
//TRANSACTION STATE
String m_strTransactionId;
String m_strTransactionExecution;
bool m_zaBinStatus[RTU_TOTAL]; // transaction status exist or not
String m_straBinId[RTU_TOTAL];
String m_straActionCode[RTU_TOTAL];
int m_iaActionQty[RTU_TOTAL];

void setup() {

  //Setting input and output for the collector
  for (int i = 0; i < RTU_TOTAL; i++) {
    //Activate Pull UP for Push Button and RTU detector, default: high, pressed: low
    pinMode(PIN_BUTTON[i], INPUT_PULLUP);
    pinMode(PIN_DETECT_RU[i], INPUT_PULLUP);
    //Setting Output for LED Relay
    pinMode(PIN_LED[i], OUTPUT);
    //Initialize LCD as display output
    m_oaRtu[i].begin(16,2);

    // Check the availability of RTU
    for (int i = 0; i < RTU_TOTAL; i++) {
      if(digitalRead(PIN_DETECT_RU[i]) == LOW) {
        // RTU connected
        m_zaIsRtuConnected[i] = true;
        // Display on LCD that RTU is waiting for running code
        Lcd_Print(m_oaRtu[i], "Initialization", "RU-" + String(i+1) + " Please Wait");
      }
      else{
        // RTU disconnected
        m_zaIsRtuConnected[i] = false;
      }
    }

    // Just give delay
    delay(50);
  }

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
  Wifi_ConnectToNetwork();

  //Wait For Running Code
  delay(1000);
}

void loop() {
  // TODO remove lcd display inside this function
  checkRtuState();

  m_ulCurrentMillis = millis();

  switch(l_baRtuState[RTU_STATE_FIRST_LAYER]){

    case RTU_STATE_INITIALIZED:
      // PRINT: get current stock data for the first time
      Lcd_Print("Connected to WiFi", "Search master API");

      if(Client_HttpGetRequest(HOST_ADDRESS, API_GET_CURRENT_DATA)){
        // listen for current stock data
        if(Client_ReceiveJsonData('<', '>') != "") {
          Serial.println(m_strRawClientData);
          Client_JsonParsingCurrentData();
          Lcd_DisplayStock();
          clearBuffer();

          l_baRtuState[RTU_STATE_FIRST_LAYER] = RTU_STATE_IDLE;
        }
      }
      break;

    case RTU_STATE_IDLE:
      // PRINT: display stock data
      // TODO try uncomment this to always display stock to lcd in idle state
//      Lcd_DisplayStock();

      // get available transaction every specific period
      if (m_ulCurrentMillis - m_ulLastHitApiGetTransaction >= GET_TRANSACTION_PERIOD_TIME) {

        Serial.println(API_GET_TRANSACTION);

        if(Client_HttpGetRequest(HOST_ADDRESS, API_GET_TRANSACTION)){
            // listen for the transaction
            if(Client_ReceiveJsonData('<', '>') != "") {
              Serial.println(m_strRawClientData);
              //Parsing raw Transaction Json Data if transaction exist
              if (Client_JsonParsingTransactionData()) {
                Serial.print("transaction IDs :");
                Serial.println(m_strTransactionId);
                l_baRtuState[RTU_STATE_FIRST_LAYER] = RTU_STATE_TRANSACTION;
              }
              //Clear
              clearBuffer();
            }
        }
        m_ulLastHitApiGetTransaction = m_ulCurrentMillis;
      }
      break;

    case RTU_STATE_TRANSACTION:
      switch (l_baRtuState[RTU_STATE_SECOND_LAYER]) {

        case RTU_STATE_TRANSACTION_ONGOING:
          Serial.println(API_TRANSACTION_CONFIRMATION);

          if(Client_HttpGetRequest(HOST_ADDRESS, API_TRANSACTION_CONFIRMATION)){
              if(Client_ReceiveJsonData('<', '>') == "") {
                Client_JsonParsingTransactionConfirmation();
                Serial.println(m_strRawClientData);
                Serial.println(m_strTransactionId);
                Serial.println(m_strTransactionExecution);
                for (int i = 0; i < RTU_TOTAL; i++) {
                    Serial.print(m_zaBinStatus[i]);
                }
                Serial.println();

                // PRINT: display active RTU
                // TODO try to move this outside this if to always print LCD
                Client_ActivateTransaction();
                l_baRtuState[RTU_STATE_SECOND_LAYER] = RTU_STATE_TRANSACTION_CONFIRMATION;
              }
          }
          break;

        case RTU_STATE_TRANSACTION_CONFIRMATION:
          //Check Push Button For confirming Transaction (HIT API)
          Client_DeactivateTransaction();

          //check active transaction
          int l_iActiveTransaction = 0;
          for (int i = 0; i < RTU_TOTAL; i++) {
            // PRINT: display stock data for idle RTU and display transaction data for active RTU
            if (m_zaBinStatus[i] == true) {
              l_iActiveTransaction++;
              Lcd_TransactionOngoing(i);
            }else{
              Lcd_DisplayStock(i);
            }
          }

          //transaction done
          if(l_iActiveTransaction == 0) {
            m_strTransactionId = "";
            l_baRtuState[RTU_STATE_FIRST_LAYER] = RTU_STATE_IDLE;
            l_baRtuState[RTU_STATE_SECOND_LAYER] = RTU_STATE_TRANSACTION_ONGOING;
          }
          break;
      }
      break;
  }

  //Loop Sampling Time
  delay(1);
}


/** LCD Related Function **/
//Function print to specific LCD
void Lcd_Print(LiquidCrystal p_oLcd, String p_strFirstRow, String p_strSecondRow) {
  p_oLcd.clear();
  p_oLcd.setCursor(0, 0);
  p_oLcd.print(p_strFirstRow);
  p_oLcd.setCursor(0, 1);
  p_oLcd.print(p_strSecondRow);
}

//Function print to all LCD
void Lcd_Print(String p_strFirstRow, String p_strSecondRow){
  for (int i = 0; i < RTU_TOTAL; i++) {
    Lcd_Print(m_oaRtu[i], p_strFirstRow, p_strSecondRow);
  }
}

//Function LCD display specific SKU & quantity
void Lcd_DisplayStock(int p_iRtuId) {
  // TODO try to remove lcd begin
  m_oaRtu[p_iRtuId].begin(16, 2);
  m_oaRtu[p_iRtuId].clear();
  m_oaRtu[p_iRtuId].setCursor(0, 0);
  m_oaRtu[p_iRtuId].print("SKU:" + m_straSkuName[p_iRtuId]);
  m_oaRtu[p_iRtuId].setCursor(0, 1);
  m_oaRtu[p_iRtuId].print("Qty:" + String(m_iaSkuQty[p_iRtuId]));
  m_oaRtu[p_iRtuId].setCursor(11, 1);
  m_oaRtu[p_iRtuId].print("RU-" + String(p_iRtuId));
}

//Function LCD display all SKU & quantity
void Lcd_DisplayStock() {
  //Print Stock SKU to 10 RU
  for (int i = 0; i < RTU_TOTAL; i++) {
    Lcd_DisplayStock(i);
  }
}

//Confirmation Status
void Lcd_TransactionOngoing(int p_iRtuId) {
  // (note: line 1 is the second row, since counting begins with 0):
  m_oaRtu[p_iRtuId].clear();
  m_oaRtu[p_iRtuId].setCursor(0, 0);
  m_oaRtu[p_iRtuId].print("Action : " + m_straActionCode[p_iRtuId]);
  m_oaRtu[p_iRtuId].setCursor(0, 1);
  m_oaRtu[p_iRtuId].print("Quantity : " + String(m_iaActionQty[p_iRtuId]));
}
/** LCD Related Function **/

/** WIFI ESP Function **/
//Connect to your wifi network
void Wifi_ConnectToNetwork() {
  // attempt to connect to WiFi network
  while (m_iStatus != WL_CONNECTED) {
    Serial.print("Attempting to connect to WPA SSID: ");
    Serial.println(WIFI_SSID);

    // Connect to WPA/WPA2 network
    m_iStatus = WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  }

  Serial.println("You're connected to the network");
}
/** WIFI ESP Function **/

/** Client Function **/
bool Client_HttpGetRequest(char p_baServer[], String p_strQuery) {
  bool l_zIsConnected = false;

  // TODO Try removing this loop
  while(!l_zIsConnected) {
    //Close All Socket
    m_oClient.stop();

    Serial.println("Attempting to connect to raspi API: " + String(p_baServer));
    l_zIsConnected = m_oClient.connect(p_baServer, CLIENT_CONNECTING_TIMEOUT);
  }

  // if there's a successful connection
  if (l_zIsConnected) {
    Serial.println("Connected");
    // send the HTTP GET request
    m_oClient.println(("GET " + p_strQuery + " HTTP/1.1"));
    Serial.println(p_strQuery);
    //m_oClient.println(F("GET /get-current-data-by-collector/1 HTTP/1.1"));
    //m_oClient.println(F("Host: 83f3-182-253-87-219.ngrok.io"));
    m_oClient.println(("Host: " + String(p_baServer)));
    m_oClient.println("Connection: close");
    m_oClient.println();

    return true;
  }

  // if you couldn't make a connection
  Lcd_Print("Failed", "Restart Collector");
  Serial.println("Connection failed, please restart related collector");
  return false;
}

String Client_ReceiveJsonData(char p_bStartMarker, char p_bEndMarker) {

  char l_baReceivedChars[MAX_RECEIVED_CHAR_LENGTH];
  static boolean l_zRecvInProgress = false;
  static int l_iNdx = 0;
  char l_chRc;
  boolean l_zNewData = false;

  while (m_oClient.available() > 0 && l_zNewData == false) {
    l_chRc = m_oClient.read();

    if (l_zRecvInProgress == true) {
      if (l_chRc != p_bEndMarker) {
        l_baReceivedChars[l_iNdx] = l_chRc;
        l_iNdx++;
        if (l_iNdx >= MAX_RECEIVED_CHAR_LENGTH) {
          l_iNdx = MAX_RECEIVED_CHAR_LENGTH - 1;
        }
      }
      else {
        l_baReceivedChars[l_iNdx] = '\0'; // terminate the string
        l_zRecvInProgress = false;
        l_iNdx = 0;
        l_zNewData = true;
      }
    }
    else if (l_chRc == p_bStartMarker) {
      l_zRecvInProgress = true;
    }
  }

  if (l_zNewData == true) {
    Serial.println(l_baReceivedChars);
  }

  m_strRawClientData = String(l_baReceivedChars);
  return m_strRawClientData;
}

//Parsing CurrentData
void Client_JsonParsingCurrentData() {
  StaticJsonDocument<MAX_RECEIVED_CHAR_LENGTH> l_oDoc;
  DeserializationError l_oError = deserializeJson(l_oDoc, m_strRawClientData);

  if (l_oError) {
    Serial.println("Error Parsing Json Current Data");
  }

  //Status Data
  bool l_zStatus = l_oDoc["success"];

  //Function Code
  String l_strMessage = l_oDoc["message"];

  //Data Object (10 RU , SKU Name, Bin ID,
  JsonObject l_oData = l_oDoc["data"];
  JsonArray l_oaArrayData = l_oDoc["data"].as<JsonArray>();

  //Parsing Array Data For 10 SKU
  for (int i = 0; i <= RTU_TOTAL; i++) {
    JsonObject l_oSku = l_oaArrayData[i];
    String l_strDeviceId = (const char*)l_oSku["device_id"];
    int l_iBinId = l_strDeviceId.substring(3).toInt();
    String l_strSkuName = (const char*)l_oSku["sku_name"];
    int l_iSkuQty = (int)l_oSku["quantity"];

    // get SKU name and its quantity
    m_straSkuName[l_iBinId] = l_strSkuName;
    m_iaSkuQty[l_iBinId] = l_iSkuQty;
  }
}

//Parsing TransactionData
bool Client_JsonParsingTransactionData() {
  bool l_zStatus = false;
  StaticJsonDocument<MAX_RECEIVED_CHAR_LENGTH> l_oDoc;
  DeserializationError l_oError = deserializeJson(l_oDoc, m_strRawClientData);

  if (l_oError) {
    Serial.println("Error Parsing Json Transaction Data");
  }
  else { //Parsing the Data And Move to Global Var
    //Transaction_Status
    l_zStatus = l_oDoc["success"];

    if (l_zStatus == true) {
      Serial.print("trans Stat : "); Serial.println(l_zStatus);
      //Transaction_id
      m_strTransactionId = (const char*)l_oDoc["transaction_id"];
      //Data Array ( x Transaction Bin )
      JsonArray l_oaArrayTransaction = l_oDoc["data"].as<JsonArray>();
      //Parsing Array Transaction Data For x SKU

      for (int i = 0; i <= RTU_TOTAL; i++) {
        JsonObject l_oJsonTransactionData = l_oaArrayTransaction[i];
        //Get Local Var
        String l_strBinId = (const char*)l_oJsonTransactionData["device_id"];
        String l_action_code = (const char*)l_oJsonTransactionData["action"];
        int l_iActionQty = (int)l_oJsonTransactionData["quantity"];
        //Decode bin ID
        int l_iBinId = l_strBinId.substring(3).toInt();
        // int ll_bin_id = l_oaJsonTransactionData["bin_id"];
        //Move Global Array
        m_zaBinStatus[l_iBinId] = true;
        m_straBinId[l_iBinId] = l_strBinId;
        m_straActionCode[l_iBinId] = l_action_code;
        m_iaActionQty[l_iBinId] = l_iActionQty;
      }
    }
    return l_zStatus;
  }
}

//Parsing CurrentData
void Client_JsonParsingTransactionConfirmation() {
  StaticJsonDocument<MAX_RECEIVED_CHAR_LENGTH> l_oDoc;
  DeserializationError l_oError = deserializeJson(l_oDoc, m_strRawClientData);

  if (l_oError) {
    Serial.println("Error Parsing Transaction Confirmation");
  }
  //Parsing the Data And Move to Global Var
  else {
    //Transaction_Status
    JsonObject l_oJsonTransactionData = l_oDoc["data"];
    m_strTransactionExecution = (const char*)l_oJsonTransactionData["status"];
  }
}

void Client_JsonParsingPickingsConfirmation(int p_iBinIds) {
  StaticJsonDocument<MAX_RECEIVED_CHAR_LENGTH> l_oDoc;
  DeserializationError l_oError = deserializeJson(l_oDoc, m_strRawClientData);

  if (l_oError) {
    Serial.println("Error Parsing Pickings Confirmation");
  }
  else { //Parsing the Data And Move to Global Var
    //Transaction_Status
    bool transStatus = l_oDoc["success"];
    if (transStatus) {
      JsonObject l_oJsonTransactionData = l_oDoc["data"];
      m_zaBinStatus[p_iBinIds] = false;
      //        m_straSkuName[p_iBinIds]=(const char*)l_oaJsonTransactionData["sku_name"];
      m_iaSkuQty[p_iBinIds] = (int)l_oJsonTransactionData["quantity"];
      Serial.println(m_zaBinStatus[p_iBinIds]);
      //        Serial.println(m_straSkuName[p_iBinIds]);
    }
  }
}

//Function Pickings
void Client_ActivateTransaction() {
  //Turn on LED
  for (int i=0; i < RTU_TOTAL; i++) {
    if (m_zaBinStatus[i] == true) {
      // LCD Update
      Lcd_TransactionOngoing(i);
      delay(50);
      digitalWrite(PIN_LED[i], HIGH); //LED ON
    }
    else {
      Lcd_DisplayStock(i);
      digitalWrite(PIN_LED[i], LOW); //LED OFF
    }
  }
}

void Client_DeactivateTransaction() {
  // check if button was pressed
  checkPushButton();

  // TODO try to remove for loop statement then hit api with specific RTU only that button was pressed previously
  for (int i = 1; i <= 10; i++) {
    if (m_zaIsButtonPressed[i] == true) {
      //Hit API
      // Client_HttpGetRequest(HOST_ADDRESS, "/confirm-done?bin_id="+String(i)+"&transaction_id="+m_strTransactionId);
      if(Client_HttpGetRequest(HOST_ADDRESS, "/confirm-done?transaction_id=" + m_strTransactionId + "&device_id=" + m_straBinId[i])){
        //Get Serial Data
        if (Client_ReceiveJsonData('<', '>') != "") {
          //Parse API
          Serial.println(m_strRawClientData);
          Client_JsonParsingPickingsConfirmation(i);

          //Turn Off LED & Update LCD
          if (m_zaBinStatus[i] == false) {
            //Turn Off LED
            Lcd_DisplayStock(i);
            delay(50);
            digitalWrite(PIN_LED[i], LOW);
          }
          m_zaIsButtonPressed[i] = false;
        }
      }
    }
  }
}
/** Client Function **/

/** Other Function **/
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

void checkRtuState() {
  for (int i = 0; i < RTU_TOTAL; i++) {

    // RTU just connected and last state is not connected
    if (digitalRead(PIN_DETECT_RU[i]) == LOW && m_zaIsRtuConnected[i] == false){
      delay(100);
      m_zaIsRtuConnected[i] = true;
      Serial.println("RTU Was Connected : " + String(i));
      Serial.println(m_zaIsRtuConnected[i]);
      Lcd_DisplayStock(i);
    }

    // RTU just disconnected and last state is connected
    if (digitalRead(PIN_DETECT_RU[i]) == HIGH && m_zaIsRtuConnected[i] == true) {
      delay(100);
      m_zaIsRtuConnected[i] = false;
      Serial.println("RTU Was Disconnected : " + String(i));
      Serial.println(m_zaIsRtuConnected[i]);
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
      m_zaIsButtonPressed[i] = true;
      Serial.println("Button Was Pressed : " + String(i));
      Serial.println(m_zaIsButtonPressed[i]);
      // digitalWrite(PIN_LED[i],!digitalRead(PIN_BUTTON[i]));
    }
  }
}
/** Other Function **/
