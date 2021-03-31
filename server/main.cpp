#include "lib/TCPSocket.cpp"
#include <thread>
#include <atomic>
#include <chrono>
#include <fstream>

#define MINUTES 15
#define DEVICES 6
using namespace std;

// Measurement structure
struct Record {
    int id;
    float ax, ay, az, gx, gy, gz, mx, my, mz;
    float q0, q1, q2, q3;
};

// Measurements array
Record storage[MINUTES*60*1000][DEVICES];

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
                    << "," << storage[i][k].mz;
            else
                file << ",,,,,,,,,";
        }
        file << '\n';
    }
    file.flush();
    file.close();
}

void clientThread(Socket clientSocket) {
    int pingTimer = 0; // alive test counter
    int id = 0;
    Record* recordBuffer = new Record;
    memset(recordBuffer, 0, sizeof(Record)); // Clear buffer memory
    while (1) {
        while (!isMeasuring); // Wait command
        int receivedBytes = receiveDataTCP(clientSocket, (char*)(recordBuffer), sizeof(Record));
        if (receivedBytes <= 0) break; // Interrupt if error or disconnect
        if (receivedBytes != sizeof(Record)) continue; // If transmition error
        id = recordBuffer->id;
        if ((id <= 0) || (id > 255)) continue; // Use device id as consistency indicator
        int millis = ms(startTime); // Package received time
        memcpy((void*)&(storage[millis][id%DEVICES]), (void*)(recordBuffer), sizeof(Record)); // Save measurement to storage
    }
    cout << "[INFO] Device " << id << " disconnected..." << endl;
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
    SocketProps* dataGateProps = createSocketProps(1235);
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

int main() {
    Socket acceptorSocket = createSocketTCP();
    SocketProps* acceptorProps = createSocketProps(1234);
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