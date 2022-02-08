#include "https.h"
#include "logger.h"
#include "parser.h"
#include <arpa/inet.h>
#include <cstdlib>
#include <ctype.h>
#include <netinet/in.h>
#include <openssl/ssl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <sys/socket.h>
#include <thread>
#include <unistd.h>
#include <vector>

HTTPSServer::HTTPSServer(const char *certificate, const char *key,
                         const char *address, int port, int maxClients) {
  this->certificateFile = certificate;
  this->keyFile = key;
  this->address = address;
  this->port = port;
  this->maxClients = maxClients;

  std::srand(std::time(0));
}

void HTTPSServer::setHandler(HTTPHandler *handler) { this->handler = handler; }

void HTTPSServer::serveForever() {
  if (!this->setupSocket())
    return;

  while (this->running) {
    // handle request
    SSL *client = this->acceptConnection();
    if (client == nullptr) {
      SSL_shutdown(client);
      SSL_free(client);
      continue;
    }

    // create thread and handle client
    this->handleClient(client);
    // std::thread t(&HTTPSServer::handleClient, this, client);
    // t.detach();
  }

  this->closeSocket();
  this->closeSSLContext();
}

void HTTPSServer::serveOnce() {
  if (!this->setupSocket())
    return;

  // handle request
  SSL *client = this->acceptConnection();
  if (client == nullptr) {
    SSL_shutdown(client);
    SSL_free(client);
    this->closeSocket();

    return;
  }

  this->handleClient(client);

  this->closeSocket();
  this->closeSSLContext();
}

void HTTPSServer::shutdown() { this->running = false; }

bool HTTPSServer::setupSocket() {
  if (!this->setupSSLContext())
    return false;

  this->socketDiscriptor = socket(AF_INET, SOCK_STREAM, 0);

  if (this->socketDiscriptor == 0) {
    eputs("couldn't create socket");
    return false;
  }

  int option = 1;
  if (setsockopt(this->socketDiscriptor, SOL_SOCKET,
                 SO_REUSEADDR | SO_REUSEPORT, &option, sizeof(option))) {
    eputs("couldn't set socket options");
    return false;
  }

  struct sockaddr_in address;
  address.sin_family = AF_INET;
  address.sin_addr.s_addr = INADDR_ANY;
  address.sin_port = htons(this->port);

  if (bind(this->socketDiscriptor, (struct sockaddr *)&address,
           sizeof(struct sockaddr_in)) < 0) {
    eputs("couldn't bind socket to port");
    return false;
  }

  if (listen(this->socketDiscriptor, this->maxClients) < 0) {
    eputs("couldn't start listening");
    return false;
  }

  LOGGER.log("%nServer up and running on http://0.0.0.0:%d/\n", port);
  return true;
}

bool HTTPSServer::setupSSLContext() {
  const SSL_METHOD *method = TLS_server_method();
  this->sslContext = SSL_CTX_new(method);
  if (!this->sslContext) {
    eputs("couldn't create ssl context");
    return false;
  }

  if (SSL_CTX_use_certificate_file(this->sslContext, this->certificateFile,
                                   SSL_FILETYPE_PEM) <= 0) {
    eputs("couldn't load public certificate");
    return false;
  }

  if (SSL_CTX_use_PrivateKey_file(this->sslContext, this->keyFile,
                                  SSL_FILETYPE_PEM) <= 0) {
    eputs("couldn't load private key");
    return false;
  }

  return true;
}

void HTTPSServer::closeSSLContext() { SSL_CTX_free(this->sslContext); }

void HTTPSServer::closeSocket() { close(this->socketDiscriptor); }

SSL *HTTPSServer::acceptConnection() {
  int client = accept(this->socketDiscriptor, nullptr, nullptr);

  SSL *ssl = SSL_new(this->sslContext);
  SSL_set_fd(ssl, client);

  if (SSL_accept(ssl) <= 0) {
    eputs("couldn't accept ssl connection");
    return nullptr;
  }

  return ssl;
}

void HTTPSServer::handleClient(SSL *client) {
  HTTPRequest req;
  HTTPResponse res;

  int fd = SSL_get_fd(client);
  int clientNum = fd ^ std::rand() + std::rand();

  char ip[INET_ADDRSTRLEN];
  getIpAddr(fd, ip);

  bool success = this->readRequest(client, req);

  LOGGER.log("%a <%n %s %s - %s\n", clientNum,
             getMethodName(req.method).c_str(), req.resource.c_str(), ip);

  if (success) {
    this->handler->handleRequest(req, res);
    this->writeResponse(client, res);

    LOGGER.log("%a >%n %i %s %n- %s\n", clientNum, res.code,
               getResponseName(res.code).c_str(), ip);

  } else {
    LOGGER.log("%a !%n dropped %s\n", clientNum, ip);
  }

  SSL_shutdown(client);
  SSL_free(client);
  close(fd);
}

bool HTTPSServer::readRequest(SSL *client, HTTPRequest &req) {
  char *buffer = (char *)malloc(4096);
  memset(buffer, 0, 4096);

  SSL_read(client, buffer, 4096);

  return parseHttpRequest(buffer, req);
}

void HTTPSServer::writeResponse(SSL *client, HTTPResponse &res) {
  std::string response = "HTTP/" + std::to_string(res.major) + "." +
                         std::to_string(res.minor) + " " +
                         std::to_string(res.code) + "  \r\n";

  for (HTTPHeader &header : res.headers) {
    response += header.name + ": " + header.value + "\r\n";
  }

  response += "\r\n" + res.body;

  SSL_write(client, response.c_str(), response.length());
}