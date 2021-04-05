#include <Arduino.h>
#include <WiFi.h>
#include "Kalman.h"
#include "MPU9250.h"

const char* IP = "192.168.1.42";
const int PORT = 1234;

const char* SSID1 = "K-Lab";
const char* PASS1 = "allhailklab";

const char* SSID2 = "ImperiumDev";
const char* PASS2 = "imperium01";

// Measurement structure
struct Record {
    int id;
    float ax, ay, az, gx, gy, gz, mx, my, mz;
    float q0, q1, q2, q3;
};

Record rec;
WiFiClient client;
MPU9250 IMU;
long lastMS = 0; // Last measurement timestamp

void startWiFi(const char* ssid, const char* pass);
void connectHost(const char* ip, int port);
int isConnectedToHost();
int initIMU();
int isIMUReady();
void packRecord();
int sendRecord();


void setup() {
    Serial.begin(115200);
    Wire.begin();
    startWiFi(SSID1, PASS1);
    if (initIMU() < 0) esp_restart();
    IMU.setMagBias(23, 22, -0.5);
    IMU.setMagScale(1, 1, 1);
}

void loop() {
    connectHost(IP, PORT);
    while (isConnectedToHost()) {
        if (!isIMUReady()) continue;
        packRecord();
        sendRecord();
    }
}


void startWiFi(const char* ssid, const char* pass) {
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, pass);
    while (WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        delay(250);
    }
    Serial.println("");
    String ipString = WiFi.localIP().toString();
    rec.id = atoi(ipString.substring(ipString.lastIndexOf('.') + 1).c_str());
    Serial.println(ipString);
    Serial.println(rec.id);
}


void connectHost(const char* ip, int port) {
    client.connect(ip, port);
}


int isConnectedToHost() {
    return client.connected();
}


int initIMU() {
    int status = IMU.setup(0x69);
    Serial.println(status);
    if (status < 0) {
        Serial.println("IMU initialization unsuccessful");
        Serial.println("Check IMU wiring or try cycling power");
        Serial.print("Status: ");
        Serial.println(status);
    }
    return status;
}


int isIMUReady() {
    return IMU.update();
}


void packRecord() {
    rec.ax = Kalman::filter(IMU.getAccX(), 0);
    rec.ay = Kalman::filter(IMU.getAccY(), 1);
    rec.az = Kalman::filter(IMU.getAccZ(), 2);
    rec.gx = Kalman::filter(IMU.getGyroX(), 3);
    rec.gy = Kalman::filter(IMU.getGyroY(), 4);
    rec.gz = Kalman::filter(IMU.getGyroZ(), 5);
    rec.mx = IMU.getMagX();
    rec.my = IMU.getMagY();
    rec.mz = IMU.getMagZ();
    rec.q0 = IMU.getQuaternionW();
    rec.q1 = IMU.getQuaternionX();
    rec.q2 = IMU.getQuaternionY();
    rec.q3 = IMU.getQuaternionZ();
}


int sendRecord() {
    return client.write((char*)&rec, sizeof(Record));
}