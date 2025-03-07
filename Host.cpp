#include <iostream>
#include <winsock2.h> // Core Sockets (Windows)
#include <windows.h>

#define DEFAULT_CLIENT_IP "127.0.0.1"
#define DEFAULT_SERVER_IP "127.0.0.1"
#define DEFAULT_PORT 1024
#define BUFFER_SIZE 1024

struct SocketInfo {
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
    return {windowsClientSocket, serverAddress};
}

void connectClientWindows(SOCKET clientSocket, sockaddr_in serverAddress) {
    if (connect(clientSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == SOCKET_ERROR) { // Connects to Open Server Socket
        int errorCode = WSAGetLastError();
        if (errorCode == WSAECONNREFUSED) {
            std::cerr << "Connection was refused by the server." << std::endl;
        }
        else if (errorCode == WSAETIMEDOUT) {
            std::cerr << "Connection timed out." << std::endl;
        }
        else {
            std::cerr << "Unknown error: " << errorCode << std::endl;
        }
        closesocket(clientSocket); // Closes Socket
        WSACleanup(); // Terminates Winsock
        exit(1);
    }
    std::cout << "Client Connected!" << std::endl;
}

void sendCommand(SOCKET clientSocket) {
    std::string command;
    std::cout << "Enter command to execute: ";
    std::cin >> command;

    // Send command
    send(clientSocket, command.c_str(), command.length(), 0);

    // Receive output
    char buffer[BUFFER_SIZE] = {0};
    int recvResult = recv(clientSocket, buffer, BUFFER_SIZE - 1, 0);
    if (recvResult > 0) {
        buffer[recvResult] = '\0';
        std::cout << "Command output:\n" << buffer << std::endl;
    } else {
        std::cerr << "Failed to receive output: " << WSAGetLastError() << std::endl;
    }
}

int main(int argc, char* argv[]) {
    std::string clientIP = DEFAULT_CLIENT_IP;
    std::string serverIP = DEFAULT_SERVER_IP;
    int port = DEFAULT_PORT;

    // Parse command-line arguments
    if (argc > 1) { clientIP = argv[1]; } // Set Server IP
    if (argc > 2) { port = atoi(argv[2]); } // Set Server Port

    std::cout << "Connecting to Client IP: " << clientIP << " on Port: " << port << std::endl;

    SocketInfo socketInfo;
    socketInfo = createSocketWindows();
    socketInfo.serverAddress.sin_port = htons(static_cast<u_short>(port)); // Set Server Port
    socketInfo.serverAddress.sin_addr.s_addr = inet_addr(serverIP.c_str()); // Set Server IP
    connectClientWindows(socketInfo.windowsClientSocket, socketInfo.serverAddress);
    sendCommand(socketInfo.windowsClientSocket); // Sends commands to the client for remote execution
    return 0;
}