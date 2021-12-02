// include the library code:
#include <LiquidCrystal.h>
#include <ArduinoJson.h>
#include "WiFiEsp.h"


//Delay
unsigned long startMillis;  //some global variables available anywhere in the program
unsigned long currentMillis;
const unsigned long period = 6000;


//WIFI
//char ssid[] = "tamsos blok HG 25A";            // your network SSID (name)
//char pass[] = "qwerty123456";        // your network password
//char ssid[] = "c7";            // your network SSID (name)
//char pass[] = "da645591";        // your network password
char ssid[] = "Warehouse Biteship";            // your network SSID (name)
char pass[] = "Bismilahlancar";        // your network password
//char ssid[] = "Diskum_723";            // your network SSID (name)
//char pass[] = "19283746abcd";  


int status = WL_IDLE_STATUS;     // the Wifi radio's status
//char server[] = "061e-182-2-170-99.ngrok.io";
//String host = "061e-182-2-170-99.ngrok.io"; 
char server[] = "192.168.0.6";
String host = "192.168.0.6"; 
String atResponse;
unsigned long lastConnectionTime = 0;         // last time you connected to the server, in milliseconds
const unsigned long postingInterval = 10000L; // delay between updates, in milliseconds

// Initialize the Ethernet client object
WiFiEspClient client;

//LCD Intialization
//int rtuLcdContrast [10] = {100,100,100,100,100,100,100,100,100,100};
const int d4 = 25, d5 = 24, d6 = 23, d7 = 22;
//Pin : rs,en,d4,d5,d6,d7
LiquidCrystal rtu_1_LCD(27, 26, d4, d5, d6, d7);
LiquidCrystal rtu_2_LCD(29, 28, d4, d5, d6, d7);
LiquidCrystal rtu_3_LCD(31, 30, d4, d5, d6, d7);
LiquidCrystal rtu_4_LCD(33, 32, d4, d5, d6, d7);
LiquidCrystal rtu_5_LCD(35, 34, d4, d5, d6, d7);
LiquidCrystal rtu_6_LCD(37, 36, d4, d5, d6, d7);
LiquidCrystal rtu_7_LCD(39, 38, d4, d5, d6, d7);
LiquidCrystal rtu_8_LCD(41, 40, d4, d5, d6, d7);
LiquidCrystal rtu_9_LCD(43, 42, d4, d5, d6, d7);
LiquidCrystal rtu_10_LCD(45, 44, d4, d5, d6, d7);
//Pin for LED (Pin LED RU1, Pin LED RU2,...)
const int pinLED[11]={0,47,46,50,52,17,16,15,14,48,49};
//Pin for Push Button
const int pinButton[11]={0,2,3,4,5,6,7,8,9,10,11};
//Button Virtual Latch
bool buttonDeactive[11]={false,false,false,false,false,false,false,false,false,false,false};
//API List
String APIgetCurrentData="/get-current-data-by-collector/1";
String APIgetTransaction="/get-transaction-by-collector/1";
String APItransactionConfirm="/confirm-on-process-by-collector/1";
String APIpickingsConfirm="/confirm-done?bin_id=";
//RU Var Init
String SKU_name[11];
int SKU_qty[11];
//RU Transaction Data
bool transaction_status=false;
String transaction_id;
String transaction_execution;
bool bin_status[11];
String bin_id[11];
String action_code[11];
int action_qty[11];
bool transaction_updated=false;
bool confirmation_status=false;

//API Data
String rawData;
String rawStockData;
String rawTransactionData;
String rawTransactionConfirmation;
String rawPickingsConfirmation;
//Receive Serial
boolean newData = false;
bool firstrun=false;
const int numChars = 1500;
char receivedChars[numChars];
bool waitSerial[10];
bool waitSerialCommand[10];



void setup() {
  //Setting UP Pin Mode
  for(int i=1;i<=10;i++){
    //Activate Pull UP for Push Button
    pinMode(pinButton[i],INPUT_PULLUP);
    Serial.println(pinButton[i]);
    //Setting Output for LED Relay
    pinMode(pinLED[i],OUTPUT);  
  }
  startMillis = millis();
//Initiate LCD (Contrast)
 // analogWrite(6,rtuLcdContrast[1]);
//Initialize LCD
   rtu_1_LCD.begin(16, 2);
  rtu_2_LCD.begin(16, 2);
  rtu_3_LCD.begin(16, 2);
  rtu_4_LCD.begin(16, 2);
  rtu_5_LCD.begin(16, 2);
  rtu_6_LCD.begin(16, 2);
  rtu_7_LCD.begin(16, 2);
  rtu_8_LCD.begin(16, 2);
  rtu_9_LCD.begin(16, 2);
  rtu_10_LCD.begin(16, 2);
//Wait For Running Code
  delay(100);
LCDprint(rtu_1_LCD,"Intialization","RU-1 Please Wait");
  LCDprint(rtu_2_LCD,"Intialization","RU-2 Please Wait");
  LCDprint(rtu_3_LCD,"Intialization","RU-3 Please Wait");
  LCDprint(rtu_4_LCD,"Intialization","RU-4 Please Wait");
  LCDprint(rtu_5_LCD,"Intialization","RU-5 Please Wait");
  LCDprint(rtu_6_LCD,"Intialization","RU-6 Please Wait");
  LCDprint(rtu_7_LCD,"Intialization","RU-7 Please Wait");
  LCDprint(rtu_8_LCD,"Intialization","RU-8 Please Wait");
  LCDprint(rtu_9_LCD,"Intialization","RU-9 Please Wait");
  LCDprint(rtu_10_LCD,"Intialization","RU-10 Please Wait");
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
  currentMillis = millis();
  //Run 1st Command
  if(firstrun==false){
    httpGetRequest(host,APIgetCurrentData);
    waitSerial[0]=true;
    firstrun=true;
     for(int i=1;i<=10;i++){

  //s  Serial.println(pinButton[i]);

  }
  }
  //Wait For 1st Initial Data
  if(waitSerial[0]==true){
    rawStockData = recvWithStartEndMarkers('<','>');
    if(rawStockData!=""){
      getInitialData();
      Serial.println(rawStockData);
      waitSerial[0]=false;
      waitSerialCommand[1]=true;
      clearBuffer();
      for(int i=0;i<=1500;i++){
          receivedChars[i]=NULL;  
      }
      
    }
  }

   if(waitSerial[0]==false){
      //Listen For New Transaction
      listenTransaction();
       //Check Transaction Status
      if(transaction_status==true){
        //Check Push Button For Deactivate Transaction
        checkPushButton();
        //Deactive Transaction Status when PBs are pressed
        deactivateTransaction();
      }
   }
    //Loop Sampling Time 
  delay(1);
}

String recvWithStartEndMarkers(char startMarker, char endMarker) {
    static boolean recvInProgress = false;
    static int ndx = 0;
    char rc;
 
    while (client.available() > 0 && newData == false) {
        rc = client.read();

        if (recvInProgress == true) {
            if (rc != endMarker) {
                receivedChars[ndx] = rc;
                ndx++;
                if (ndx >= numChars) {
                    ndx = numChars - 1;
                }
            }
            else {
                receivedChars[ndx] = '\0'; // terminate the string
                recvInProgress = false;
                ndx = 0;
                newData = true;
            }
        }

        else if (rc == startMarker) {
            recvInProgress = true;
        }
    }

    if (newData == true) {
        Serial.println(receivedChars);
        newData = false;
    }
    return receivedChars;
}

//Clear Serial Buffer
void clearBuffer(){
  //Clear Buffer
  Serial1.flush();
  Serial.flush();
  while(Serial1.available()||Serial.available()){
    Serial.read();
    Serial1.read();
  }
}

//Connect to your wifi network
void connectWifi() {
  // attempt to connect to WiFi network
  while ( status != WL_CONNECTED) {
    Serial.print("Attempting to connect to WPA SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network
    status = WiFi.begin(ssid, pass);
  }
  Serial.println("You're connected to the network");
}

String httpGetRequest (String hostAddress, String query) {
  //Close All Socket
  client.stop();
  // if there's a successful connection
  if (client.connect(server, 3000)) {
    Serial.println("Connecting...");
    // send the HTTP GET request
    client.println(("GET "+ query +" HTTP/1.1"));
    Serial.println(query);
    //client.println(F("GET /get-current-data-by-collector/1 HTTP/1.1"));
    //client.println(F("Host: 83f3-182-253-87-219.ngrok.io"));
    client.println(("Host: " + hostAddress));
    client.println("Connection: close");
    client.println();
  }
  else {
    // if you couldn't make a connection
    Serial.println("Connection failed");
  }
  return rawData;
}

//Parsing CurrentData
void JsonParsing_CurrentData(){
   StaticJsonDocument<600> doc;
   DeserializationError error = deserializeJson(doc,rawStockData);
   if(error){
    Serial.println("Error Parsing Json Current Data");
   }
   //Status Data
   bool status = doc["success"];
   //Function Code
   String message = doc["message"];
   //Data Object (10 RU , SKU Name, Bin ID, 
   JsonObject data = doc["data"];
   JsonArray arrayData = doc["data"].as<JsonArray>();
   //Parsing Array Data For 10 SKU
   for(int i=0;i<=11;i++){
      JsonObject SKU = arrayData[i];
      String l_bin_id = (const char*)SKU["device_id"];
      int l2_bin_id = l_bin_id.substring(3).toInt(); 
      String Sku_name = (const char*)SKU["sku_name"];
      int Sku_qty = (int)SKU["quantity"];
      SKU_name[l2_bin_id]=Sku_name;
      SKU_qty[l2_bin_id]=Sku_qty;
   }
}

//Parsing CurrentData
bool JsonParsing_TransactionData(){
   bool statuses =false;
   StaticJsonDocument<600> parsedData;
   DeserializationError error = deserializeJson(parsedData, rawTransactionData);
   if(error){
    Serial.println("Error Parsing Json Transaction Data");
   }
   else{ //Parsing the Data And Move to Global Var
      //Transaction_Status
      statuses = parsedData["success"];

      if(statuses == true){
            Serial.print("trans Stat : ");Serial.println(statuses);
            //Transaction_id
            transaction_id = (const char*)parsedData["transaction_id"];
            //Data Array ( x Transaction Bin )
            JsonArray array_transaction = parsedData["data"].as<JsonArray>();
            //Parsing Array Transaction Data For x SKU
            
            for(int i=0;i<=11;i++){
                JsonObject json_transactionData = array_transaction[i];
                //Get Local Var
                String l_bin_id = (const char*)json_transactionData["device_id"];
                String l_action_code = (const char*)json_transactionData["action"];
                int l_action_qty = (int)json_transactionData["quantity"];
                //Decode bin ID
                int l2_bin_id = l_bin_id.substring(3).toInt(); 
                // int ll_bin_id = json_transactionData["bin_id"];
                //Move Global Array
                bin_status[l2_bin_id]=true;
                bin_id[l2_bin_id]=l_bin_id;
                action_code[l2_bin_id]=l_action_code;
                action_qty[l2_bin_id]=l_action_qty;   
            } 
      }
      return statuses;
   }
}

//Parsing CurrentData
void JsonParsing_TransactionConfirmation(){
   StaticJsonDocument<300> parsedData;
   DeserializationError error = deserializeJson(parsedData,rawTransactionConfirmation);
   if(error){
    Serial.println("Error Parsing Transaction Confirmation");
   }
   else{ //Parsing the Data And Move to Global Var
      //Transaction_Status
      JsonObject json_transactionData = parsedData["data"];
      transaction_execution = (const char*)json_transactionData["status"]; 
   }
}

void JsonParsing_PickingsConfirmation(int binIDs){
   StaticJsonDocument<400> parsedData;
   DeserializationError error = deserializeJson(parsedData,rawPickingsConfirmation);
   if(error){
    Serial.println("Error Parsing Pickings Confirmation");
    confirmation_status=false;
   }
   else{ //Parsing the Data And Move to Global Var
      //Transaction_Status
      bool transStatus = parsedData["success"];
      confirmation_status= transStatus;
      if(transStatus==true){
        JsonObject json_transactionData = parsedData["data"];
        bin_status[binIDs]=false;
//        SKU_name[binIDs]=(const char*)json_transactionData["sku_name"];
        SKU_qty[binIDs]=(int)json_transactionData["quantity"];
        Serial.println(bin_status[binIDs]);
//        Serial.println(SKU_name[binIDs]);
      } 
      else{
        //Do Nothing
      }
   }
}

//Function Print Single LCD
void LCDprint(LiquidCrystal lcd,String firstRow, String secondRow){
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(firstRow);
  lcd.setCursor(0, 1);
  lcd.print(secondRow);
}

//Confirmatin Status
void LCDTransactionUpdate (LiquidCrystal lcd,String action, String quantity){
  // (note: line 1 is the second row, since counting begins with 0):
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Action : " + action);
  lcd.setCursor(0, 1);
  lcd.print("Quantity : " + quantity);
  return;
}

//LCDIdle
void LCDTransactionStockUpdate (LiquidCrystal lcd,String SKU, String quantity){
  // (note: line 1 is the second row, since counting begins with 0):
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("SKU : " + SKU);
  lcd.setCursor(0, 1);
  lcd.print("Quantity : " + quantity);
  return;
}

//Update All RU LCD Function Custom Text
void LCDAllUpdatesCustom(String Line1, String Line2){
  //Print to 10 RU
  LCDprint(rtu_1_LCD,Line1,Line2);
  LCDprint(rtu_2_LCD,Line1,Line2);
  LCDprint(rtu_3_LCD,Line1,Line2);
  LCDprint(rtu_4_LCD,Line1,Line2);
  LCDprint(rtu_5_LCD,Line1,Line2);
  LCDprint(rtu_6_LCD,Line1,Line2);
  LCDprint(rtu_7_LCD,Line1,Line2);
  LCDprint(rtu_8_LCD,Line1,Line2);
  LCDprint(rtu_9_LCD,Line1,Line2);
  LCDprint(rtu_10_LCD,Line1,Line2);

}

//Function LCD Update ALL SKU & Stock Data
void LCDAllUpdatesStock(){
  //Print Stock SKU to 10 RU
  LCDTransactionStockUpdate(rtu_1_LCD,SKU_name[1],(String)SKU_qty[1]);
  LCDTransactionStockUpdate(rtu_2_LCD,SKU_name[2],(String)SKU_qty[2]);
  LCDTransactionStockUpdate(rtu_3_LCD,SKU_name[3],(String)SKU_qty[3]);
  LCDTransactionStockUpdate(rtu_4_LCD,SKU_name[4],(String)SKU_qty[4]);
  LCDTransactionStockUpdate(rtu_5_LCD,SKU_name[5],(String)SKU_qty[5]);
  LCDTransactionStockUpdate(rtu_6_LCD,SKU_name[6],(String)SKU_qty[6]);
  LCDTransactionStockUpdate(rtu_7_LCD,SKU_name[7],(String)SKU_qty[7]);
  LCDTransactionStockUpdate(rtu_8_LCD,SKU_name[8],(String)SKU_qty[8]);
  LCDTransactionStockUpdate(rtu_9_LCD,SKU_name[9],(String)SKU_qty[9]);
  LCDTransactionStockUpdate(rtu_10_LCD,SKU_name[10],(String)SKU_qty[10]);
}

//Function Pickings
void activateTransaction(){
  //Turn on LED
  for(int i=1;i<=10;i++){
    if(bin_status[i]==true){
      //LCD Update
        if(i==1){LCDTransactionUpdate(rtu_1_LCD,action_code[i],String(action_qty[i]));}
        if(i==2){LCDTransactionUpdate(rtu_2_LCD,action_code[i],String(action_qty[i]));}
        if(i==3){LCDTransactionUpdate(rtu_3_LCD,action_code[i],String(action_qty[i]));}
        if(i==4){LCDTransactionUpdate(rtu_4_LCD,action_code[i],String(action_qty[i]));}
        if(i==5){LCDTransactionUpdate(rtu_5_LCD,action_code[i],String(action_qty[i]));}
        if(i==6){LCDTransactionUpdate(rtu_6_LCD,action_code[i],String(action_qty[i]));}
        if(i==7){LCDTransactionUpdate(rtu_7_LCD,action_code[i],String(action_qty[i]));}
        if(i==8){LCDTransactionUpdate(rtu_8_LCD,action_code[i],String(action_qty[i]));}
        if(i==9){LCDTransactionUpdate(rtu_9_LCD,action_code[i],String(action_qty[i]));}
        if(i==10){LCDTransactionUpdate(rtu_10_LCD,action_code[i],String(action_qty[i]));}
        delay(50);
        digitalWrite(pinLED[i],HIGH);//LED ON
        
    }
    else{
        digitalWrite(pinLED[i],LOW);//LED OFF
    }
  }
}

void deactivateTransaction(){
   for(int i=1;i<=10;i++){
        if(buttonDeactive[i]==true){
         // Serial.println("HIT BUTTON : " + String(i));
          if(waitSerial[3]!=true){
            waitSerialCommand[3]=true;  
          }        
          if(waitSerialCommand[3]==true){
            clearBuffer();
            for(int i=0;i<=1500;i++){
              receivedChars[i]=NULL;  
            }
            //Hit API
           // httpGetRequest(host, "/confirm-done?bin_id="+String(i)+"&transaction_id="+transaction_id);
            httpGetRequest(host, "/confirm-done?transaction_id="+transaction_id+"&device_id="+bin_id[i]);
            
            waitSerial[3]=true;
            waitSerialCommand[3]=false;
          }
           if(waitSerial[3]==true){
            //Get Serial Data
            rawPickingsConfirmation = recvWithStartEndMarkers('<','>');
            if(rawPickingsConfirmation !=""){
            //Parse API  
            JsonParsing_PickingsConfirmation(i);
            //Turn Off LED & Update LCD
            if(bin_status[i]==false){
              //Turn Off LED  
             
              //delay(50);
              switch (i) {
                case 1:
                  LCDTransactionStockUpdate(rtu_1_LCD,SKU_name[i],String(SKU_qty[i]));
                  break;
                case 2:
                  LCDTransactionStockUpdate(rtu_2_LCD,SKU_name[i],String(SKU_qty[i]));
                  break;
                case 3:
                  LCDTransactionStockUpdate(rtu_3_LCD,SKU_name[i],String(SKU_qty[i]));
                  break;
                case 4:
                  LCDTransactionStockUpdate(rtu_4_LCD,SKU_name[i],String(SKU_qty[i]));
                  break;
                case 5:
                  LCDTransactionStockUpdate(rtu_5_LCD,SKU_name[i],String(SKU_qty[i]));
                  break;
                case 6:
                  LCDTransactionStockUpdate(rtu_6_LCD,SKU_name[i],String(SKU_qty[i]));
                  break;
                case 7:
                  LCDTransactionStockUpdate(rtu_7_LCD,SKU_name[i],String(SKU_qty[i]));
                  break;
                case 8:
                  LCDTransactionStockUpdate(rtu_8_LCD,SKU_name[i],String(SKU_qty[i]));
                  break;
                case 9:
                  LCDTransactionStockUpdate(rtu_9_LCD,SKU_name[i],String(SKU_qty[i]));
                  break;
                case 10:
                  LCDTransactionStockUpdate(rtu_10_LCD,SKU_name[i],String(SKU_qty[i]));
                  break;
                default:
                  // statements
                  break;
              }
               delay(50);
               digitalWrite(pinLED[i],LOW);
            }
            buttonDeactive[i]=false;
            waitSerial[3]=false;
            }  
          } 
          i = 11;     
        }  
   }
}

//Read Push Button
bool buttonWasPressed(int pinButton){
  bool statusButton=false;
  if(digitalRead(pinButton)==LOW){
    statusButton=true;
  }else{
    statusButton=false;
  }
  return statusButton;
}

//Complete Transaction Request Function
void checkPushButton(){
  //Serial.println("Button Pressed");
 
    for(int i=1;i<=10;i++){
        //Check Button are Pressed or Not
        if(buttonWasPressed(pinButton[i])&&(bin_status[i]==true)){
          buttonDeactive[i]=true; 
          Serial.println("Button Was Pressed : " + String(i));
          Serial.println(buttonDeactive[i]);
         // digitalWrite(pinLED[i],!digitalRead(pinButton[i]));
        } 
    }
  
}

//listen Transaction Reuqest Function
void listenTransaction(){
  //Check Active transaction
  int transactionTotal=0;
  for(int i=1;i<=10;i++){
    if(bin_status[i]==true){
      transactionTotal++;  
    }
    if(i==10&&transactionTotal==0&&transaction_updated==true){
       clearBuffer();
      for(int i=0;i<=1500;i++){
        receivedChars[i]=NULL;  
      }
      rawData="";
      rawStockData="";
      rawTransactionData="";
      rawTransactionConfirmation="";
      rawPickingsConfirmation="";
      transaction_execution="";
      transaction_status=false;
      transaction_id="";
      transaction_updated=false;
      waitSerialCommand[1]=true;
      waitSerialCommand[2]=false;
      waitSerialCommand[3]=false;
      waitSerial[1]=false;
      waitSerial[2]=false;
      waitSerial[3]=false;
    }
  }
  
  //If Already Transaction, Confirm the transaction
  if(transaction_status == false){
      //Request Transaction Data
      if(waitSerialCommand[1]==true && (currentMillis - startMillis >= period)){
        Serial.println(APIgetTransaction);
        httpGetRequest(host,APIgetTransaction);
        waitSerialCommand[1]=false; 
        waitSerial[1]=true;
        startMillis = currentMillis;
      }
      if(waitSerial[1]==true){
          rawTransactionData = recvWithStartEndMarkers('<','>');
         
          if(rawTransactionData!=""){
             Serial.println(rawTransactionData);
            //Parsing raw Transaction Json Data
            transaction_status = JsonParsing_TransactionData();
            if(transaction_status==false){
                  waitSerialCommand[1]=true;
                  waitSerial[1]=false;
            }else{
              waitSerial[1]=false;
              waitSerialCommand[2]=true;
              Serial.print("stat :");Serial.println(transaction_status);
              Serial.print("id :");Serial.println(transaction_id);
            }
            
          } 
          else{
          
          }
          //Clear
             clearBuffer();
             for(int i=0;i<=1500;i++){
                receivedChars[i]=NULL;  
             }
      }
      
  }else{ //Req Pull Data From Raspi if no transaction 
      if(transaction_execution!="execute"){
         //Sent Confirmation Transaction to Raspi
          if(waitSerialCommand[2]==true){
             Serial.println(APItransactionConfirm);
             httpGetRequest(host,APItransactionConfirm);
             waitSerialCommand[2]=false; 
             waitSerial[2]=true; 
             
          }
          if(waitSerial[2]==true){
             rawTransactionConfirmation = recvWithStartEndMarkers('<','>');
             if(rawTransactionConfirmation!=""){
                //Parsing raw Transaction Confirm Json Data
                JsonParsing_TransactionConfirmation();
                Serial.println(rawTransactionConfirmation);
                waitSerial[2]=false;
                Serial.println(transaction_id);
                Serial.println(transaction_updated);
                Serial.println(transaction_execution);
                Serial.print(bin_status[1]);Serial.print(bin_status[2]);Serial.print(bin_status[3]);Serial.print(bin_status[4]);Serial.print(bin_status[5]);
                Serial.print(bin_status[6]);Serial.print(bin_status[7]);Serial.print(bin_status[8]);Serial.print(bin_status[9]);Serial.print(bin_status[10]);
                //waitSerialCommand[3]=true;
          } }
      }else{
        if(transaction_updated==false){
          Serial.println("sampai");
          //Update LCD & Turn on LED
          activateTransaction();
          transaction_updated=true;
        }else{
          //Do Nothing Wait Until All Bin Transaction Completed    
        }
      }
  } 
}
//Get Initial SKU & Stock Data Function
void getInitialData(){
  //Get Data
 // rawStockData = httpGetRequest(host,APIgetCurrentData);
    //Parsing Stock Data
   JsonParsing_CurrentData();
   //Add If Here  
   //Print to RU LCD
   LCDAllUpdatesStock(); 
}
