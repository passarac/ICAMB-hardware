// TEMPERATURE AND HUMID SENSOR
#include "DHT.h"
#define DHTPIN 15     // Digital pin connected to the DHT sensor
#define DHTTYPE DHT11   // DHT 11
DHT dht(DHTPIN, DHTTYPE);

//IMU SENSOR
#include "I2Cdev.h"
#include "MPU6050.h"
#if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
    #include "Wire.h"
#endif
MPU6050 accelgyro;
int16_t ax, ay, az;
int16_t gx, gy, gz;
#define OUTPUT_READABLE_ACCELGYRO

//load
#include "HX711.h"
#define LOADCELL_DOUT_PIN  4
#define LOADCELL_SCK_PIN  18
HX711 scale;
float calibration_factor = -7050; //-7050 worked for my 440lb max scale setup

//firebase
#include <WiFi.h>
#include <FirebaseESP32.h>
#include <string>
//Change the following info
#define FIREBASE_HOST "icamb-85756.firebaseio.com"
#define FIREBASE_AUTH "TVyMMXaBk12CuH9TQDfO9lqMsACHYZRvQIw3rgeH"
#define WIFI_SSID "hotspot"
#define WIFI_PASSWORD "thisisthepassword"
//Initialize current_id variable
int current_id;
//Initialize firebase database path
String current_path;
String db_path;
// Define FirebaseESP8266 data object for data sending and receiving
FirebaseData firebaseData;

void setup() {
  Serial.begin(115200);

  //DHT SENSOR
  Serial.println(F("DHTxx test!"));
  dht.begin();

  //IMU SENSOR
  #if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
        Wire.begin();
    #elif I2CDEV_IMPLEMENTATION == I2CDEV_BUILTIN_FASTWIRE
        Fastwire::setup(400, true);
    #endif
    // initialize device
    Serial.println("Initializing I2C devices...");
    accelgyro.initialize();
    // verify connection
    Serial.println("Testing device connections...");
    Serial.println(accelgyro.testConnection() ? "MPU6050 connection successful" : "MPU6050 connection failed");

    //load
    scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
    scale.set_scale();
    scale.tare(); //Reset the scale to 0

    //firebase
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

void loop() {
  // Wait a few seconds between measurements.
  delay(1000);
  float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();
  // Read temperature as Fahrenheit (isFahrenheit = true)
  float f = dht.readTemperature(true);
  // Compute heat index in Fahrenheit (the default)
  float hif = dht.computeHeatIndex(f, h);
  // Compute heat index in Celsius (isFahreheit = false)
  float hic = dht.computeHeatIndex(t, h, false);
  Serial.print(F("Humidity: "));
  Serial.print(h);
  Serial.print(F("%  Temperature: "));
  Serial.print(t);
  Serial.print(F("°C "));
  Serial.println("");

  accelgyro.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);

  Serial.print("ax: ");
  Serial.print(ax);
  Serial.print("\t");
  Serial.print("ay: ");
  Serial.print(ay);
  Serial.print("\t");
  Serial.print("az: ");
  Serial.print(az);
  Serial.print("\t");
  Serial.print("gx: ");
  Serial.print(gx);
  Serial.print("\t");
  Serial.print("gy: ");
  Serial.print(gy);
  Serial.print("\t");
  Serial.print("gz: ");
  Serial.println(gz);
  Serial.println("");

  scale.set_scale(calibration_factor);
  Serial.print("Reading: ");
  int weight = scale.get_units();
  weight = abs(weight);
  Serial.print(weight);
  Serial.print(" lbs");

   if(Firebase.getInt(firebaseData, "/sensor_info/current_id"))
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
  //Serial.println("Get current_id from Firebase success");
  //Serial.print("Current_id= ");
  //Serial.print(current_id);
  //Serial.println(" ");

  //Set db_path
  current_path = String(current_id);
  db_path = "/sensor_info/" + current_path;

  int accel_x = ax;
  int accel_y = ay;
  int accel_z = az;

  int gyro_x = gx;
  int gyro_y = gy;
  int gyro_z = gz;
  
  float humid = h;
  float temp = t;

  float load_cell = weight;
  
  Firebase.setString(firebaseData, db_path+"/pat_id", "patient001");
  
  //Sending sensor values
  Firebase.setInt(firebaseData, db_path+"/accel_x", accel_x);
  Firebase.setInt(firebaseData, db_path+"/accel_y", accel_y);
  Firebase.setInt(firebaseData, db_path+"/accel_z", accel_z);
  
  Firebase.setInt(firebaseData, db_path+"/gyro_x", gyro_x);
  Firebase.setInt(firebaseData, db_path+"/gyro_y", gyro_y);
  Firebase.setInt(firebaseData, db_path+"/gyro_z", gyro_z);
  
  Firebase.setInt(firebaseData, db_path+"/humid", humid);
  
  Firebase.setInt(firebaseData, db_path+"/load_cell", load_cell);
  
  Firebase.setInt(firebaseData, db_path+"/temp", temp);

  //incrementing the current_id by 1
  current_id = current_id + 1;

  //replacing current_id with new value into firebase
  if(Firebase.setInt(firebaseData, "/sensor_info/current_id", current_id))
  {
    //Success
    Serial.println("Set int data success");
  }
  else{
    //Failed?, get the error reason from firebaseData
    Serial.print("Error in setInt, ");
    Serial.println(firebaseData.errorReason());
  }

 
}
