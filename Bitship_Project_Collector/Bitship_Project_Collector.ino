// include the library code:
#include <LiquidCrystal.h>
#include <ArduinoJson.h>
#include <WiFiEsp.h>


/** PIN Setting */
const int REMOTE_UNIT_AMOUNT = 10;
// TODO try char data type to digital pin
const int     PIN_D4        = 25;
const int     PIN_D5        = 24;
const int     PIN_D6        = 23;
const int     PIN_D7        = 22;
LiquidCrystal m_oaRtu[REMOTE_UNIT_AMOUNT] = {
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
const int PIN_LED[REMOTE_UNIT_AMOUNT]      = {47, 46, 50, 52, 17, 16, 15, 14, 48, 49};
//Pin for Push Button
const int PIN_BUTTON[REMOTE_UNIT_AMOUNT]   = {2, 3, 4, 5, 6, 7, 8, 9, 10, 11};
bool m_zaIsButtonPressed[REMOTE_UNIT_AMOUNT] = {false, false, false, false, false, false, false, false, false, false};
//Pin for RTU Detector
const int PIN_DETECT_RU[REMOTE_UNIT_AMOUNT]  = {A15, A14, A13, A12, A11, A10, A9, A8, A7, A6};
bool m_zaIsRtuConnected[REMOTE_UNIT_AMOUNT]  = {false, false, false, false, false, false, false, false, false, false};


/** Wifi */
//const char    WIFI_SSID[]         = "tamsos blok HG 25A";            // your network m_chaSsid (name)
//const char    WIFI_PASSWORD[]     = "qwerty123456";        // your network m_chPassword
//const char    WIFI_SSID[]         = "c7";            // your network m_chaSsid (name)
//const char    WIFI_PASSWORD[]     = "da645591";        // your network m_chPassword
//const char    WIFI_SSID[]         = "Warehouse Biteship";            // your network m_chaSsid (name)
//const char    WIFI_PASSWORD[]     = "Bismilahlancar";        // your network m_chPassword
//const char    WIFI_SSID[]         = "Diskum_723";            // your network m_chaSsid (name)
//const char    WIFI_PASSWORD[]     = "19283746abcd";
const char    WIFI_SSID[]         = "LAN";            // your network m_chaSsid (name)
const char    WIFI_PASSWORD[]     = "LAN43406";
int m_iStatus = WL_IDLE_STATUS;
WiFiEspClient m_oClient;


/** Client */
const String COLLECTOR_IDENTIFIER = "1";
const String API_REGISTER_RU = "/ru-registration-collector/" + COLLECTOR_IDENTIFIER;
const String API_GET_CURRENT_DATA = "/get-current-data-by-collector/" + COLLECTOR_IDENTIFIER;
const String API_GET_TRANSACTION = "/get-transaction-by-collector/"+ COLLECTOR_IDENTIFIER;
const String API_TRANSACTION_CONFIRM = "/confirm-on-process-by-collector/" + COLLECTOR_IDENTIFIER;
const String API_PICKING_CONFIRM = "/confirm-done?bin_id=";
//receive global variable
bool l_bIsSerialReceive = false;
const int RECEIVED_CHAR_LENGTH = 1500;
char m_caReceivedChars[RECEIVED_CHAR_LENGTH];
String m_strRawReceivedData;
bool m_bNewData = false;
//const char HOST_ADDRESS[] = "192.168.0.6";
const char HOST_ADDRESS[] = "192.168.1.7";
const int HOST_PORT = 3000;
//state
const char RU_STATE_FIRST_LAYER = 0;
const char RU_STATE_SECOND_LAYER = RU_STATE_FIRST_LAYER + 1;
const char RU_STATE_LAYER_THICKNESS = RU_STATE_SECOND_LAYER + 1;
char m_caRtuState[RU_STATE_LAYER_THICKNESS];
//state first layer
const char RU_STATE_REGISTRATION = 0;
const char RU_STATE_READY = RU_STATE_REGISTRATION + 1;
const char RU_STATE_IDLE = RU_STATE_READY + 1;
const char RU_STATE_TRANSACTION = RU_STATE_IDLE + 1;
//state second layer
const char RU_STATE_TRANSACTION_ACTIVE = 0;
const char RU_STATE_TRANSACTION_CONFIRMATION = RU_STATE_TRANSACTION_ACTIVE + 1;
//RU state-ready variable
String m_straSkuName[REMOTE_UNIT_AMOUNT];
int m_iaSkuQty[REMOTE_UNIT_AMOUNT];
//RU state-idle variable
unsigned long m_ulLastGetActiveTransactionApi = 0;
unsigned long PERIOD_TIME_GET_ACTIVE_TRANSACTION = 6000;
String m_strTransactionId;
bool m_zaBinStatus[REMOTE_UNIT_AMOUNT]; // transaction status exist or not
String m_straBinId[REMOTE_UNIT_AMOUNT];
String m_straActionCode[REMOTE_UNIT_AMOUNT];
int m_iaActionQty[REMOTE_UNIT_AMOUNT];
//RU state-transaction-active
String m_strTransactionExecution;
//RU state-transaction-confirm
bool m_zConfirmationStatus = false;

/** Others */
unsigned long m_ulCurrentMillis;

void setup() {

  //Setting input and output for the collector
  for (int i = 0; i < REMOTE_UNIT_AMOUNT; i++) {

    //Activate Pull UP for Push Button and RTU detector, default: high, pressed: low
    pinMode(PIN_BUTTON[i], INPUT_PULLUP);
    pinMode(PIN_DETECT_RU[i], INPUT_PULLUP);
    //Setting Output for LED Relay
    pinMode(PIN_LED[i], OUTPUT);
    //Initialize LCD as display output
    m_oaRtu[i].begin(16,2);
  }

  //Initiate Serial Communication
  Lcd_Print("Initiate WiFi", "Please Wait");

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

  //check connected device
  for (int i = 0; i < REMOTE_UNIT_AMOUNT; i++) {
    // Check the availability of RTU
    if(digitalRead(PIN_DETECT_RU[i]) == LOW) {
      // RTU connected
      m_zaIsRtuConnected[i] = true;
      // Display on LCD that RTU is waiting for running code
      Serial.println("Connect: C" + COLLECTOR_IDENTIFIER + "B" + String(i+1));
      Lcd_Print(m_oaRtu[i], "Wifi Connected", "C" + COLLECTOR_IDENTIFIER + "B" + String(i+1) + " Please Wait");
    }
    else{
      // RTU disconnected
      m_zaIsRtuConnected[i] = false;
      Serial.println("Disonnect: C" + COLLECTOR_IDENTIFIER + "B" + String(i+1));
    }
  }
}

void loop() {

  m_ulCurrentMillis = millis();

  switch (m_caRtuState[RU_STATE_FIRST_LAYER]) {

    case RU_STATE_REGISTRATION:
      //display to LCD: registering device
      Lcd_PrintRegisteringDevice();

      // request
      if (!l_bIsSerialReceive) {
        //construct registering device in json format
        String l_strConnectedDevice = "{\"collector\": " + COLLECTOR_IDENTIFIER + ", \"status\": [";
        for (int i = 0; i < REMOTE_UNIT_AMOUNT; i++) {
          l_strConnectedDevice += (i!=(REMOTE_UNIT_AMOUNT-1)) ? (String)m_zaIsRtuConnected[i]+"," : (String)m_zaIsRtuConnected[i];
        }
        l_strConnectedDevice += "]}";

        // Post to API register
        if(httpPostRequest(API_REGISTER_RU, l_strConnectedDevice)){
          l_bIsSerialReceive = true;
        }
      }

      // receive
      if (l_bIsSerialReceive) {
        m_strRawReceivedData = recvWithStartEndMarkers('<', '>');

        //parsed confirmation data
        if (m_strRawReceivedData != "") {
          if(JsonParsing_RegisterConfirmation()){
            Serial.println("registered");
            clearBuffer();

            m_caRtuState[RU_STATE_FIRST_LAYER] = RU_STATE_READY;
          }
          //post request again if confirmation not accepted
          l_bIsSerialReceive = false;
        }
      }
      break;

    case RU_STATE_READY:
        //display to LCD: registering device
      Lcd_PrintGettingBinData();

      // request
      if (!l_bIsSerialReceive) {
        // Get from API current data
        if(httpGetRequest(API_GET_CURRENT_DATA)){
          l_bIsSerialReceive = true;
        }
      }

      // receive
      if (l_bIsSerialReceive) {
          m_strRawReceivedData = recvWithStartEndMarkers('<', '>');

          //parsed bin data
          if (m_strRawReceivedData != "") {

            JsonParsing_CurrentData();
            clearBuffer();
            m_caRtuState[RU_STATE_FIRST_LAYER] = RU_STATE_IDLE;
            l_bIsSerialReceive = false;
          }
        }

      break;

    case RU_STATE_IDLE:
        //display to LCD: bin stock data
        Lcd_PrintCurrentBinData();

        // back to device registration if any connected/disconnected device
        // TODO check if RU1 PIN always says "RTU Was Disconnect : 0" if not connected
//        if (checkRtuState() != 0) {
//          m_caRtuState[RU_STATE_FIRST_LAYER] = RU_STATE_REGISTRATION;
//          break;
//        }

        // request every specific period
        if (!l_bIsSerialReceive && m_ulCurrentMillis - m_ulLastGetActiveTransactionApi >= PERIOD_TIME_GET_ACTIVE_TRANSACTION) {
          // Get from API current data
          if(httpGetRequest(API_GET_TRANSACTION)){
            l_bIsSerialReceive = true;
          }
          m_ulLastGetActiveTransactionApi = m_ulCurrentMillis;
        }

        //receive
        if (l_bIsSerialReceive) {
          m_strRawReceivedData = recvWithStartEndMarkers('<', '>');

          //parsed transaction data
          if (m_strRawReceivedData != "") {

            if(JsonParsing_TransactionData()) {
              clearBuffer();
              m_caRtuState[RU_STATE_FIRST_LAYER] = RU_STATE_TRANSACTION;
              m_caRtuState[RU_STATE_SECOND_LAYER] = RU_STATE_TRANSACTION_ACTIVE;
            }
            l_bIsSerialReceive = false;
          }
        }
      break;

    // TODO this switch case is not tested
    case RU_STATE_TRANSACTION:

      switch (m_caRtuState[RU_STATE_SECOND_LAYER]) {
        case RU_STATE_TRANSACTION_ACTIVE:
          //display to LCD: transaction status
          Lcd_PrintTransactionState(false);

          // request
          if (!l_bIsSerialReceive){
            // Get from API current data
            if(httpGetRequest(API_TRANSACTION_CONFIRM)){
              l_bIsSerialReceive = true;
            }
          }

          //receive
          if (l_bIsSerialReceive) {
            m_strRawReceivedData = recvWithStartEndMarkers('<', '>');

            //parsed transaction data
            if (m_strRawReceivedData != "") {
              JsonParsing_TransactionConfirmation();
              clearBuffer();
              m_caRtuState[RU_STATE_SECOND_LAYER] = RU_STATE_TRANSACTION_CONFIRMATION;
              l_bIsSerialReceive = false;
            }
          }
          break;
        case RU_STATE_TRANSACTION_CONFIRMATION:
          //display to LCD: transaction status
          int l_iActiveTransaction = Lcd_PrintTransactionState(true);
          checkPushButton();

          for (int i = 0; i < REMOTE_UNIT_AMOUNT; i++) {
            if (m_zaIsButtonPressed[i] == true) {
              // request
              if (!l_bIsSerialReceive){
                if(httpGetRequest("/confirm-done?transaction_id=" + m_strTransactionId + "&device_id=" + m_straBinId[i])){
                  l_bIsSerialReceive = true;
                }
              }

              //receive
              if (l_bIsSerialReceive) {
                m_strRawReceivedData = recvWithStartEndMarkers('<', '>');
                if (m_strRawReceivedData != "") {
                  //Parse API
                  JsonParsing_PickingsConfirmation(i);
                  clearBuffer();
                  m_zaIsButtonPressed[i] = false;
                  m_zaBinStatus[i] = false;
                }
              }
            }
          }

          // transaction done
          if (l_iActiveTransaction == 0) {
            m_caRtuState[RU_STATE_FIRST_LAYER] = RU_STATE_IDLE;
            m_caRtuState[RU_STATE_SECOND_LAYER] = RU_STATE_TRANSACTION_ACTIVE;
          }
          break;
      }
      break;
  }

  delay(1);
}


/** LCD Related Function */
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

  for (int i = 0; i < REMOTE_UNIT_AMOUNT; i++) {
    Lcd_Print(m_oaRtu[i], p_strFirstRow, p_strSecondRow);
  }
}

void Lcd_PrintDeviceNotRegistered(int p_iBinId){

  Lcd_Print(m_oaRtu[p_iBinId], "RU: C" + COLLECTOR_IDENTIFIER + "B" + String(p_iBinId+1), "Status:Unregistered");
}

void Lcd_PrintRegisteringDevice() {
  for (int i = 0; i < REMOTE_UNIT_AMOUNT; i++) {
    if(m_zaIsRtuConnected[i]){
      Lcd_Print(m_oaRtu[i], "Registering", "C" + COLLECTOR_IDENTIFIER + "B" + String(i+1) + " Please Wait");
    }
    else{
      Lcd_PrintDeviceNotRegistered(i);
    }
  }
}

void Lcd_PrintGettingBinData() {
  for (int i = 0; i < REMOTE_UNIT_AMOUNT; i++) {
    if(m_zaIsRtuConnected[i]){
      Lcd_Print(m_oaRtu[i], "Getting bin data", "C" + COLLECTOR_IDENTIFIER + "B" + String(i+1) + " Please Wait");
    }
    else{
      Lcd_PrintDeviceNotRegistered(i);
    }
  }
}

void Lcd_PrintCurrentBinData(int p_iaBinId) {
  for (int i = 0; i < REMOTE_UNIT_AMOUNT; i++) {
    if(m_zaIsRtuConnected[i]){
      m_oaRtu[i].clear();
      m_oaRtu[i].setCursor(0,0);
      m_oaRtu[i].print("SKU:" + m_straSkuName[i]);
      m_oaRtu[i].setCursor(0,1);
      m_oaRtu[i].print("Qty:" + String(m_iaSkuQty[i]));
      m_oaRtu[i].setCursor(10,1);
      m_oaRtu[i].print("C" + COLLECTOR_IDENTIFIER + "B" + String(i+1));
    }
    else{
      Lcd_PrintDeviceNotRegistered(i);
    }
  }
}

void Lcd_PrintCurrentBinData() {
  for (int i = 0; i < REMOTE_UNIT_AMOUNT; i++) {
    if(m_zaIsRtuConnected[i]){
      Lcd_PrintCurrentBinData(i);
    }
    else{
      Lcd_PrintDeviceNotRegistered(i);
    }
  }
}

int Lcd_PrintTransactionState(bool p_bIsExecution){

  int l_iActiveTransaction = 0;

  for (int i = 0; i < REMOTE_UNIT_AMOUNT; i++) {
    if(m_zaIsRtuConnected[i]){
      if(m_zaBinStatus[i]){
        if (p_bIsExecution) {
          m_oaRtu[i].clear();
          m_oaRtu[i].setCursor(0, 0);
          m_oaRtu[i].print("Act:" + m_straActionCode[i]);
          m_oaRtu[i].setCursor(0, 1);
          m_oaRtu[i].print("Qty:" + String(m_iaActionQty[i]));
          m_oaRtu[i].setCursor(10,1);
        } else {
          m_oaRtu[i].clear();
          m_oaRtu[i].setCursor(0, 0);
          m_oaRtu[i].print("Waiting for confirmation");
        }
        m_oaRtu[i].print("C" + COLLECTOR_IDENTIFIER + "B" + String(i+1));
        digitalWrite(PIN_LED[i], HIGH); //LED ON

        l_iActiveTransaction++;
      }
      else{
        Lcd_PrintCurrentBinData(i);
        digitalWrite(PIN_LED[i], LOW); //LED OFF
      }
    }
    else{
      Lcd_PrintDeviceNotRegistered(i);
    }
  }

  return l_iActiveTransaction;
}

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


/** Client **/
bool httpPostRequest (String p_strQuery, String p_strPayload) {

  bool l_zIsConnected = false;

  while(!l_zIsConnected) {
    //Close All Socket
    m_oClient.stop();
    l_zIsConnected = m_oClient.connect(HOST_ADDRESS, HOST_PORT);
    Serial.println("Attempting to connect to raspi API: " + String(HOST_ADDRESS));
  }

  // if there's a successful connection
  if (l_zIsConnected) {
    Serial.println("Connected");
    Serial.println(p_strQuery);

    // send the HTTP GET request
    m_oClient.println(("POST " + p_strQuery + " HTTP/1.1"));
    Serial.println(("POST " + p_strQuery + " HTTP/1.1"));
    m_oClient.println(("Host: " + String(HOST_ADDRESS)));
    Serial.println(("Host: " + String(HOST_ADDRESS)));
    m_oClient.println("Accept: application/json");
    m_oClient.println(("Content-Type: application/json"));
    Serial.println(("Content-Type: application/json"));
    m_oClient.println("Content-Length: " +String(p_strPayload.length() +1));
    Serial.println("Content-Length: " +String(p_strPayload.length() + 1));
    m_oClient.println();
    m_oClient.println((p_strPayload));
    Serial.println((p_strPayload));

    return true;
  }

  Lcd_Print("Failed", "Restart Collector");
  Serial.println("Connection failed, please restart related collector");
  return false;
}

bool httpGetRequest (String p_strQuery) {
  bool l_zIsConnected = false;
  while(!l_zIsConnected) {
    //Close All Socket
    m_oClient.stop();
    l_zIsConnected = m_oClient.connect(HOST_ADDRESS, HOST_PORT);
    Serial.println("Attempting to connect to raspi API: " + String(HOST_ADDRESS));
  }

  // if there's a successful connection
  if (l_zIsConnected) {
    Serial.println("Connected");
    // send the HTTP GET request
    m_oClient.println(("GET " + p_strQuery + " HTTP/1.1"));
    Serial.println(p_strQuery);
    m_oClient.println(("Host: " + String(HOST_ADDRESS)));
    m_oClient.println("Connection: close");
    m_oClient.println();

    return true;
  }

  Lcd_Print("Failed", "Restart Collector");
  Serial.println("Connection failed, please restart related collector");
  return false;
}

String recvWithStartEndMarkers(char p_chStartMarker, char p_chEndMarker) {
  static boolean l_zRecvInProgress = false;
  static int l_iNdx = 0;
  char l_chRc;

  //wait for api return value
  while (!m_oClient.available());

  //there is something to be parsed
  while (m_oClient.available() > 0 && m_bNewData == false) {
    l_chRc = m_oClient.read();

    if (l_zRecvInProgress == true) {
      if (l_chRc != p_chEndMarker) {
        m_caReceivedChars[l_iNdx] = l_chRc;
        l_iNdx++;
        if (l_iNdx >= RECEIVED_CHAR_LENGTH) {
          l_iNdx = RECEIVED_CHAR_LENGTH - 1;
        }
      }
      else {
        m_caReceivedChars[l_iNdx] = '\0'; // terminate the string
        l_zRecvInProgress = false;
        l_iNdx = 0;
        m_bNewData = true;
      }
    }
    else if (l_chRc == p_chStartMarker) {
      l_zRecvInProgress = true;
    }
  }

  if (m_bNewData == true) {
    Serial.println(m_caReceivedChars);
    m_bNewData = false;
  }
  return m_caReceivedChars;
}

bool JsonParsing_RegisterConfirmation(){

  // TODO parse and check register confirmation data
  return true;
}

//Parsing CurrentData
void JsonParsing_CurrentData() {
  StaticJsonDocument<RECEIVED_CHAR_LENGTH> l_oDoc;
  DeserializationError l_oError = deserializeJson(l_oDoc, m_strRawReceivedData);

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
  for (int i = 0; i < REMOTE_UNIT_AMOUNT; i++) {
    JsonObject l_oSku = l_oaArrayData[i];
    String l_strDeviceId = (const char*)l_oSku["device_id"];
    int l_iBinId = l_strDeviceId.substring(3).toInt();
    String l_strSkuName = (const char*)l_oSku["sku_name"];
    int l_iSkuQty = (int)l_oSku["quantity"];
    m_straSkuName[l_iBinId] = l_strSkuName;
    m_iaSkuQty[l_iBinId] = l_iSkuQty;
  }
}

//Parsing transaction data
bool JsonParsing_TransactionData() {
  bool l_zStatus = false;
  StaticJsonDocument<RECEIVED_CHAR_LENGTH> l_oParsedData;
  DeserializationError l_oError = deserializeJson(l_oParsedData, m_strRawReceivedData);

  if (!l_oError) {
    //Parsing the Data And Move to Global Var
    //Transaction_Status
    l_zStatus = l_oParsedData["success"];

    if (l_zStatus == true) {
      Serial.print("trans Stat : ");
      Serial.println(l_zStatus);
      //Transaction_id
      m_strTransactionId = (const char*)l_oParsedData["transaction_id"];
      //Data Array ( x Transaction Bin )
      JsonArray l_oaArrayTransaction = l_oParsedData["data"].as<JsonArray>();
      //Parsing Array Transaction Data For x SKU

      for (int i = 0; i <= REMOTE_UNIT_AMOUNT; i++) {
        JsonObject l_oJsonTransactionData = l_oaArrayTransaction[i];
        //Get Local Var
        String l_strBinId = (const char*)l_oJsonTransactionData["device_id"];
        String l_strActionCode = (const char*)l_oJsonTransactionData["action"];
        int l_iActionQty = (int)l_oJsonTransactionData["quantity"];
        //Decode bin ID
        int l_iBinId = l_strBinId.substring(3).toInt();
        // int ll_bin_id = l_oaJsonTransactionData["bin_id"];
        //Move Global Array
        m_zaBinStatus[l_iBinId] = true;
        m_straBinId[l_iBinId] = l_strBinId;
        m_straActionCode[l_iBinId] = l_strActionCode;
        m_iaActionQty[l_iBinId] = l_iActionQty;
      }
      return l_zStatus;
    }
  }

  return false;
  Serial.println("Error Parsing Json Transaction Data");
}

//Parsing CurrentData
void JsonParsing_TransactionConfirmation() {
  StaticJsonDocument<RECEIVED_CHAR_LENGTH> l_oParsedData;
  DeserializationError l_oError = deserializeJson(l_oParsedData, m_strRawReceivedData);

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
  StaticJsonDocument<RECEIVED_CHAR_LENGTH> l_oParsedData;
  DeserializationError l_oError = deserializeJson(l_oParsedData, m_strRawReceivedData);

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
  }
}


/** Other Function **/
//Clear Buffer
void clearBuffer() {
  //clear Buffer
  Serial1.flush();
  Serial.flush();

  //deplete buffer
  while (Serial1.available() || Serial.available()) {
    Serial.read();
    Serial1.read();
  }

  //clear global array
  for(int i=0; i< RECEIVED_CHAR_LENGTH;i++){
    m_caReceivedChars[i]=NULL;
  }
  m_strRawReceivedData = "";
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
  for (int i = 0; i < REMOTE_UNIT_AMOUNT; i++) {
  //Check Button are Pressed or Not
    if (buttonWasPressed(PIN_BUTTON[i]) && (m_zaBinStatus[i] == true)) {
      m_zaIsButtonPressed[i] = true;
      Serial.println("Button Was Pressed : " + String(i));
      Serial.println(m_zaIsButtonPressed[i]);
    }
  }
}

//Complete Transaction Request Function
int checkRtuState() {

  int l_iAnyDeviceChange = 0;

  for (int i = 0; i < REMOTE_UNIT_AMOUNT; i++) {

  // RTU just connected and last state is not connected
    if (digitalRead(PIN_DETECT_RU[i]) == LOW && m_zaIsRtuConnected[i] == false){
      m_zaIsRtuConnected[i] = true;
      Serial.println("RTU Was Connect : " + String(i));
      Serial.println(m_zaIsRtuConnected[i]);
      l_iAnyDeviceChange++;
    }

    // RTU just disconnected and last state is connected
    if (digitalRead(PIN_DETECT_RU[i]) == HIGH && m_zaIsRtuConnected[i] == true) {
      m_zaIsRtuConnected[i] = false;
      Serial.println("RTU Was Disconnect : " + String(i));
      Serial.println(m_zaIsRtuConnected[i]);
      l_iAnyDeviceChange++;
    }
  }

  Serial.println("How many connected/disconnected devices: " + String(l_iAnyDeviceChange));
  return l_iAnyDeviceChange;
}
