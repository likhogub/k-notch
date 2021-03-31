#include "lib/TCPSocket.cpp"
#include "lib/MadgwickAHRS.h"
#include <thread>
#include <mutex>
using namespace std;


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

double quat[4] = {0};
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
    SocketProps* props = createSocketProps(1236);
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
    //filter.begin(100);
    Socket sock = createSocketTCP();
    SocketProps* props = createSocketProps("127.0.0.1", 1235); // Connect to data collecting server
    connectSocket(sock, props);
    thread acceptor(acceptorThread);
    acceptor.detach();
    Record* buf = new Record[6];
    char ch = '0';
    int i = 0;
    while (1) {
        sendDataTCP(sock, &ch, 1);
        receiveDataTCP(sock, (char*)buf, sizeof(Record[6]));

        quatMutex.lock();
        quat[0] = buf->q0;
        quat[1] = buf->q1;
        quat[2] = buf->q2;
        quat[3] = buf->q3;
        quatMutex.unlock();
        //cout << quat[0] << " " << quat[1] << " " << quat[2] << " " << quat[3] << endl;
        usleep(100);
    }

    return 0;
}