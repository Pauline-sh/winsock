#define WIN32_LEAN_AND_MEAN
#define LOCALHOST_PORT "8765"

#include <WinSock2.h> 
#include <ws2tcpip.h> // тут addrinfo лежит
#include <iostream>

#pragma comment (lib, "Ws2_32.lib")

#define server_ip "your_ipv4" // station

using namespace std;

addrinfo init_socket_hints_cl()
{
	addrinfo hints;		  // —войства(адрес) создаваемого сокета
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC;// ¬се равно, использовать ли ipv4 или ipv6
	hints.ai_socktype = SOCK_STREAM; // «адаем потоковый тип сокета
	hints.ai_protocol = IPPROTO_TCP; // »спользуем протокол TCP
	return hints;
}

int main()
{
	WSADATA wsaData;

	int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (result != 0)
	{
		cerr << "WSAStartup failed with error: " << result << "\n";
		cin.get();
		return result;
	}	

	const int buf_size = 1024;
	char recvbuf[buf_size];
	int recvbuflen = buf_size;
	char *sendbuf = "HEWWO IT'S ME I WAS WONDERING IF AFTER ALL THESE YEARS YOU'D LIKE TO MEET";

	struct addrinfo* addr = NULL; // структура, хран€ща€ информацию об IP-адресе  слущающего сокета		
	struct addrinfo* ptr = NULL;
	struct addrinfo hints = init_socket_hints_cl(); // Ўаблон дл€ инициализации структуры адреса	

	result = getaddrinfo(server_ip, LOCALHOST_PORT, &hints, &addr);// Resolve the server address and port
	if (result != 0)
	{
		cout << "getaddrinfo failed with error " << result << endl;
		WSACleanup();
		cin.get();
		return 1;
	}

	SOCKET ConnectSocket = INVALID_SOCKET;
	for (ptr = addr; ptr != NULL; ptr = ptr->ai_next) // Attempt to connect to an address until one succeeds
	{
		ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);// Create a SOCKET for connecting to server
		if (ConnectSocket == INVALID_SOCKET)
		{
			cout << "socket failed with error " << WSAGetLastError() << endl;
			WSACleanup();
			cin.get();
			return 1;
		}
		result = connect(ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen); // Connect to server.
		if (result == SOCKET_ERROR)
		{
			closesocket(ConnectSocket);
			ConnectSocket = INVALID_SOCKET;
			continue;
		}
		break;
	}

	freeaddrinfo(addr);

	if (ConnectSocket == INVALID_SOCKET) 
	{
		cout << "Unable to connect to server!" << endl;
		WSACleanup();
		cin.get();
		return 1;
	}
	
	result = send(ConnectSocket, sendbuf, (int)strlen(sendbuf), 0); // Send an initial buffer
	if (result == SOCKET_ERROR) 
	{
		cout << "send failed with error " << WSAGetLastError() << endl;
		closesocket(ConnectSocket);
		WSACleanup();
		cin.get();
		return 1;
	}

	cout << "Data successfully sent to server " << endl;	
	
	while (result > 0) // Receive until the peer closes the connection
	{
		result = recv(ConnectSocket, recvbuf, recvbuflen, 0);
		if (result > 0)
		{
			cout << "DATA FROM SERVER:" << endl
			<< recvbuf << endl;
		}
			
		else if (result == 0)
			cout << "Connection closed" << endl;
		else
			cout << "recv failed with error " << WSAGetLastError() << endl;
	}

	result = shutdown(ConnectSocket, SD_SEND); // shutdown the connection since no more data will be sent
	if (result == SOCKET_ERROR)
	{
		cout << "shutdown failed with error " << WSAGetLastError() << endl;
		closesocket(ConnectSocket);
		WSACleanup();
		cin.get();
		return 1;
	}
	
	cin.get();
	closesocket(ConnectSocket); // cleanup
	WSACleanup();

	return 0;
}