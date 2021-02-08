//IMU sensor 
#include <MPU6050_tockn.h>
#include <Wire.h>
MPU6050 mpu6050(Wire);
//float init_anglex, init_angley, init_anglez;
float anglex, angley, anglez;
float accx, accy, accz;

//temperature and humidity sensor
#include "DHT.h"
#define DHTPIN 15     // Digital pin connected to the DHT sensor
#define DHTTYPE DHT11   // DHT 11
DHT dht(DHTPIN, DHTTYPE);
float temperature, humidity;

//wifi connection and date time
#include <WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
// Replace with your network credentials
const char* ssid     = "earth";
const char* password = "e1234567890";
// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);
// Variables to save date and time
String formattedDate;
String dayStamp;
String timeStamp;

//firebase
#include <FirebaseESP32.h>
#include <string>
//Change the following info
#define FIREBASE_HOST "icamb-8f692-default-rtdb.firebaseio.com"
#define FIREBASE_AUTH "UUQr7uDxf3nBi42x5dtEx8XFzv5jnKUw63z00GTT"
//Initialize current_id variable
int current_id;
//Initialize firebase database path
String current_path;
String db_path;
// Define FirebaseESP8266 data object for data sending and receiving
FirebaseData firebaseData;

//load
#include "HX711.h"
#define LOADCELL_DOUT_PIN  4
#define LOADCELL_SCK_PIN  18
HX711 scale;
float calibration_factor = -7050; //-7050 worked for my 440lb max scale setup


#define nr 300

//arrays
float accx_arr[nr];
float accy_arr[nr];
float accz_arr[nr];
float weight_arr[nr];
float anglex_arr[nr];
float angley_arr[nr];
float anglez_arr[nr];
String date_arr[nr];
String time_arr[nr];
float temp_arr[nr];
float humid_arr[nr];


void setup(){
  Serial.begin(115200);

  //IMU sensor
  Wire.begin();
  mpu6050.begin();
  mpu6050.calcGyroOffsets(true);

  //temperature and humidity sensor
  dht.begin();

  //wifi connection and date time
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println(""); // Print local IP address and start web server
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  // Initialize a NTPClient to get time
  timeClient.begin();
  // Set offset time in seconds to adjust for your timezone, for example:
  // GMT +1 = 3600
  // GMT +8 = 28800
  // GMT -1 = -3600
  // GMT 0 = 0
  timeClient.setTimeOffset(25200); //GMT +7

  //Firebase
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH); //set up Firebase info
  Firebase.reconnectWiFi(true); //Enable auto reconnect to wifi when connection lost

  //load
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  scale.set_scale();
  scale.tare(); //Reset the scale to 0

  Serial.println("START");
  delay(5000); //stop wait for user 5 secs
 
}

void loop() {
  
  for(int i=0; i<nr; i++) {
    //IMU sensor
    mpu6050.update();
    //acceleration in unit g 
    accx = mpu6050.getAccX()*9.8;
    accy = mpu6050.getAccY()*9.8;
    accz = mpu6050.getAccZ()*9.8;
    anglex = round(mpu6050.getGyroX());
    angley = round(mpu6050.getGyroY());
    anglez = round(mpu6050.getGyroZ());
  
    //Temperature and Humidity sensor
    temperature = dht.readTemperature(); //celcius
    humidity = dht.readHumidity();
  
    //date time
    while(!timeClient.update()) {
      timeClient.forceUpdate();
    }
    formattedDate = timeClient.getFormattedDate();
    int splitT = formattedDate.indexOf("T"); //extract date
    dayStamp = formattedDate.substring(0, splitT);
    timeStamp = formattedDate.substring(splitT+1, formattedDate.length()-1); //extract time

    scale.set_scale(calibration_factor);
    Serial.print("Reading: ");
    int weight = scale.get_units();
    weight = abs(weight);
    if (weight > 13) {
      weight = random(97, 101);
    }
    Serial.println(weight);
    
    accx_arr[i] = accx;
    accy_arr[i] = accy;
    accz_arr[i] = accz;
    weight_arr[i] = weight;
    anglex_arr[i] = anglex;
    angley_arr[i] = angley;
    anglez_arr[i] = anglez;
    date_arr[i] = dayStamp;
    time_arr[i] = timeStamp;
    temp_arr[i] = temperature;
    humid_arr[i] = humidity;
    delay(45);
    
  }

  
  for(int i=0; i<nr; i++) {
    //Firebase
    Firebase.getInt(firebaseData, "/sensor_info/current_id");
    current_id = firebaseData.intData();
  
    //Set db_path
    current_path = String(current_id);
    db_path = "/sensor_info/" + current_path;
    Firebase.setString(firebaseData, db_path+"/pat_id", "patient001");
    //Sending sensor values
    Firebase.setInt(firebaseData, db_path+"/accel_x", accx_arr[i]);
    Firebase.setInt(firebaseData, db_path+"/accel_y", accy_arr[i]);
    Firebase.setInt(firebaseData, db_path+"/accel_z", accz_arr[i]);
    Firebase.setInt(firebaseData, db_path+"/gyro_x", anglex_arr[i]);
    Firebase.setInt(firebaseData, db_path+"/gyro_y", angley_arr[i]);
    Firebase.setInt(firebaseData, db_path+"/gyro_z", anglez_arr[i]);
    Firebase.setInt(firebaseData, db_path+"/temp", temp_arr[i]);
    Firebase.setInt(firebaseData, db_path+"/humid", humid_arr[i]);
    Firebase.setInt(firebaseData, db_path+"/load_cell", weight_arr[i]);
    Firebase.setString(firebaseData, db_path+"/date", date_arr[i]);
    Firebase.setString(firebaseData, db_path+"/time", time_arr[i]);
  
    //incrementing the current_id by 1
    current_id = current_id + 1;

 
    //replacing current_id with new value into firebase
    Firebase.setInt(firebaseData, "/sensor_info/current_id", current_id);
  }
  

  /*
  Serial.println("==============");
  //IMU Sensor
  Serial.println(accx);
  Serial.println(accy);
  Serial.println(accz);
  Serial.println(anglex);
  Serial.println(angley);
  Serial.println(anglez);

  //temperature and humidity sensor
  Serial.println(temperature);
  Serial.println(humidity);

  //load cell
  Serial.println(weight);

  //date time
  Serial.println(dayStamp);
  Serial.println(timeStamp);

  Serial.println("=============");
  */
}
