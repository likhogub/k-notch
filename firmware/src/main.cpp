#include <Arduino.h>
#include <WiFi.h>
#include "Kalman.h"
#include "MPU9250.h"

int id = 0; //Device id

const char* host = "192.168.1.224";
const int port = 1234;

const char* ssid1 = "K-Lab";
const char* password1 = "allhailklab";

const char* ssid2 = "ImperiumDev";
const char* password2 = "imperium01";

//Measurement structure
struct Record {
    int id;
    float ax, ay, az, gx, gy, gz, mx, my, mz;
};

Record rec;
WiFiClient client;
MPU9250 IMU(Wire, 0x68);

void startWiFi();
void connectHost();
int isConnectedToHost();
void initDeviceId();
int initIMU();
int isIMUReady();
void packRecord();
int sendRecord();

void setup() {
    Serial.begin(115200);
    startWiFi();
    initDeviceId();
    if (initIMU() < 0) esp_restart();
    IMU.calibrateGyro();
    IMU.calibrateAccel();
}

void loop() {
    connectHost();
    while (isConnectedToHost()) {
        if (!isIMUReady()) continue;
        packRecord();
        sendRecord();    
    }
}

void startWiFi() {
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid2, password2);
    while (WiFi.status() != WL_CONNECTED) {
        delay(250);
        Serial.print(".");
    }
    Serial.println("");
    Serial.print("IP ");
    Serial.println(WiFi.localIP());
}

void connectHost() {
    client.connect(host, port);
}

int isConnectedToHost() {
    return client.connected();
}

void initDeviceId() {
    int lastDot = WiFi.localIP().toString().lastIndexOf('.');
    id = atoi(WiFi.localIP().toString().substring(lastDot+1).c_str());
    rec.id = id;
}

int initIMU() {
    int status = IMU.begin(); //Start IMU device
    Serial.println(status);
    if (status < 0) {
        Serial.println("IMU initialization unsuccessful");
        Serial.println("Check IMU wiring or try cycling power");
        Serial.print("Status: ");
        Serial.println(status);
    } else {
        IMU.setAccelRange(MPU9250::ACCEL_RANGE_4G);
        IMU.setGyroRange(MPU9250::GYRO_RANGE_500DPS);
    }
    return status;
}

int isIMUReady() {
    return IMU.readSensor();
}

void packRecord() {
    rec.ax = Kalman::filter(IMU.getAccelX_mss(), 0);
    rec.ay = Kalman::filter(IMU.getAccelY_mss(), 1);
    rec.az = Kalman::filter(IMU.getAccelZ_mss(), 2);
    rec.gx = Kalman::filter(IMU.getGyroX_rads(), 3);
    rec.gy = Kalman::filter(IMU.getGyroY_rads(), 4);
    rec.gz = Kalman::filter(IMU.getGyroZ_rads(), 5);
    rec.mx = Kalman::filter(IMU.getMagX_uT(), 6);
    rec.my = Kalman::filter(IMU.getMagY_uT(), 7);
    rec.mz = Kalman::filter(IMU.getMagZ_uT(), 8);
}

int sendRecord() {
    return client.write((char*)&rec, sizeof(Record));
}