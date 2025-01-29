#include <iostream>

#ifdef _WIN32
#include <winsock2.h> // Core Sockets (Windows)
#else
#include <sys/socket.h> // Core Sockets (Linux)
#endif

#define DEFAULT_CLIENT_IP "127.0.0.1"
#define DEFAULT_SERVER_IP "127.0.0.1"
#define DEFAULT_PORT 1024
#define BUFFER_SIZE 1024

struct SocketInfo {
    SOCKET linuxClientSocket;
    SOCKET windowsClientSocket;
    sockaddr_in serverAddress;
};

SocketInfo createSocketWindows() {
    WSADATA winSockData; // Initializes Winsock
    if (WSAStartup(MAKEWORD(2, 2), &winSockData) != 0) {
        std::cerr << "WSAStartup failed!" << std::endl;
        exit(1);
    }

    SOCKET windowsClientSocket = socket(AF_INET, SOCK_STREAM, 0); // Creates Socket
    if (windowsClientSocket == INVALID_SOCKET) {
        std::cerr << "Socket creation failed: " << WSAGetLastError() << std::endl;
        WSACleanup(); // Terminates Winsock
        exit(1);
    }

    sockaddr_in serverAddress; // Stores Address of Socket
    serverAddress.sin_family = AF_INET; // IPv4

    std::cout << "Socket created!" << std::endl;
    return {INVALID_SOCKET, windowsClientSocket, serverAddress};
}

SocketInfo createSocketLinux() {
    SOCKET linuxClientSocket = socket(AF_INET, SOCK_STREAM, 0); // Creates Socket
    if (linuxClientSocket == INVALID_SOCKET) {
        perror("Socket creation failed!");
        exit(1);
    }

    sockaddr_in serverAddress; // Stores Address of Socket
    serverAddress.sin_family = AF_INET; // IPv4

    std::cout << "Socket created!" << std::endl;
    return {linuxClientSocket, INVALID_SOCKET, serverAddress};
}

void connectClientWindows(SOCKET clientSocket, sockaddr_in serverAddress) {
    if (connect(clientSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == SOCKET_ERROR) { // Connects to Open Server Socket
        std::cerr << "Socket connection failed: " << WSAGetLastError() << std::endl;
        closesocket(clientSocket); // Closes Socket
        WSACleanup(); // Terminates Winsock
        exit(1);
    }
}

void connectClientLinux(int clientSocket, sockaddr_in serverAddress) {
    if (connect(static_cast<SOCKET>(clientSocket), (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == -1) { // Connects to Open Server Socket
        perror("Socket connection failed!");
        exit(1);
    }
}

int main(int argc, char* argv[]) {
    std::string clientIP = DEFAULT_CLIENT_IP;
    std::string serverIP = DEFAULT_SERVER_IP;
    int port = DEFAULT_PORT;

    // Parse command-line arguments
    if (argc > 1) { serverIP = argv[1]; } // Set Server IP
    if (argc > 2) { port = atoi(argv[2]); } // Set Server Port

    std::cout << "Connecting to Server IP: " << serverIP << " on Port: " << port << std::endl;

    SocketInfo socketInfo;
    #ifdef _WIN32 // Determines the OS (Windows or Linux)
    std::cout << "Running on Windows" << std::endl;
    socketInfo = createSocketWindows();
    socketInfo.serverAddress.sin_port = htons(static_cast<u_short>(port)); // Set Server Port
    socketInfo.serverAddress.sin_addr.s_addr = inet_addr(serverIP.c_str()); // Set Server IP
    connectClientWindows(socketInfo.windowsClientSocket, socketInfo.serverAddress);
    #else
    std::cout << "Running on Linux" << std::endl;
    socketInfo = createSocketLinux();
    socketInfo.serverAddress.sin_port = htons(port); // Set Server Port
    socketInfo.serverAddress.sin_addr.s_addr = inet_addr(serverIP.c_str()); // Set Server IP
    connectClientLinux(socketInfo.linuxClientSocket, socketInfo.serverAddress);
    #endif

    return 0;
}