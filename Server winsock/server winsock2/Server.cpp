#define WIN32_LEAN_AND_MEAN
#define LOCALHOST_PORT "8765"

#include <WinSock2.h> 
#include <ws2tcpip.h> // ��� addrinfo �����
#include <iostream>
#include <sstream>

#pragma comment (lib, "Ws2_32.lib")

using namespace std;

int receive_data(SOCKET client_socket, char recvbuf[], int recvbuflen)
{
	int error_code = recv(client_socket, recvbuf, recvbuflen, 0);
	return error_code;
}

void send_data(SOCKET client_socket, char recvbuf[])
{
	stringstream response; // ���� ����� ������������ ����� �������
	stringstream response_body; // ���� ������

	response_body
		<< "<title>Ma server boi</title>\n"
		<< "<h1>Hewwo</h1>\n"
		<< "<pre>" << recvbuf << "</pre>\n";

	response // ��������� ���� ����� ������ � �����������
		<< "HTTP/1.1 200 OK\r\n"
		<< "Version: HTTP/1.1\r\n"
		<< "Content-Type: text/html; charset=utf-8\r\n"
		<< "Content-Length: " << response_body.str().length()
		<< "\r\n\r\n"
		<< response_body.str();

	int send_error_code = send(client_socket, response.str().c_str(), // ���������� ����� ������� � ������� ������� send
		response.str().length(), 0);
	if (send_error_code == SOCKET_ERROR)
	{
		closesocket(client_socket);
		WSACleanup();
		throw "send failed";
	}
}

addrinfo init_socket_hints()
{
	addrinfo hints;		  // ��������(�����) ������������ ������
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;// AF_INET ����������, ��� ������������ ���� ��� ������ � �������
	hints.ai_socktype = SOCK_STREAM; // ������ ��������� ��� ������
	hints.ai_protocol = IPPROTO_TCP; // ���������� �������� TCP									 
	hints.ai_flags = AI_PASSIVE;// ����� �������� �� �����, ����� ��������� �������� ����������
	return hints;
}

int main()
{
	try
	{
		WSADATA wsaData;
		int error_code = WSAStartup(MAKEWORD(2, 2), &wsaData);
		if (error_code != 0)
		{
			throw "WSAStartup failed with error";
		}

		addrinfo* addr = NULL; // ���������, �������� ���������� �� IP-������  ���������� ������
		addrinfo hints = init_socket_hints();
		error_code = getaddrinfo(NULL, LOCALHOST_PORT, &hints, &addr);// HTTP-������ ����� ������ �� NNNN-� ����� ����������
		if (error_code != 0)
		{
			WSACleanup();
			throw "getaddrinfo failed";
		}

		SOCKET master_socket = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
		if (master_socket == INVALID_SOCKET)
		{
			WSACleanup();
			freeaddrinfo(addr);
			throw "Socket initialization failed";
		}
		error_code = bind(master_socket, addr->ai_addr, (int)addr->ai_addrlen);
		if (error_code == SOCKET_ERROR)
		{
			freeaddrinfo(addr);
			closesocket(master_socket);
			WSACleanup();
			throw "binding failed";
		}
		freeaddrinfo(addr);
		error_code = listen(master_socket, SOMAXCONN);
		if (error_code == SOCKET_ERROR)
		{
			closesocket(master_socket);
			WSACleanup();
			throw "listening failed";
		}

		while (true)
		{
			cout << "Waiting for client..." << endl;
			SOCKET client_socket = accept(master_socket, NULL, NULL);  // �������� �������
			if (client_socket == INVALID_SOCKET)
			{
				closesocket(master_socket);
				WSACleanup();
				throw "accepting failed";
			}


			const int buf_size = 1024;
			char recvbuf[buf_size];
			int recvbuflen = buf_size;
			//string recvbuf;

			error_code = receive_data(client_socket, recvbuf, recvbuflen);

			if (error_code > 0)
			{
				cout << "Receiving connection..." << endl;
				recvbuf[error_code] = '\0'; // �� ����� ������ ���������� ������, ������� ������ ����� ����� ������ � ������ �������.
				send_data(client_socket, recvbuf);
			}
			else if (error_code == 0)
			{
				cerr << "Nothing was sent, connection closing ... " << endl;
			}
			else
			{
				closesocket(client_socket);
				WSACleanup();
				throw "recv failed";
			}

			error_code = shutdown(client_socket, SD_SEND);
			if (error_code == SOCKET_ERROR)
			{
				closesocket(client_socket);
				WSACleanup();
				throw "shutdown failed";
			}
			closesocket(client_socket);
		}

		error_code = shutdown(master_socket, SD_SEND);
		if (error_code == SOCKET_ERROR)
		{
			closesocket(master_socket);
			WSACleanup();
			throw "shutdown failed";
		}
		closesocket(master_socket);
		WSACleanup();
	}
	catch (string err_num)
	{
		cout << err_num;
	}	

	return 0;
}