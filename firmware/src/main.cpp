#include <Arduino.h>
#include <WiFi.h>
#include "Kalman.h"
#include "MPU9250.h"
#include "MadgwickAHRS.h"

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
MPU9250 IMU(Wire, 0x68);
Madgwick filter;
long long lastMS = 0; // Last measurement timestamp

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
    //IMU.setMagCalX(23, 1);
    //IMU.setMagCalY(22, 1);
    //IMU.setMagCalZ(-0.5, 1);

    IMU.calibrateGyro();
    //IMU.setSrd(19);

    IMU.setAccelRange(MPU9250::ACCEL_RANGE_4G);
    IMU.setGyroRange(MPU9250::GYRO_RANGE_500DPS);
    IMU.setDlpfBandwidth(MPU9250::DLPF_BANDWIDTH_184HZ);
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
    Serial.println(rec.id);
}


void connectHost(const char* ip, int port) {
    client.connect(ip, port);
}


int isConnectedToHost() {
    return client.connected();
}


int initIMU() {
    if (IMU.begin() < 0) {
        Serial.println("IMU initialization unsuccessful");
        Serial.println("Check IMU wiring or try cycling power");
        Serial.print("Status: ");
        return -1;
    }
    return 0;
}


int isIMUReady() {
    return IMU.readSensor();
}

long i = 0;
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

    long long curMS = esp_timer_get_time();
    filter.invSampleFreq = (curMS - lastMS) / 20000.0f;
    lastMS = curMS;
    if (i%500 == 0)
        filter.update(rec.gx, rec.gy, rec.gz, rec.ax, rec.ay, rec.az, rec.mx, rec.my, rec.mz);
    else
        filter.updateIMU(rec.gx, rec.gy, rec.gz, rec.ax, rec.ay, rec.az);

    rec.q0 = filter.q0;
    rec.q1 = filter.q1;
    rec.q2 = filter.q2;
    rec.q3 = filter.q3;
    i++;
}


int sendRecord() {
    return client.write((char*)&rec, sizeof(Record));
}

// float getRndFloat() {
//     return 1.0f * rand() / 32768.0f;
// }


// int isIMUReady() {
//     return 1;
// }

// void packRecord() {
//     rec.ax = getRndFloat();
//     rec.ay = getRndFloat();
//     rec.az = getRndFloat();
//     rec.gx = getRndFloat();
//     rec.gy = getRndFloat();
//     rec.gz = getRndFloat();
//     rec.mx = getRndFloat();
//     rec.my = getRndFloat();
//     rec.mz = getRndFloat();
//     rec.q0 = getRndFloat();
//     rec.q1 = getRndFloat();
//     rec.q2 = getRndFloat();
//     rec.q3 = getRndFloat();
// }