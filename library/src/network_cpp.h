#include "lib.h"
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include "winsock2.h"
#pragma comment(lib, "ws2_32.lib")

struct NetworkState {
  WSAData ws;
  SOCKET s;
  SOCKADDR_IN sa;
};

global NetworkState net;

void network_init() {
  WSAStartup(MAKEWORD(2,2), &net.ws);
  
  net.s = socket(AF_INET, SOCK_STREAM, 0);
  
  net.sa.sin_family = AF_INET;
  net.sa.sin_port = htons(1234);
}

void server_run() {
  if (bind(net.s, (struct sockaddr*)&net.sa, sizeof(net.sa)) == SOCKET_ERROR) {
    Error("Bind failed: %d", WSAGetLastError());
    return;
  }
  
  if (listen(net.s, 100) == SOCKET_ERROR) {
    Error("Listen failed: %d", WSAGetLastError());
    return;
  }
  
  Info("Server is listening...");
  
  SOCKET client_socket;
  SOCKADDR_IN client_addr;
  i32 client_addr_size = sizeof(client_addr);
  
  u8 arr[20] = {};
  
  while ((client_socket = accept(net.s, (struct sockaddr*)&client_addr, &client_addr_size)) != INVALID_SOCKET) {
    Info("Client connected!");
    // Handle the client connection here
    
    while (recv(client_socket, (char*)arr, sizeof(arr), 0) > 0) {
      Info("Server read: %s", String(arr));
      
      u8 to_client[] = "hello client!";
      send(client_socket, (char*)to_client, sizeof(to_client), 0);
    }
  }
  
  // If accept() fails, print the error
  Error("Accept failed: %d", WSAGetLastError());
}

void connect_to_server() {
  net.sa.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");

  if (connect(net.s, (struct sockaddr*)&net.sa, sizeof(net.sa)) == SOCKET_ERROR) {
    Error("Connect failed: %d", WSAGetLastError());
    return;
  }
  
  Info("Connected to server!");
  
  u8 arr[20] = "hello server!";
  send(net.s, (char*)arr, cstr_length(arr), 0);
  MemZeroArray(arr);
  recv(net.s, (char*)arr, sizeof(arr), 0);
  Info("%s", String(arr));
  
  os_sleep(3000);
  
  closesocket(net.s);
}
