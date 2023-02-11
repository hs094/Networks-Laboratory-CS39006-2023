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
#define QSIZE 10
using namespace std;

void filePathManager(char* filePath); // // handles the front "/" in file name.
bool handleGET(int new_socket, char* recv_buff, char* filePath);
bool handlePUT(int new_socket, char* recv_buff, char* filePath);

int main(int argc, char* argv[]) {

	if (argc != 2) {
		cout << "Invalid Arguments\nUsage: ./a.out <PORT>\nExample: ./a.out 8521" << endl;
		return 1;
	}

	int serv_socket = socket(AF_INET, SOCK_STREAM, 0);
	int port = atoi(argv[1]);

	sockaddr_in server_addr;
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port);
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	socklen_t length_server = sizeof(server_addr);

	bind(serv_socket, (sockaddr*)&server_addr, length_server);

	listen(serv_socket, QSIZE);

	while (true) {

		sockaddr_in client_addr;
		socklen_t clientSize;

		int new_socket = accept(serv_socket, (sockaddr*)&client_addr, &clientSize);

		int pid = fork();
		if (pid == 0) {

			if (new_socket > 0) {
				cout << "Client connected " << endl;
			}

			int responseCode;
			char recv_buff[BUFF_SIZE];
			char send_buff[BUFF_SIZE];

			recv(new_socket, recv_buff, BUFF_SIZE, 0);

			stringstream ss(recv_buff);
			string method;
			ss >> method; // first word

			char filePath[SMALL_SIZE];
			ss >> filePath; // second word

			if (method.compare("GET") == 0) {
				handleGET(new_socket, recv_buff, filePath);
			}
			else if (method.compare("PUT") == 0) {
				handlePUT(new_socket, recv_buff, filePath);
			}
			close(new_socket);
			break; // break the while loop in child 
		}
		else {
			close(new_socket);
		}

	}
	return 0;

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

bool handleGET(int new_socket, char* recv_buff, char* filePath) {

	filePathManager(filePath);
	cout << "File Path received: " << filePath << endl;
	stringstream ss;
	char output[BUFF_SIZE];
	memset(output, 0, BUFF_SIZE);

	ifstream server_file(filePath);
	if (!server_file.good()) {
		sprintf(output, "HTTP/1.0 404 NotFound\r\n\r\n");
	}
	else {
		// Read from file 
		ss << server_file.rdbuf();
		string fileContent_string = ss.str();
		const char* fileContents = fileContent_string.c_str();
		sprintf(output, "HTTP/1.0 200 OK\r\n\r\n%s", fileContents);
	}

	send(new_socket, output, BUFF_SIZE, 0);
	cout << "Data Sent" << endl;

	server_file.close();
	return true;

}

bool handlePUT(int new_socket, char* recv_buff, char* filePath) {

	filePathManager(filePath);
	cout << "File Path received: " << filePath << endl;

	// first send 200 okay reply
	char okayReply[SMALL_SIZE];
	sprintf(okayReply, "HTTP/1.0 200 OK\r\n\r\n");
	send(new_socket, okayReply, SMALL_SIZE, 0);

	// wait for the contents to be received
	recv(new_socket, recv_buff, BUFF_SIZE, 0);

	// write the contents to file
	ofstream server_file(filePath);
	stringstream ss;
	ss << recv_buff;
	server_file << ss.str();
	server_file.close();

	cout << "File is created on the server" << endl;
	send(new_socket, okayReply, SMALL_SIZE, 0);

	return true;

}