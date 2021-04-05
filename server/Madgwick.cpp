#include "lib/TCPSocket.cpp"
#include "lib/MadgwickAHRS.h"
#include <thread>
#include <mutex>
using namespace std;

const char* HOST_IP = "127.0.0.1";
const int HOST_PORT = 1235;
const int OUTPUT_PORT = 1236;
const int DEVICES = 6;

// Measurement structure
struct Record {
    int id;
    float ax, ay, az, gx, gy, gz, mx, my, mz;
    float q0, q1, q2, q3;
};

void printRecord(Record* rec) {
    cout << rec->id << " " 
        << rec->ax << " " 
        << rec->ay << " " 
        << rec->az << " "
        << rec->gx << " " 
        << rec->gy << " " 
        << rec->gz << " "
        << rec->mx << " " 
        << rec->my << " " 
        << rec->mz << " ";
}

float quat[4*DEVICES] = {0};
mutex quatMutex;


void clientThread(Socket clientSocket) {
    cout << "[INFO] Client connected." << endl;
    char ch;
    while (1) {
        int receivedBytes = receiveDataTCP(clientSocket, &ch, 128); // Wait request
        if (receivedBytes <= 0) break; // Interrupt if error or disconnect
        quatMutex.lock();
        string result = to_string(quat[0]);
        result += ";";
        result += to_string(quat[1]);
        result += ";";
        result += to_string(quat[2]);
        result += ";";
        result += to_string(quat[3]);
        quatMutex.unlock();
        sendDataTCP(clientSocket, result.c_str(), result.size());
    }
    close(clientSocket);
    cout << "[INFO] Client disconnected." << endl;
}

void acceptorThread() {
    Socket serverSocket = createSocketTCP();
    SocketProps* props = createSocketProps(OUTPUT_PORT);
    setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, 0, sizeof(int)); // Allow reusing port
    bindSocket(serverSocket, props);
    listenSocket(serverSocket);
    cout << "[INFO] Madgwick filter started." << endl;
    while (1) {
        Socket clientSocket = acceptSocket(serverSocket);
        thread client(clientThread, clientSocket);
        client.detach();
    }
}

int main() {
    Socket sock = createSocketTCP();
    SocketProps* props = createSocketProps(HOST_IP, HOST_PORT); // Connect to data collecting server
    connectSocket(sock, props);
    thread acceptor(acceptorThread);
    acceptor.detach();
    Record* buf = new Record[6];
    char ch = '0';
    int i = 0;
    while (1) {
        sendDataTCP(sock, &ch, 1);
        if (receiveDataTCP(sock, (char*)buf, sizeof(Record[6])) != sizeof(Record[6])) break;

        quatMutex.lock();
        for (int i = 0; i < DEVICES; i++) {
            quat[i*DEVICES] = buf[i].q0;
            quat[i*DEVICES + 1] = buf[i].q1;
            quat[i*DEVICES + 2] = buf[i].q2;
            quat[i*DEVICES + 3] = buf[i].q3;
        }
        quatMutex.unlock();
        
        if (i % 100 == 0) {
            for (int i = 0; i < DEVICES; i++)
                cout << buf[i].q0 << " " << buf[i].q1 << " " << buf[i].q2 << " " << buf[i].q3 << " > ";
            cout << endl;
        }
        i++;
        usleep(100);
    }
    close(sock);
    return 0;
}