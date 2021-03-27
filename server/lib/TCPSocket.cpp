#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <iostream>
using namespace std;

int PACKAGE_SIZE = 128;
char LOCALHOST[]  = "127.0.0.1";

typedef sockaddr_in SocketProps;
typedef int Socket;

Socket createSocketTCP() {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        perror("TCP socket creating error");
    }
    return sock;
}

SocketProps* createSocketProps(const char* address, int port) {
    SocketProps* socketProps = new SocketProps;
    memset(socketProps, 0, sizeof(*socketProps));
    socketProps->sin_family = AF_INET;
    socketProps->sin_port = htons(port);
    if (strncmp(address, "", 2))
        socketProps->sin_addr.s_addr = inet_addr(address);
    else
        socketProps->sin_addr.s_addr = INADDR_ANY;
    return socketProps;
}

SocketProps* createSocketProps(int port) {
    return createSocketProps("", port);
}

SocketProps* createSocketProps() {
    SocketProps* socketProps = new SocketProps;
    return socketProps;
}

Socket listenSocket(Socket sock, int queueLength=-1) {
    if (-1 == listen(sock, queueLength)) {
        perror("Socket listen error");
    }
    return 0;
}

Socket bindSocket(Socket sock, SocketProps* socketProps) {
    if (-1 == bind(sock, (sockaddr* )socketProps, sizeof(*socketProps))) {
        perror("Socket bind error");
    }
    return sock;
}

Socket connectSocket(Socket sock, SocketProps* socketProps) {
    return connect(sock, (sockaddr *)socketProps, sizeof(*socketProps));
}

Socket acceptSocket(Socket sock, SocketProps *socketProps) {
    socklen_t socketPropsSize = sizeof(*socketProps);
    Socket acceptedSocket = accept(sock, (sockaddr*)socketProps, &socketPropsSize);
    if (acceptedSocket == -1) {
        perror("Socket accept error");
    }
    return acceptedSocket;
}

Socket acceptSocket(Socket sock) {
    SocketProps* socketProps = new SocketProps;
    return acceptSocket(sock, socketProps);
}

int sendDataTCP(Socket sock, const char* buffer, int bytes=PACKAGE_SIZE) {
    int len = send(sock, buffer, bytes, 0);
    if (len == -1) {
        perror("Sending error");
    }
    return len;
}

int receiveDataTCP(Socket sock, char* buffer, int bytes=PACKAGE_SIZE) {
    int tmp = recv(sock, buffer, bytes, 0);
    return tmp;
}