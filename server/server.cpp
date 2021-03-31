#include "TCPSocket.cpp"
#include <pthread.h>
#include <chrono>
#include <mutex>
#include <fstream>

using namespace std;
mutex maxIdMutex;

int packets[256] = {0};

int arrId[256] = {0};

struct Record {
    int id;
    float ax, ay, az, gx, gy, gz, mx, my, mz;
    float q0, q1, q2, q3;
};

struct ThreadArgs {
    Socket clientSocket;
    Record* recordBuffer;
};

std::chrono::duration<long int, std::ratio<1, 1000000000>>::rep startTime;
std::chrono::duration<long int, std::ratio<1, 1000000000>>::rep endTime;

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

void showPacketCount() {
    cout << "Package stats: ";
    bool isAny = false;
    for (int i = 0; i < 256; i++) {
        int c = packets[i];
        if (c) {
            if (isAny) 
                cout << ",";
            cout << i << ":" << packets[i];
            isAny = true;
        }
    }
    if (!isAny)
        cout << "no packages" << endl;
    
    else 
        cout << endl;
}

Record storage[7500000][6] = {0};
bool wait = true; 
void* clientThread(void* _args) {
    ThreadArgs* args = (ThreadArgs*)_args;
    memset(args->recordBuffer, 0, sizeof(Record));
    while (1) {
        while (wait) {}
        int tmp = receiveDataTCP(args->clientSocket, (char*)(args->recordBuffer), sizeof(Record));
        if (0 >= tmp) break;
        if (tmp != sizeof(Record)) continue;
        int id = args->recordBuffer->id;
        if ((id <= 0) || (id > 255)) continue;
        int ms = (std::chrono::system_clock::now().time_since_epoch().count() - startTime)/1000000;
        int packIndex = packets[id];
        if ((packIndex % 1000) == 0) {
            //cout << packets[args->recordBuffer->id] << " ";
            //printRecord(args->recordBuffer);
            //cout << id << " " << ms << " " << packIndex << endl;;
            //showPacketCount();
        }
        memcpy((void*)&(storage[ms][id%6]), (void*)(args->recordBuffer), sizeof(Record));
        //printRecord(&storage[ms][id]);
        packets[id]++;
    }
    cout << "Thread offline " <<  args->clientSocket << endl;
    close(args->clientSocket);
    delete args->recordBuffer;
    delete args;
    return 0;
}

void* acceptorThread(void* _sock) {
    cout << "SERVER STARTED" << endl;
    while(1) {
        Socket sock = *(Socket*)_sock;
        struct timeval tv;
        tv.tv_sec = 2;
        tv.tv_usec = 0;

        cout << "Waiting" << endl;
        Socket clientSocket = acceptSocket(sock);
        cout << "Client connected " << clientSocket << endl;
        setsockopt(clientSocket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);
        ThreadArgs* args = new ThreadArgs;
        args->clientSocket = clientSocket;
        args->recordBuffer = new Record;
        pthread_t thread_ptr;
        pthread_create(&thread_ptr, NULL, clientThread, args);
    }
    cout << "SERVER STOPPED" << endl;
}

int max(int* arr, int size) {
    int m = arr[0];
    for (int i = 0; i < size; i++) {
        m = (m < arr[i]) ? arr[i] : m;
    }
    return m;
}


void saveToCSV(long start, long stop) {
    cout << start << " " << stop << endl;
    fstream file("rec.csv", ios::out);
    for (int i = 0; i < (stop-start)/1000000; i++) {
        file << i;
        for (int k = 0; k < 6; k++) {
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

int main() {
    Socket sock = createSocketTCP();
    SocketProps* props = createSocketProps(1234);
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, 0, sizeof(int));
    cout << sock << endl;
    bindSocket(sock, props);
    listenSocket(sock);

    pthread_t acceptor_thread_ptr;
    pthread_create(&acceptor_thread_ptr, NULL, acceptorThread, &sock);
    while(1){
        char ch;
        cin >> ch;
        if (ch == 'p') {
            //pthread_cancel(acceptor_thread_ptr);
            int ms;
            if (wait)
                ms = (endTime - startTime)/1000000;
            else 
                ms = (std::chrono::system_clock::now().time_since_epoch().count() - startTime) / 1000000;
            
            for (int i = 0; i < ms; i++) {
                cout << i << " ";
                for (int k = 0; k < 6; k++)
                    printRecord(&storage[i][k]);
                cout << endl;
            }
        } else if (ch == 'c') {
            showPacketCount();
        } else if (ch == '+') {
            if (wait) {
                startTime = std::chrono::system_clock::now().time_since_epoch().count();
                wait = false;
                cout << "[INFO] Recording started." << endl;
            } else {
                cout << "[WARNING] Recording already started." << endl;
            }

        } else if (ch == '-') {
            if (!wait) {
                wait = true;
                endTime = std::chrono::system_clock::now().time_since_epoch().count();
                cout << "[INFO] Recording stopped." << endl;
            } else {
                cout << "[WARNING] Recording already stopped." << endl;
            }
        } else if (ch == 'r') {
            if (wait) {
                memset(&storage, 0, sizeof(storage));
                cout << "[INFO] Memory cleared." << endl;
            } else {
                cout << "[WARNING] Memory not cleared. Recording now." << endl;
            }
        } else if (ch == 's') {
            saveToCSV(startTime, endTime);
                cout << "[INFO] Recording saved." << endl;
        }
    };

    return 0;
}
