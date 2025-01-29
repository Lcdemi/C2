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

    SOCKET windowsServerSocket = socket(AF_INET, SOCK_STREAM, 0); // Creates Socket
    if (windowsServerSocket == INVALID_SOCKET) {
        std::cerr << "Socket creation failed: " << WSAGetLastError() << std::endl;
        WSACleanup(); // Terminates Winsock
        exit(1);
    }

    sockaddr_in serverAddress; // Stores Address of Socket
    serverAddress.sin_family = AF_INET; // IPv4

    std::cout << "Socket created!" << std::endl;
    return {INVALID_SOCKET, windowsServerSocket, serverAddress};
}

SocketInfo createSocketLinux() {
    SOCKET linuxServerSocket = socket(AF_INET, SOCK_STREAM, 0); // Creates Socket
    if (linuxServerSocket == INVALID_SOCKET) {
        perror("Socket creation failed!");
        exit(1);
    }

    sockaddr_in serverAddress; // Stores Address of Socket
    serverAddress.sin_family = AF_INET; // IPv4

    std::cout << "Socket created!" << std::endl;
    return {linuxServerSocket, INVALID_SOCKET, serverAddress};
}

void connectSocketWindows(SOCKET serverSocket, sockaddr_in serverAddress) {
    if (bind(serverSocket, (sockaddr*)&serverAddress, sizeof(serverAddress)) == SOCKET_ERROR) { // Binds Socket to Address
        std::cerr << "Socket binding failed: " << WSAGetLastError() << std::endl;
        closesocket(serverSocket); // Closes Socket
        WSACleanup(); // Terminates Winsock
        exit(1);
    }

    std::cout << "Windows socket bound successfully!" << std::endl;

    if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR) { // Listens for Connections
        std::cerr << "Socket listening failed: " << WSAGetLastError() << std::endl;
        closesocket(serverSocket); // Closes Socket
        WSACleanup(); // Terminates Winsock
        exit(1);
    }

    std::cout << "Windows socket is now listening!" << std::endl;

    SOCKET clientSocket = accept(serverSocket, NULL, NULL); // Accepts Connection
    if (clientSocket == INVALID_SOCKET) {
        std::cerr << "Socket accepting failed: " << WSAGetLastError() << std::endl;
        closesocket(serverSocket); // Closes Socket
        WSACleanup(); // Terminates Winsock
        exit(1);
    }

    std::cout << "Windows socket accepted connection!" << std::endl;
}

void connectSocketLinux(SOCKET serverSocket, sockaddr_in serverAddress) {
    if (bind(serverSocket, (sockaddr*)&serverAddress, sizeof(serverAddress)) == -1) { // Binds Socket to Address
        perror("Socket binding failed!");
        closesocket(serverSocket); // Closes Socket
        exit(1);
    }

    std::cout << "Linux socket bound successfully!" << std::endl;

    if (listen(serverSocket, SOMAXCONN) == -1) { // Listens for Connections
        perror("Socket listening failed!");
        closesocket(serverSocket); // Closes Socket
        exit(1);
    }

    std::cout << "Linux socket is now listening!" << std::endl;

    SOCKET clientSocket = accept(serverSocket, NULL, NULL); // Accepts Connection
    if (clientSocket == INVALID_SOCKET) {
        perror("Socket accepting failed!");
        closesocket(serverSocket); // Closes Socket
        exit(1);
    }

    std::cout << "Linux socket accepted connection!" << std::endl;
}

int main(int argc, char* argv[]) {
    std::string clientIP = DEFAULT_CLIENT_IP;
    int port = DEFAULT_PORT;

    // Parse command-line arguments
    if (argc > 1) { clientIP = argv[1]; } // Set Server IP
    if (argc > 2) { port = atoi(argv[2]); } // Set Port

    std::cout << "Client IP: " << clientIP << std::endl;
    std::cout << "Port: " << port << std::endl;

    SocketInfo socketInfo;
    #ifdef _WIN32 // Determines the OS (Windows or Linux)
    std::cout << "Running on Windows" << std::endl;
    socketInfo = createSocketWindows();
    socketInfo.serverAddress.sin_port = htons(static_cast<u_short>(port)); // Set Port
    socketInfo.serverAddress.sin_addr.s_addr = inet_addr(clientIP.c_str()); // Set Server IP
    connectSocketWindows(static_cast<SOCKET>(socketInfo.windowsClientSocket), socketInfo.serverAddress);
    #else
    std::cout << "Running on Linux" << std::endl;
    socketInfo = createSocketLinux();
    socketInfo.serverAddress.sin_port = htons(port); // Set Port
    socketInfo.serverAddress.sin_addr.s_addr = inet_addr(serverIP.c_str()); // Set Server IP
    connectClientLinux(socketInfo.linuxClientSocket, socketInfo.serverAddress);
    #endif

    return 0;
}
