#include "lib/TCPSocket.cpp"
#include <signal.h>
#include <thread>
#include <atomic>
#include <chrono>
#include <fstream>
using namespace std;


const int DEVICES = 6;
const int MINUTES = 15;

const int PORT = 1234;
const int DATAGATE_PORT = 1235;

//Global acceptor socket variable (for signal)
Socket acceptorSocket;

// Measurement structure
struct Record {
    int id;
    float ax, ay, az, gx, gy, gz, mx, my, mz;
    float q0, q1, q2, q3;
};

// Measurements array
Record storage[MINUTES*60*1000][DEVICES];

// Device ID -> sensor index
int idToIndexMapping[256] = {-1};

// Measuring state
atomic<bool> isMeasuring (false);

// Measurement started timestamp 
long startTime = 0;

// Measurement ended timestamp 
long endTime = 0;

// Get current time in milliseconds since epoch
long ms() {
    return std::chrono::system_clock::now().time_since_epoch().count() / 1000000;
}

// Get current time in milliseconds since given time
int ms(long start) {
    return ms() - start;
}

// Save measurements to disk as .csv
void saveToCSV(long start, long stop) {
    string path("recordings/");
    path += to_string(start);
    path += ".csv";
    cout << path << endl;
    fstream file(path.c_str(), ios::out);
    for (int i = 0; i < (stop-start); i++) {
        file << i;
        for (int k = 0; k < DEVICES; k++) {
            if (storage[i][k].id) 
                file << "," << storage[i][k].ax 
                    << "," << storage[i][k].ay
                    << "," << storage[i][k].az
                    << "," << storage[i][k].gx
                    << "," << storage[i][k].gy
                    << "," << storage[i][k].gz
                    << "," << storage[i][k].mx
                    << "," << storage[i][k].my
                    << "," << storage[i][k].mz
                    << "," << storage[i][k].q0
                    << "," << storage[i][k].q1
                    << "," << storage[i][k].q2
                    << "," << storage[i][k].q3;
            else
                file << ",,,,,,,,,,,,,";
        }
        file << '\n';
    }
    file.flush();
    file.close();
}

// ID -> INDEX
int idToIndex(int deviceId) {
    if (idToIndexMapping[deviceId] == -1) 
        idToIndexMapping[deviceId] = max(idToIndexMapping[0], idToIndexMapping[255]) + 1;
    return idToIndexMapping[deviceId];
}


void clientThread(Socket clientSocket) {
    Record* recordBuffer = new Record; // Init memory for Record structure
    memset(recordBuffer, 0, sizeof(Record)); // Clear buffer memory
    int deviceId = -1; // Initial invalid device id
    do {
        cout << receiveDataTCP(clientSocket, (char*)(recordBuffer), sizeof(Record)) << endl;
        cout << recordBuffer->ax << " " << recordBuffer->ay << " " << recordBuffer->az << endl;
        deviceId = recordBuffer->id;
    } while ((deviceId < 0) || (deviceId > 255)); // Wait until "good" device id arrive
    int deviceIndex = idToIndex(deviceId); // Array index assignment
    while (1) {
        while (!isMeasuring); // Wait command
        int receivedBytes = receiveDataTCP(clientSocket, (char*)(recordBuffer), sizeof(Record));
        if (receivedBytes <= 0) break; // Interrupt if error or disconnect
        if (receivedBytes != sizeof(Record)) continue; // If transmition error
        if (recordBuffer->id != deviceId) continue; // Use device id as consistency indicator
        int millis = ms(startTime); // Package received time
        memcpy((void*)&(storage[millis][deviceIndex]), (void*)(recordBuffer), sizeof(Record)); // Save measurement to storage
    }
    cout << "[INFO] Device " << deviceIndex << " disconnected..." << endl;
    close(clientSocket);
    delete recordBuffer;
}


void acceptorThread(Socket acceptorSocket) {
    if (acceptorSocket < 0) {
        cout << "[ERROR] Server not started." << endl;
        return;
    } else {
        cout << "[INFO] Server started." << endl;
    }
    struct timeval tv; // Forced timeout disconnect
    tv.tv_sec = 2;
    tv.tv_usec = 0;
    while (1) {
        Socket clientSocket = acceptSocket(acceptorSocket);
        cout << "[INFO] Device connected." << endl;
        setsockopt(clientSocket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof(tv));
        thread newThread(clientThread, clientSocket); // Run new thread for each device
        newThread.detach();
    }
}

// Sends last non-zero measurement to client
void dataGateThread(Socket clientSocket) {
    char* buf = new char[129];
    while (1) {
        int receivedBytes = receiveDataTCP(clientSocket, buf, 128); // Wait request
        if (receivedBytes <= 0) break; // Interrupt if error or disconnect
        int millis = (startTime) ? ms(startTime) : 0;
        for (int i = millis; i >= 0; i--) {
            bool found = false;
            for (int j = 0; j < DEVICES; j++) {
                if (i == 0 || storage[i][j].id != 0) {
                    found = true;
                    break;
                }
            }
            if (found) {
                sendDataTCP(clientSocket, (char*)&(storage[i]), sizeof(storage[i]));
                break;
            }
        }
    }
    cout << "[INFO] Data gate client disconnected." << endl;
    close(clientSocket);
    delete buf;
}

void dataGateAcceptorThread() {
    Socket dataGateSocket = createSocketTCP();
    SocketProps* dataGateProps = createSocketProps(DATAGATE_PORT);
    setsockopt(dataGateSocket, SOL_SOCKET, SO_REUSEADDR, 0, sizeof(int)); // Allow reusing port
    bindSocket(dataGateSocket, dataGateProps);
    listenSocket(dataGateSocket);
    while (1) {
        Socket clientSocket = acceptSocket(dataGateSocket);
        cout << "[INFO] Data gate client connected." << endl;
        thread newThread(dataGateThread, clientSocket);
        newThread.detach();
    }
}

void terminate (int param)
{
    std::cout << "Stopping..." << std::endl;
    close(acceptorSocket);
    exit(0);
}

int main() {
    signal(SIGTERM, terminate);
    acceptorSocket = createSocketTCP();
    SocketProps* acceptorProps = createSocketProps(PORT);
    setsockopt(acceptorSocket, SOL_SOCKET, SO_REUSEADDR, 0, sizeof(int)); // Allow reusing port
    bindSocket(acceptorSocket, acceptorProps);
    listenSocket(acceptorSocket);
    thread acceptor(acceptorThread, acceptorSocket); // Run async device acceptor
    thread dataGate(dataGateAcceptorThread); // Run async device acceptor
    while (1) {
        char ch;
        cin >> ch;
        switch (ch) {
            case '+': {
                if (!isMeasuring) {
                    startTime = ms();
                    isMeasuring = true;
                    cout << "[INFO] Recording started." << endl;
                } else {
                    cout << "[WARNING] Recording already started." << endl;
                }
                break;
            }
            case '-': {
                if (isMeasuring) {
                    isMeasuring = false;
                    endTime = ms();
                    cout << "[INFO] Recording stopped." << endl;
                } else {
                    cout << "[WARNING] Recording already stopped." << endl;
                }
                break;
            }
            case 'r': {
                if (!isMeasuring) {
                    memset(&storage, 0, sizeof(storage));
                    cout << "[INFO] Memory cleared." << endl;
                } else {
                    cout << "[WARNING] Memory not cleared. Recording now." << endl;
                }
                break;
            }
            case 's': {
                if (!isMeasuring) {
                    saveToCSV(startTime, endTime);
                    cout << "[INFO] Recording saved." << endl;
                } else {
                    cout << "[WARNING] Recording not saved. Recording now..." << endl;
                }
                break;
            }
            case 'q': {
                cout << "[INFO] Stopping server..." << endl;
                close(acceptorSocket);
                exit(0);
            }
            case 'h': {
                cout << "[INFO] + => start recording" << endl;
                cout << "[INFO] - => stop recording" << endl;
                cout << "[INFO] s => save recording" << endl;
                cout << "[INFO] r => reset memory" << endl;
                cout << "[INFO] q => exit" << endl;
                break;
            }
            default: {
                cout << "[WARNING] Unknown command... Press h for commands..." << endl;
                break;
            }
        }
    }
    acceptor.join();
    return 0;
}