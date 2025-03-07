#include <iostream>
#include <winsock2.h> // Core Sockets (Windows)

#define DEFAULT_SERVER_IP "127.0.0.1"
#define DEFAULT_PORT 1024
#define BUFFER_SIZE 1024

struct SocketInfo {
    SOCKET clientSocket;
    sockaddr_in serverAddress;
};

SocketInfo createClientSocket() {
    WSADATA winSockData; // Initializes Winsock
    if (WSAStartup(MAKEWORD(2, 2), &winSockData) != 0) {
        std::cerr << "WSAStartup failed!" << std::endl;
        exit(1);
    }

    SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, 0); // Creates Socket
    if (clientSocket == INVALID_SOCKET) {
        std::cerr << "Socket creation failed: " << WSAGetLastError() << std::endl;
        WSACleanup(); // Terminates Winsock
        exit(1);
    }

    sockaddr_in serverAddress; // Stores Address of Socket
    serverAddress.sin_family = AF_INET; // IPv4

    std::cout << "Client socket created!" << std::endl;
    return {clientSocket, serverAddress};
}

void connectClient(SOCKET clientSocket, sockaddr_in serverAddress, int port) {
    serverAddress.sin_port = htons(static_cast<u_short>(port)); // Set Server Port
    if (connect(clientSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == SOCKET_ERROR) {
        std::cerr << "Connection failed: " << WSAGetLastError() << std::endl;
        closesocket(clientSocket);
        WSACleanup();
        exit(1);
    }

    std::cout << "Connected to server!" << std::endl;
}

void sendCommand(SOCKET clientSocket) {
    std::string command;
    std::cout << "Enter command to execute (type 'exit' to quit): ";
    std::cin >> command;

    if (command == "exit") {
        send(clientSocket, "exit", 4, 0); // Sending exit signal to the server
        std::cout << "Exiting client." << std::endl;
        return;  // Exit the function
    }

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
    std::string serverIP = DEFAULT_SERVER_IP;
    int port = DEFAULT_PORT;

    // Parse command-line arguments
    if (argc > 1) { serverIP = argv[1]; }
    if (argc > 2) { port = atoi(argv[2]); }

    std::cout << "Connecting to Server IP: " << serverIP << " on Port: " << port << std::endl;

    SocketInfo socketInfo = createClientSocket();
    socketInfo.serverAddress.sin_addr.s_addr = inet_addr(serverIP.c_str()); // Set Server IP
    connectClient(socketInfo.clientSocket, socketInfo.serverAddress, port);

    while (true) {
        sendCommand(socketInfo.clientSocket);
    }

    return 0;
}
