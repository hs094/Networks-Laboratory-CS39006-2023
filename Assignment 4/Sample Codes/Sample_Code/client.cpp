#include <iostream>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <string>
#include <netinet/in.h>
#include <algorithm>
#include <netdb.h>
#include <cstring>
#include <sstream>
#include <fstream>
#define BUFF_SIZE 102400
#define SMALL_SIZE 100
using namespace std;

void filePathManager(char* filePath); // handles the front "/" in file name.
char* hostname_to_ip(char* hostname); // resolves url to ip address
void extractContentFromResponse(char* content, char* codeline, int& responseCode);
bool handleGET(char recvBuff[], char* filePath);
bool handlePUT(int& client_socket, char* filePath);


int main(int argc, char *argv[]) {

	if (argc != 5) {
		cout << "Invalid Arguments\nUsage: ./a.out <IP> <PORT> <GET/PUT> <fileName>\nExample: ./a.out 127.0.0.1 80 GET index.html" << endl;
		return 1;
	}

	char* hostSupplied = argv[1];
	char* hostResolved = hostname_to_ip(hostSupplied);
	int server_port = atoi(argv[2]);
	char* method = argv[3];
	char* filePath = argv[4];

	int client_socket = socket(AF_INET, SOCK_STREAM, 0);

	sockaddr_in server_addr;
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(server_port);
	server_addr.sin_addr.s_addr = inet_addr(hostResolved);
	socklen_t length_server = sizeof(server_addr);

	connect(client_socket, (sockaddr*)&server_addr, length_server);

	char sendbuff[BUFF_SIZE];
	sprintf(sendbuff, "%s %s HTTP/1.0\r\nHost: %s\r\n\r\n", method, filePath, hostSupplied);

	send(client_socket, sendbuff, BUFF_SIZE, 0);
	cout << "Client sent " << sendbuff << endl;

	char recvBuff[BUFF_SIZE];
	recv(client_socket, recvBuff, BUFF_SIZE, 0);

	char responseLine[SMALL_SIZE];
	int responseCode;
	extractContentFromResponse(recvBuff, responseLine, responseCode);

	cout << "Client received Response: " << responseLine << endl;

	if (strcmp(method, "GET") == 0 && responseCode == 200) {
		handleGET(recvBuff, filePath);
	}
	else if (strcmp(method, "PUT") == 0 && responseCode == 200) {
		handlePUT(client_socket, filePath);
	}

	cout << "Client Exiting...." << endl;
	close(client_socket);

}

void filePathManager(char* filePath) {

	// if only "/" is supplied, replace it with "index.html"
	if (strcmp(filePath, "/") == 0) {
		strcpy(filePath, "index.html");
	}

	// remove first "/" from the pathname
	if (filePath[0] == '/') {
		string s(filePath);
		const char* temp = s.substr(1).c_str();
		strcpy(filePath, temp);
	}

}

char* hostname_to_ip(char* hostname)
{
	struct hostent* he = gethostbyname(hostname);
	struct in_addr** addr_list = (struct in_addr **) he->h_addr_list;
	return inet_ntoa(*addr_list[0]);
}

void extractContentFromResponse(char* content, char* codeline, int& responseCode) {

	stringstream ss(content);
	string firstLine;
	getline(ss, firstLine);
	strcpy(codeline, firstLine.c_str());

	stringstream ss2(firstLine);
	ss2 >> firstLine; // waste no use
	ss2 >> responseCode;

	return;
}

bool handleGET(char recvBuff[], char* filePath) {
	cout << "Response Content : \n" << recvBuff << endl;
	filePathManager(filePath);

	// write to file
	ofstream client_file(filePath);
	stringstream ss;
	ss << recvBuff;

	client_file << ss.str();
	client_file.close();

	cout << endl << "File is saved on the client" << endl;
}

bool handlePUT(int& client_socket, char* filePath) {

	// read from the file
	stringstream ss;
	ifstream client_file(filePath);
	ss << client_file.rdbuf();
	string temp = ss.str();

	const char* fileSendBuff = temp.c_str();
	send(client_socket, fileSendBuff, BUFF_SIZE, 0);
	cout << "File sent to the server" << endl;

	client_file.close();

	return true;

}