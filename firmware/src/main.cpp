#include <Arduino.h>
#include <WiFi.h>
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
        IMU.setDlpfBandwidth(MPU9250::DLPF_BANDWIDTH_184HZ);
    }
    return status;
}

int isIMUReady() {
    return IMU.readSensor();
}

void packRecord() {
    rec.ax = IMU.getAccelX_mss();
    rec.ay = IMU.getAccelY_mss();
    rec.az = IMU.getAccelZ_mss();
    rec.gx = IMU.getGyroX_rads();
    rec.gy = IMU.getGyroY_rads();
    rec.gz = IMU.getGyroZ_rads();
    rec.mx = IMU.getMagX_uT();
    rec.my = IMU.getMagY_uT();
    rec.mz = IMU.getMagZ_uT();
}

int sendRecord() {
    return client.write((char*)&rec, sizeof(Record));
}