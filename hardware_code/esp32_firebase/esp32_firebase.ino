#include <WiFi.h>
#include <FirebaseESP32.h>

//1. Change the following info
#define FIREBASE_HOST "icamb-85756.firebaseio.com"
#define FIREBASE_AUTH "TVyMMXaBk12CuH9TQDfO9lqMsACHYZRvQIw3rgeH"
#define WIFI_SSID "library"
#define WIFI_PASSWORD "l1234567890"


//2. Define FirebaseESP8266 data object for data sending and receiving
FirebaseData firebaseData;


void setup()
{

  Serial.begin(115200);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();


  //3. Set your Firebase info

  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);

  //4. Enable auto reconnect the WiFi when connection lost
  Firebase.reconnectWiFi(true);

  //5. Try to set int data to Firebase
  //The set function returns bool for the status of operation
  //firebaseData requires for sending the data
  if(Firebase.setString(firebaseData, "/hum_sensor/0/heat/date", "20/10/2563"))
  {
    //Success
     Serial.println("Set int data success");

  }else{
    //Failed?, get the error reason from firebaseData

    Serial.print("Error in setInt, ");
    Serial.println(firebaseData.errorReason());
  }
}


void loop()
{
}
