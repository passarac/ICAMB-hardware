#include <WiFi.h>
#include <FirebaseESP32.h>
#include <string>

//Change the following info
#define FIREBASE_HOST "icamb-85756.firebaseio.com"
#define FIREBASE_AUTH "TVyMMXaBk12CuH9TQDfO9lqMsACHYZRvQIw3rgeH"
#define WIFI_SSID "library"
#define WIFI_PASSWORD "l1234567890"

//Initialize current_id variable
int current_id;

//Initialize firebase database path
String current_path;
String db_path;

// Define FirebaseESP8266 data object for data sending and receiving
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


  //Set your Firebase info
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);

  //Enable auto reconnect the WiFi when connection lost
  Firebase.reconnectWiFi(true);

}

void loop()
{
  //Obtaing the current_id
  if(Firebase.getInt(firebaseData, "/current_id"))
  {
    //Success
    current_id = firebaseData.intData();
  }
  else
  {
    //Failed?, get the error reason from firebaseData
    Serial.print("Error in getInt, ");
    Serial.println(firebaseData.errorReason());
  }

  //Printing the current_id
  Serial.println("Get current_id from Firebase success");
  Serial.print("Current_id= ");
  Serial.print(current_id);
  Serial.println(" ");

  //Set db_path
  current_path = String(current_id);
  db_path = "/sensor/" + current_path;

  int accel_x = random(-8000, 8000);
  int accel_y = random(-8000, 8000);
  int accel_z = random(-8000, 8000);

  int gyro_x = random(-400, 400);
  int gyro_y = random(-400, 400);
  int gyro_z = random(-400, 400);

  int heat = random(25, 30);
  int humid = random(45, 50);
  int temp = random(26, 29);

  int load_cell = random(10, 40);

  //Sending sensor values with rand()
  Firebase.setInt(firebaseData, db_path+"/accel_x", accel_x);
  Firebase.setInt(firebaseData, db_path+"/accel_y", accel_y);
  Firebase.setInt(firebaseData, db_path+"/accel_z", accel_z);
  
  Firebase.setInt(firebaseData, db_path+"/gyro_x", gyro_x);
  Firebase.setInt(firebaseData, db_path+"/gyro_y", gyro_y);
  Firebase.setInt(firebaseData, db_path+"/gyro_z", gyro_z);
  
  Firebase.setInt(firebaseData, db_path+"/heat", heat);
  Firebase.setInt(firebaseData, db_path+"/humid", humid);
  
  Firebase.setInt(firebaseData, db_path+"/load_cell", load_cell);
  
  Firebase.setInt(firebaseData, db_path+"/temp", temp);

  //incrementing the current_id by 1
  current_id = current_id + 1;

  //replacing current_id with new value into firebase
  if(Firebase.setInt(firebaseData, "/current_id", current_id))
  {
    //Success
    Serial.println("Set int data success");
  }
  else{
    //Failed?, get the error reason from firebaseData
    Serial.print("Error in setInt, ");
    Serial.println(firebaseData.errorReason());
  }
  
  delay(10000);
  
}
