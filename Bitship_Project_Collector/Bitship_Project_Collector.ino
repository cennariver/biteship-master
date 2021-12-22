// include the library code:
#include <LiquidCrystal.h>
#include <ArduinoJson.h>
#include <WiFiEsp.h>


/** PIN Setting */
const uint8_t     REMOTE_UNIT_AMOUNT = 11;
const uint8_t     PIN_D4             = 25;
const uint8_t     PIN_D5             = 24;
const uint8_t     PIN_D6             = 23;
const uint8_t     PIN_D7             = 22;
LiquidCrystal m_oaRtu[REMOTE_UNIT_AMOUNT] = {LiquidCrystal (27, 26, PIN_D4, PIN_D5, PIN_D6, PIN_D7), // not used, NULL not supported
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
const uint8_t PIN_LED[REMOTE_UNIT_AMOUNT]        = {NULL,
                                                    47, 46, 50, 52, 17, 16, 15, 14, 48, 49
                                                   };
//Pin for Push Button
const uint8_t PIN_BUTTON[REMOTE_UNIT_AMOUNT]     = {NULL,
                                                    2, 3, 4, 5, 6, 7, 8, 9, 10, 11
                                                   };
bool m_zaIsButtonPressed[REMOTE_UNIT_AMOUNT]     = {NULL,
                                                    false, false, false, false, false,
                                                    false, false, false, false, false
                                                   };
//Pin for RTU Detector
const uint8_t PIN_DETECT_RU[REMOTE_UNIT_AMOUNT]  = {NULL,
                                                    A15, A14, A13, A12, A11, A10, A9, A8, A7, A6
                                                   };
bool m_zaIsRtuConnected[REMOTE_UNIT_AMOUNT]      = {NULL,
                                                    false, false, false, false, false,
                                                    false, false, false, false, false
                                                   };


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
const unsigned long WIFI_TIMEOUT  = 5000;
uint8_t m_iStatus = WL_IDLE_STATUS;
WiFiEspClient m_oClient;


/** Client */
const String   COLLECTOR_IDENTIFIER            = "2";
const String   API_REGISTER_RU                 = "/ru-registration-collector/" + COLLECTOR_IDENTIFIER;
const String   API_GET_CURRENT_DATA            = "/get-current-data-by-collector/" + COLLECTOR_IDENTIFIER;
const String   API_GET_TRANSACTION             = "/get-transaction-by-collector/" + COLLECTOR_IDENTIFIER;
const String   API_TRANSACTION_CONFIRM         = "/confirm-on-process-by-collector/" + COLLECTOR_IDENTIFIER;
const String   API_PICKING_CONFIRM             = "/confirm-done?bin_id=";
const String   API_TRANSACTION_PICKING_1stPart = "/confirm-done?transaction_id=";
const String   API_TRANSACTION_PICKING_2ndPart = "&device_id=";
//receive global variable
const int RECEIVED_CHAR_LENGTH         = 1500;
char m_caReceivedChars[RECEIVED_CHAR_LENGTH];
String m_strRcvSendBuffer;
bool m_zNewData = false;
//const char     HOST_ADDRESS[]  = "192.168.0.6";
const char     HOST_ADDRESS[]  = "192.168.1.5";
const int      HOST_PORT       = 3000;
//rtu state
uint8_t m_iaRtuState;
const uint8_t  RU_STATE_REGISTRATION              = 0;
const uint8_t  RU_STATE_READY                     = RU_STATE_REGISTRATION + 1;
const uint8_t  RU_STATE_IDLE                      = RU_STATE_READY + 1;
const uint8_t  RU_STATE_TRANSACTION_CONFIRMATION  = RU_STATE_IDLE + 1;
const uint8_t  RU_STATE_TRANSACTION_PICKING       = RU_STATE_TRANSACTION_CONFIRMATION + 1;
//RU state-register variable
const uint8_t  LCD_PRINT_UNREGISTERED             = 0;
const uint8_t  LCD_PRINT_REGISTERING              = LCD_PRINT_UNREGISTERED + 1;
const uint8_t  LCD_PRINT_REGISTERED               = LCD_PRINT_REGISTERING + 1;
bool m_zaRtuRegistered[REMOTE_UNIT_AMOUNT] = {NULL,
                                              false, false, false, false, false,
                                              false, false, false, false, false
                                             };
//RU state-ready variable
String m_straSkuName[REMOTE_UNIT_AMOUNT];
int m_iaSkuQty[REMOTE_UNIT_AMOUNT];
//RU state-idle variable
const unsigned long  PERIOD_TIME_GET_ACTIVE_TRANSACTION = 6000;
unsigned long m_lLastGetActiveTransactionApi = 0;
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
const uint8_t     DESERIALIZE_JSON_OK                 = 0;
const uint8_t     DESERIALIZE_JSON_EMPTY_INPUT        = 1;
const uint8_t     DESERIALIZE_JSON_INCOMPLETE_OUTPUT  = 2;
const uint8_t     DESERIALIZE_JSON_INVALID_INPUT      = 3;
const uint8_t     DESERIALIZE_JSON_NO_MEMORY          = 4;
const uint8_t     DESERIALIZE_JSON_TOO_DEEP           = 5;

void setup() {

  //Setting input and output for the collector
  for (uint8_t i = 1; i < REMOTE_UNIT_AMOUNT; i++) {

    //Activate Pull UP for Push Button and RTU detector, default: high, pressed: low
    pinMode(PIN_BUTTON[i], INPUT_PULLUP);
    pinMode(PIN_DETECT_RU[i], INPUT_PULLUP);
    //Setting Output for LED Relay
    pinMode(PIN_LED[i], OUTPUT);
    //Initialize LCD as display output
    m_oaRtu[i].begin(16, 2);
  }

  //Initiate Serial Communication
  //For Arduino Serial
  Serial.begin(115200);
  //Clear Buffer
  clearBuffer();

  //Initiate ESP8266 Startup
  Wifi_ConnectToNetwork();

  //set rtu connected state
  setRtuState();

  setLedOutput(true);
  clearBinStatus();
}

void loop() {

  m_ulCurrentMillis = millis();
  Serial.println("WiFi:" + ((WiFi.status() == WL_CONNECTED) ? "WL_CONNECTED" : String(WiFi.status())) + " | RTU:" + String(m_iaRtuState));

  //handling disconnected wifi
  if (WiFi.status() != WL_CONNECTED) {
    Wifi_ReconnectToNetwork();
  }

  //process
  switch (m_iaRtuState) {

    case RU_STATE_REGISTRATION:
      //display to LCD: registering device
      Lcd_PrintRegisteringDevice();

      //construct registering device in json format
      m_strRcvSendBuffer = "{\"collector\": " + COLLECTOR_IDENTIFIER + ", \"status\": [";
      for (uint8_t i = 1; i < REMOTE_UNIT_AMOUNT; i++) {
        m_strRcvSendBuffer += (i != (REMOTE_UNIT_AMOUNT - 1)) ? (String)m_zaIsRtuConnected[i] + "," : (String)m_zaIsRtuConnected[i];
      }
      m_strRcvSendBuffer += "]}";

      //post request to API register
      if (Client_HttpPostRequest(API_REGISTER_RU, m_strRcvSendBuffer)) {
        //receive serial data
        Serial.println("post request success API_REGISTER_RU");
        m_strRcvSendBuffer = Client_ReadSerialData('<', '>');
        if (m_strRcvSendBuffer != "") {
          //parsed confirmation data
          Serial.println("some data available");
          if (Client_JsonParseRegisterConfirmation()) {
            //continue to ready state
            Serial.println("registered");
            m_iaRtuState = RU_STATE_READY;
          }
          clearBuffer();
        }
      }
      break;

    case RU_STATE_READY:
      //display to LCD: registering device
      Lcd_PrintGettingBinData();

      //get request from API current data
      if (Client_HttpGetRequest(API_GET_CURRENT_DATA)) {
        //receive serial data
        Serial.println("get request success API_GET_CURRENT_DATA");
        m_strRcvSendBuffer = Client_ReadSerialData('<', '>');
        if (m_strRcvSendBuffer != "") {
          Serial.println("some data available");
          //parsed bin data
          if (Client_JsonParseCurrentData()) {
            //continue to idle state
            Serial.println("parsed bin data success");
            m_iaRtuState = RU_STATE_IDLE;

            //set rtu connected state
            setRtuState();
            setLedOutput(false);
          }
          clearBuffer();
        }
      }
      break;

    case RU_STATE_IDLE:
      //display to LCD: bin stock data
      Lcd_PrintCurrentBinData();

      //back to device registration if any device was connected/disconnected
      if (isRtuStateChanged() != 0 ||
          buttonWasPressed() != 0) {
        m_iaRtuState = RU_STATE_REGISTRATION;
        setLedOutput(true);
        clearBinStatus();
        break;
      }

      //request every specific period
      if (m_ulCurrentMillis - m_lLastGetActiveTransactionApi >= PERIOD_TIME_GET_ACTIVE_TRANSACTION) {
        //get request from API transaction data
        if (Client_HttpGetRequest(API_GET_TRANSACTION)) {
          //receive serial data
          Serial.println("get request success API_GET_TRANSACTION");
          m_strRcvSendBuffer = Client_ReadSerialData('<', '>');
          if (m_strRcvSendBuffer != "") {
            //parsed transaction data
            Serial.println("some data available");
            if (Client_JsonParseTransactionData()) {
              //continue to transaction state
              Serial.println("transaction active");
              m_iaRtuState = RU_STATE_TRANSACTION_CONFIRMATION;
            }
            clearBuffer();
          }
        }
        m_lLastGetActiveTransactionApi = m_ulCurrentMillis;
      }
      break;

    case RU_STATE_TRANSACTION_CONFIRMATION:
      //display to LCD: transaction status
      Lcd_PrintTransactionState(false);

      //get request from API transaction confirmation data
      if (Client_HttpGetRequest(API_TRANSACTION_CONFIRM)) {
        //receive serial data
        Serial.println("get request success API_TRANSACTION_CONFIRM");
        m_strRcvSendBuffer = Client_ReadSerialData('<', '>');
        if (m_strRcvSendBuffer != "") {
          //parsed transaction confirmation data and continue to next state
          Serial.println("some data available");
          if (Client_JsonParseTransactionConfirmation()) {
            //continue to idle state
            Serial.println("parsed transaction confirmation success");
            m_iaRtuState = RU_STATE_TRANSACTION_PICKING;
          }
          clearBuffer();
        }
      }
      break;

    case RU_STATE_TRANSACTION_PICKING:
      //display to LCD: transaction status and get rtu that need to be done
      uint8_t l_iActiveTransaction = Lcd_PrintTransactionState(true);
      isPushButtonPressed();

      //process all done transaction for each bin
      for (uint8_t i = 1; i < REMOTE_UNIT_AMOUNT; i++) {
        if (m_zaIsButtonPressed[i] == true) {
          Lcd_Print(i, "Send Done Status", "On loading. . .");

          //get request from API transaction picking data
          if (Client_HttpGetRequest(API_TRANSACTION_PICKING_1stPart + m_strTransactionId + API_TRANSACTION_PICKING_2ndPart + m_straBinId[i])) {
            //receive serial data
            Serial.println("get request success API_TRANSACTION_PICKING");
            m_strRcvSendBuffer = Client_ReadSerialData('<', '>');
            if (m_strRcvSendBuffer != "") {
              //parsed transaction picking data and continue to next state
              Serial.println("some data available");
              if (Client_JsonParsePickingsConfirmation(i)) {
                // transaction picking for specific bin is done
                Serial.println("transaction done for RTU " + String(i));
                m_zaBinStatus[i] = false;
              }
              clearBuffer();
            }
          }
          m_zaIsButtonPressed[i] = false;
        }
      }

      //transaction done
      if (l_iActiveTransaction == 0) {
        clearBinStatus();
        //back to IDLE state
        m_iaRtuState = RU_STATE_IDLE;
      }
      break;
  }
}


/** LCD Related Function */
//Function print to specific LCD
void Lcd_Print(uint8_t p_iBinId, String p_strFirstRow, String p_strSecondRow) {

  m_oaRtu[p_iBinId].begin(16, 2);
  m_oaRtu[p_iBinId].clear();
  m_oaRtu[p_iBinId].setCursor(0, 0);
  m_oaRtu[p_iBinId].print(p_strFirstRow);
  m_oaRtu[p_iBinId].setCursor(0, 1);
  m_oaRtu[p_iBinId].print(p_strSecondRow);
}

//Function print to all LCD
void Lcd_Print(String p_strFirstRow, String p_strSecondRow) {

  for (uint8_t i = 1; i < REMOTE_UNIT_AMOUNT; i++) {
    Lcd_Print(i, p_strFirstRow, p_strSecondRow);
  }
}

void Lcd_PrintCantConnectToRaspi() {
  Lcd_Print("Connection fail", "Try again in 1s");
  //give delay to try again
  delay(1000);
}

void Lcd_PrintDeviceRegisState(uint8_t p_iBinId, uint8_t p_iPrintState) {
  switch (p_iPrintState) {
    case LCD_PRINT_UNREGISTERED:
      Lcd_Print(p_iBinId, "RU: C" + COLLECTOR_IDENTIFIER + "B" + String(p_iBinId), "Unregistered");
      break;
    case LCD_PRINT_REGISTERING:
      Lcd_Print(p_iBinId, "RU: C" + COLLECTOR_IDENTIFIER + "B" + String(p_iBinId), "Registering");
      break;
    case LCD_PRINT_REGISTERED:
      Lcd_Print(p_iBinId, "RU: C" + COLLECTOR_IDENTIFIER + "B" + String(p_iBinId), "Registered");
      break;

    default:
      Lcd_Print(p_iBinId, "RU: C" + COLLECTOR_IDENTIFIER + "B" + String(p_iBinId), "Unknown");
      break;
  }
}

void Lcd_PrintRegisteringDevice() {
  for (uint8_t i = 1; i < REMOTE_UNIT_AMOUNT; i++) {
    if (m_zaRtuRegistered[i]) {
      Lcd_PrintDeviceRegisState(i, LCD_PRINT_REGISTERED);
    }
    else {
      Lcd_PrintDeviceRegisState(i, LCD_PRINT_REGISTERING);
    }
  }
}

void Lcd_PrintGettingBinData() {
  for (uint8_t i = 1; i < REMOTE_UNIT_AMOUNT; i++) {
    if (m_zaRtuRegistered[i]) {
      Lcd_Print(i, "Getting bin data", "C" + COLLECTOR_IDENTIFIER + "B" + String(i) + " Please Wait");
    }
    else {
      Lcd_PrintDeviceRegisState(i, LCD_PRINT_UNREGISTERED);
    }
  }
}

void Lcd_PrintCurrentBinData(uint8_t p_iaBinId) {
  if (m_zaRtuRegistered[p_iaBinId]) {
    m_oaRtu[p_iaBinId].begin(16, 2);
    m_oaRtu[p_iaBinId].clear();
    m_oaRtu[p_iaBinId].setCursor(0, 0);
    m_oaRtu[p_iaBinId].print("SKU:" + m_straSkuName[p_iaBinId]);
    m_oaRtu[p_iaBinId].setCursor(0, 1);
    m_oaRtu[p_iaBinId].print("Qty:" + String(m_iaSkuQty[p_iaBinId]));
    m_oaRtu[p_iaBinId].setCursor(10, 1);
    m_oaRtu[p_iaBinId].print("C" + COLLECTOR_IDENTIFIER + "B" + String(p_iaBinId));
  }
  else {
    Lcd_PrintDeviceRegisState(p_iaBinId, LCD_PRINT_UNREGISTERED);
  }
}

void Lcd_PrintCurrentBinData() {
  for (uint8_t i = 1; i < REMOTE_UNIT_AMOUNT; i++) {
    if (m_zaRtuRegistered[i]) {
      Lcd_PrintCurrentBinData(i);
    }
    else {
      Lcd_PrintDeviceRegisState(i, LCD_PRINT_UNREGISTERED);
    }
  }
}

int Lcd_PrintTransactionState(bool p_bIsExecutionState) {

  int l_iActiveTransaction = 0;

  for (uint8_t i = 1; i < REMOTE_UNIT_AMOUNT; i++) {
    if (m_zaRtuRegistered[i]) {
      if (m_zaBinStatus[i]) {
        if (p_bIsExecutionState) {
          Serial.println("active transaction bin: " + String(i));

          m_oaRtu[i].begin(16, 2);
          m_oaRtu[i].clear();
          m_oaRtu[i].setCursor(0, 0);
          m_oaRtu[i].print("Act:" + m_straActionCode[i]);
          m_oaRtu[i].setCursor(0, 1);
          m_oaRtu[i].print("Qty:" + String(m_iaActionQty[i]));
          m_oaRtu[i].setCursor(10, 1);
        } else {
          m_oaRtu[i].begin(16, 2);
          m_oaRtu[i].clear();
          m_oaRtu[i].setCursor(0, 0);
          m_oaRtu[i].print("Transaction on");
          m_oaRtu[i].setCursor(0, 1);
          m_oaRtu[i].print("confirmation ...");
        }
        m_oaRtu[i].print("C" + COLLECTOR_IDENTIFIER + "B" + String(i));
        digitalWrite(PIN_LED[i], HIGH); //LED ON

        l_iActiveTransaction++;
      }
      else {
        Lcd_PrintCurrentBinData(i);
        digitalWrite(PIN_LED[i], LOW); //LED OFF
      }

    } else {
      Lcd_PrintDeviceRegisState(i, LCD_PRINT_UNREGISTERED);
    }
  }

  Serial.println("active transaction left: " + String(l_iActiveTransaction));
  return l_iActiveTransaction;
}

/** WIFI ESP Function **/
//Connect to your wifi network
void Wifi_ConnectToNetwork() {

  Lcd_Print("Searching WiFi", "SSID:" + String(WIFI_SSID));
  //For ESP01
  Serial1.begin(115200);
  Serial1.setTimeout(WIFI_TIMEOUT);
  WiFi.init(&Serial1);

  // attempt to connect to WiFi network
  while (m_iStatus != WL_CONNECTED) {
    Serial.print("Attempting to connect to WPA SSID: ");
    Serial.println(WIFI_SSID);

    // Connect to WPA/WPA2 network
    m_iStatus = WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  }
  Lcd_Print("WiFi Connected", "SSID:" + String(WIFI_SSID));
  Serial.println("You're connected to the network");
}


void Wifi_ReconnectToNetwork() {

  Lcd_Print("Searching WiFi", "SSID:" + String(WIFI_SSID));

  //initialize wifi
  WiFi.init(&Serial1);
  WiFi.reset();

  // Connect to WPA/WPA2 network
  Serial.print("Attempting to connect to WPA SSID: ");
  Serial.println(WIFI_SSID);
  m_iStatus = WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  Lcd_Print("WiFi Connected", "SSID:" + String(WIFI_SSID));
  Serial.println("You're connected to the network");

}


/** Client **/
bool Client_HttpPostRequest (String p_strQuery, String p_strPayload) {

  //Close All Socket
  m_oClient.stop();
  Serial.println("Attempting to connect to raspi API: " + String(HOST_ADDRESS));

  // if there's a successful connection
  if (m_oClient.connect(HOST_ADDRESS, HOST_PORT)) {
    Serial.println("Connected");
    Serial.println(p_strQuery);

    // send the HTTP GET request
    m_oClient.println(("POST " + p_strQuery + " HTTP/1.1"));
    m_oClient.println(("Host: " + String(HOST_ADDRESS)));
    m_oClient.println("Connection: keep-alive");
    m_oClient.println("Accept: application/json");
    m_oClient.println(("Content-Type: application/json"));
    m_oClient.println("Content-Length: " + String(p_strPayload.length() + 1));
    m_oClient.println();
    m_oClient.println((p_strPayload));
    Serial.println((p_strPayload));

    if (m_oClient.status() == ESTABLISHED) {
      return true;
    }
  }

  Serial.println("Post failed, client status: " + String(m_oClient.status()));
  Lcd_PrintCantConnectToRaspi();
  return false;
}

bool Client_HttpGetRequest (String p_strQuery) {

  m_oClient.stop();
  Serial.println("Attempting to connect to raspi API: " + String(HOST_ADDRESS));

  // if there's a successful connection
  if (m_oClient.connect(HOST_ADDRESS, HOST_PORT)) {
    Serial.println("Connected");
    // send the HTTP GET request
    m_oClient.println(("GET " + p_strQuery + " HTTP/1.1"));
    Serial.println(p_strQuery);
    m_oClient.println(("Host: " + String(HOST_ADDRESS)));
    m_oClient.println("Connection: close");
    m_oClient.println();

    return true;
  }

  Serial.println("Get failed, client status: " + String(m_oClient.status()));
  Lcd_PrintCantConnectToRaspi();
  return false;
}

String Client_ReadSerialData(char p_chStartMarker, char p_chEndMarker) {

  static boolean l_zRecvInProgress = false;
  static int l_iNdx = 0;
  char l_cReadChar;
  int l_iSerialLen = 0;
  unsigned long l_lLastReadSerialData = millis();

  //waiting for some data
  while (!m_oClient.available()) {

    // avoid infinite loop, declare timeout
    if (millis() - l_lLastReadSerialData >= WIFI_TIMEOUT) {
      clearBuffer();
      Lcd_Print("Connection fail", "Try again in 1s");
      break;
    }
  };

  //receive serial data if available
  if (m_oClient.available() > 0) {
    //get length of incoming data
    l_iSerialLen = m_oClient.available();

    //get using for loop
    for (int i = 0; i < l_iSerialLen; i++) {
      l_cReadChar = m_oClient.read();

      if (l_zRecvInProgress == true) {
        if (l_cReadChar != p_chEndMarker) {
          m_caReceivedChars[l_iNdx] = l_cReadChar;
          l_iNdx++;
          if (l_iNdx >= RECEIVED_CHAR_LENGTH) {
            l_iNdx = RECEIVED_CHAR_LENGTH - 1;
          }
        }
        else {
          m_caReceivedChars[l_iNdx] = '\0'; // terminate the string
          l_zRecvInProgress = false;
          l_iNdx = 0;
          m_zNewData = true;
        }
      }
      else if (l_cReadChar == p_chStartMarker) {
        l_zRecvInProgress = true;
      }
    }
  }

  if (m_zNewData == true) {
    Serial.println(m_caReceivedChars);
    m_zNewData = false;
  }

  return m_caReceivedChars;
}

bool Client_JsonParseRegisterConfirmation() {

  StaticJsonDocument<RECEIVED_CHAR_LENGTH> l_oParsedData;
  DeserializationError l_oError = deserializeJson(l_oParsedData, m_strRcvSendBuffer);

  if (l_oError == DESERIALIZE_JSON_OK) {
    //Status Data
    if (l_oParsedData["success"]) {
      //Data Object (10 RU , SKU Name, Bin ID,
      JsonArray l_oaRegisteredState = l_oParsedData["registered_bin"].as<JsonArray>();

      String l_strIsRtuConnected = "";
      String l_strRtuRegistered = "";

      //Parsing Array Data For 10 RU
      for (int i = 1; i < REMOTE_UNIT_AMOUNT; i++) {
        bool l_zRegisteredState = l_oaRegisteredState[i - 1].as<boolean>();
        //store to global var
        m_zaRtuRegistered[i] = l_zRegisteredState;
        //construct string
        l_strIsRtuConnected += String(m_zaIsRtuConnected[i]);
        l_strRtuRegistered += String(m_zaRtuRegistered[i]);
      }

      // check RU status between connected and disconnected device
      for (int i = 1; i < REMOTE_UNIT_AMOUNT; i++) {
        /**
           | Connected  | Registered  | Remarks
           | 0          | 0           | OK, device not exist
           | 0          | 1           | NOK, waiting for device to be connected
           | 1          | 0           | NOK, waiting for device to be registered
           | 1          | 1           | OK, device ready
        */
        if (m_zaIsRtuConnected[i] != m_zaRtuRegistered[i]) {
          //try again in 1 second
          Serial.println("some device is not connected/registered");
          Serial.println("Con:" + l_strIsRtuConnected);
          Serial.println("Reg:" + l_strRtuRegistered);
          Lcd_Print("Con:" + l_strIsRtuConnected, "Reg:" + l_strRtuRegistered);
          delay(1000);
          return false;
        }
      }

      return true;
    }
  }

  Serial.println("Error Parsing Register Confirmation, error code ");
  printDeserializeJsonErrorCode(l_oError);
  return false;
}

//Parsing CurrentData
bool Client_JsonParseCurrentData() {
  StaticJsonDocument<RECEIVED_CHAR_LENGTH> l_oDoc;
  DeserializationError l_oError = deserializeJson(l_oDoc, m_strRcvSendBuffer);

  if (l_oError == DESERIALIZE_JSON_OK) {
    //Status Data
    bool l_zStatus = l_oDoc["success"];

    //Function Code
    String l_strMessage = l_oDoc["message"];

    //Data Object (10 RU , SKU Name, Bin ID,
    JsonObject l_oData = l_oDoc["data"];
    JsonArray l_oaArrayData = l_oDoc["data"].as<JsonArray>();

    //Parsing Array Data For 10 SKU
    for (int i = 1; i < REMOTE_UNIT_AMOUNT; i++) {
      JsonObject l_oSku = l_oaArrayData[i - 1];
      String l_strDeviceId = (const char*)l_oSku["device_id"];
      int l_iBinId = l_strDeviceId.substring(3).toInt();
      String l_strSkuName = (const char*)l_oSku["sku_name"];
      int l_iSkuQty = (int)l_oSku["quantity"];

      // save to global variable
      m_straSkuName[l_iBinId] = l_strSkuName;
      m_iaSkuQty[l_iBinId] = l_iSkuQty;
    }

    return true;
  }

  Serial.print("Error Parsing Current Data, error code ");
  printDeserializeJsonErrorCode(l_oError);
  return false;
}

//Parsing transaction data
bool Client_JsonParseTransactionData() {
  bool l_zStatus = false;
  StaticJsonDocument<RECEIVED_CHAR_LENGTH> l_oParsedData;
  DeserializationError l_oError = deserializeJson(l_oParsedData, m_strRcvSendBuffer);

  if (l_oError == DESERIALIZE_JSON_OK) {
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

      for (uint8_t i = 1; i < REMOTE_UNIT_AMOUNT; i++) {
        JsonObject l_oJsonTransactionData = l_oaArrayTransaction[i - 1];
        //Get Local Var
        String l_strBinId = (const char*)l_oJsonTransactionData["device_id"];
        String l_strActionCode = (const char*)l_oJsonTransactionData["action"];
        int l_iActionQty = (int)l_oJsonTransactionData["quantity"];
        //Decode bin ID
        int l_iBinId = l_strBinId.substring(3).toInt();
        // int ll_bin_id = l_oaJsonTransactionData["bin_id"];

        // save to global variable
        m_zaBinStatus[l_iBinId] = true;
        m_straBinId[l_iBinId] = l_strBinId;
        m_straActionCode[l_iBinId] = l_strActionCode;
        m_iaActionQty[l_iBinId] = l_iActionQty;
      }
    }
    return l_zStatus;
  }

  Serial.println("Error Parsing Transaction Data, error code ");
  printDeserializeJsonErrorCode(l_oError);
  return false;
}

//Parsing CurrentData
bool Client_JsonParseTransactionConfirmation() {
  StaticJsonDocument<RECEIVED_CHAR_LENGTH> l_oParsedData;
  DeserializationError l_oError = deserializeJson(l_oParsedData, m_strRcvSendBuffer);

  if (l_oError == DESERIALIZE_JSON_OK) {
    //Parsing the Data And Move to Global Var
    //Transaction_Status
    JsonObject l_oJsonTransactionData = l_oParsedData["data"];
    m_strTransactionExecution = (const char*)l_oJsonTransactionData["status"];

    return true;
  }

  Serial.println("Error Parsing Transaction Confirmation, error code ");
  printDeserializeJsonErrorCode(l_oError);
  return false;
}

bool Client_JsonParsePickingsConfirmation(int p_iBinIds) {
  StaticJsonDocument<RECEIVED_CHAR_LENGTH> l_oParsedData;
  DeserializationError l_oError = deserializeJson(l_oParsedData, m_strRcvSendBuffer);

  if (l_oError == DESERIALIZE_JSON_OK) {
    //Parsing the Data And Move to Global Var
    //Transaction_Status
    bool l_zTransStatus = l_oParsedData["success"];
    m_zConfirmationStatus = l_zTransStatus;

    if (l_zTransStatus == true) {
      JsonObject l_oJsonTransactionData = l_oParsedData["data"];
      m_zaBinStatus[p_iBinIds] = false;
      //        m_straSkuName[p_iBinIds]=(const char*)l_oaJsonTransactionData["sku_name"];
      m_iaSkuQty[p_iBinIds] = (int)l_oJsonTransactionData["quantity"];
      Serial.println(m_zaBinStatus[p_iBinIds]);
      //        Serial.println(m_straSkuName[p_iBinIds]);
    }

    return m_zConfirmationStatus;
  }

  Serial.println("Error Parsing Picking Confirmation, error code ");
  printDeserializeJsonErrorCode(l_oError);
  return false;
}


/** Other Function **/
void printDeserializeJsonErrorCode(DeserializationError p_oError) {

  if (p_oError == DESERIALIZE_JSON_OK) {
    Serial.println("DESERIALIZE_JSON_OK");
  } else if (p_oError == DESERIALIZE_JSON_EMPTY_INPUT) {
    Serial.println("DESERIALIZE_JSON_EMPTY_INPUT");
  } else if (p_oError == DESERIALIZE_JSON_INCOMPLETE_OUTPUT) {
    Serial.println("DESERIALIZE_JSON_INCOMPLETE_OUTPUT");
  } else if (p_oError == DESERIALIZE_JSON_INVALID_INPUT) {
    Serial.println("DESERIALIZE_JSON_INVALID_INPUT");
  } else if (p_oError == DESERIALIZE_JSON_NO_MEMORY) {
    Serial.println("DESERIALIZE_JSON_NO_MEMORY");
  } else if (p_oError == DESERIALIZE_JSON_TOO_DEEP) {
    Serial.println("DESERIALIZE_JSON_TOO_DEEP");
  } else {
    Serial.println("DESERIALIZE_JSON_UNKNOWN");
  }

  Lcd_PrintCantConnectToRaspi();
}

void clearBinStatus() {

  //reset all values
  for (uint8_t i = 1; i < REMOTE_UNIT_AMOUNT; i++) {
    m_zaBinStatus[i] = false;
    m_straBinId[i] = "";
    m_straActionCode[i] = "";
    m_iaActionQty[i] = 0;
  }
  m_strTransactionExecution = "";
  m_zConfirmationStatus = false;
}

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

  //clear global variable that has been used for receive serial data
  for (int i = 0; i < RECEIVED_CHAR_LENGTH; i++) {
    m_caReceivedChars[i] = NULL;
  }
  m_strRcvSendBuffer = "";
}

//Read Push Button
bool buttonWasPressed(uint8_t p_iPinButton) {
  bool l_zStatusButton = false;

  if (digitalRead(p_iPinButton) == LOW) {
    l_zStatusButton = true;
  } else {
    l_zStatusButton = false;
  }

  return l_zStatusButton;
}

int buttonWasPressed() {

  uint8_t l_iCounter = 0;

  for (uint8_t i = 1; i < REMOTE_UNIT_AMOUNT; i++) {
    if (buttonWasPressed(i)) {
      l_iCounter++;
    }
  }

  return l_iCounter;
}

//Complete Transaction Request Function
void isPushButtonPressed() {
  //Serial.println("Button Pressed");
  for (uint8_t i = 1; i < REMOTE_UNIT_AMOUNT; i++) {
    //Check Button are Pressed or Not
    if (buttonWasPressed(PIN_BUTTON[i]) && (m_zaBinStatus[i] == true)) {
      m_zaIsButtonPressed[i] = true;
      Serial.println("Button Was Pressed : " + String(i));
      Serial.println(m_zaIsButtonPressed[i]);
    }
  }
}

//Complete Transaction Request Function
int isRtuStateChanged() {

  int l_iAnyDeviceChange = 0;

  for (uint8_t i = 1; i < REMOTE_UNIT_AMOUNT; i++) {

    // RTU just connected and last state is not connected
    if (digitalRead(PIN_DETECT_RU[i]) == LOW && m_zaIsRtuConnected[i] == false) {
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

  //  Serial.println("How many devices that was just connected/disconnected: " + String(l_iAnyDeviceChange));
  return l_iAnyDeviceChange;
}

void setRtuState() {

  for (uint8_t i = 1; i < REMOTE_UNIT_AMOUNT; i++) {
    // Check the availability of RTU
    if (digitalRead(PIN_DETECT_RU[i]) == LOW) {
      // RTU connected
      m_zaIsRtuConnected[i] = true;
      //      Serial.println("Connect: C" + COLLECTOR_IDENTIFIER + "B" + String(i+1));
    }
    else {
      // RTU disconnected
      m_zaIsRtuConnected[i] = false;
      //      Serial.println("Disonnect: C" + COLLECTOR_IDENTIFIER + "B" + String(i+1));
    }
  }
}

void setLedOutput(bool p_zOutput) {

  for (uint8_t i = 1; i < REMOTE_UNIT_AMOUNT; i++) {

    if (m_zaIsRtuConnected[i] == true) {
      digitalWrite(PIN_LED[i], p_zOutput ? HIGH : LOW);
    }
  }
}
