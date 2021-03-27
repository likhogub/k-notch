#include "lib/TCPSocket.cpp"
#include "lib/MadgwickAHRS.h"
#include "lib/MadgwickAHRS.cpp"
#include <thread>
using namespace std;


Madgwick filter;

// Measurement structure
struct Record {
    int id;
    float ax, ay, az, gx, gy, gz, mx, my, mz;
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

void clientThread(Socket clientSocket) {
    cout << "[INFO] Client connected." << endl;
    char* buf = new char[129];
    while (1) {
        int receivedBytes = receiveDataTCP(clientSocket, buf, 128); // Wait request
        if (receivedBytes <= 0) break; // Interrupt if error or disconnect
        string result = to_string(filter.q0);
        result += ";";
        result += to_string(filter.q1);
        result += ";";
        result += to_string(filter.q2);
        result += ";";
        result += to_string(filter.q3);
        result += ";";
        result += to_string(filter.getPitch());
        result += ";";
        result += to_string(filter.getRoll());
        result += ";";
        result += to_string(filter.getYaw());
        sendDataTCP(clientSocket, result.c_str(), result.size());
    }
    close(clientSocket);
    delete buf;
    cout << "[INFO] Client disconnected." << endl;
}

void acceptorThread() {
    Socket serverSocket = createSocketTCP();
    SocketProps* props = createSocketProps(1236);
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
    while (1) {
        sendDataTCP(sock, &ch, 1);
        receiveDataTCP(sock, (char*)buf, sizeof(Record[6]));
        filter.update(buf->gx, buf->gy, buf->gz, buf->ax, buf->ay, buf->az, buf->mx, buf->my, buf->mz);
        usleep(1000);
    }

    return 0;
}