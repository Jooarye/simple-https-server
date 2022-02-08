#include "http.h"
#include "logger.h"
#include "parser.h"
#include <arpa/inet.h>
#include <cstdlib>
#include <ctype.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <sys/socket.h>
#include <thread>
#include <unistd.h>
#include <vector>

void getIpAddr(int client, char str[INET_ADDRSTRLEN]) {
  memcpy(str, "999.999.999.999\0", 16);

  struct sockaddr_in addr;
  int len = sizeof(sockaddr_in);

  if (getsockname(client, (struct sockaddr *)&addr, (socklen_t *)&len) == -1) {
    return;
  }

  inet_ntop(AF_INET, &addr.sin_addr, str, INET_ADDRSTRLEN);

  return;
}

HTTPMethod getMethod(std::string str) {
  if (str == "GET")
    return HTTP_GET;
  else if (str == "HEAD")
    return HTTP_HEAD;
  else if (str == "POST")
    return HTTP_POST;
  else if (str == "PUT")
    return HTTP_PUT;
  else if (str == "DELETE")
    return HTTP_DELETE;
  else if (str == "CONNECT")
    return HTTP_CONNECT;
  else if (str == "OPTIONS")
    return HTTP_OPTIONS;
  else if (str == "TRACE")
    return HTTP_TRACE;
  else if (str == "PATCH")
    return HTTP_PATCH;

  return HTTP_NONE;
}

std::string getMethodName(HTTPMethod method) {
  switch (method) {
  case HTTP_NONE:
    return "NONE";
  case HTTP_GET:
    return "GET";
  case HTTP_HEAD:
    return "HEAD";
  case HTTP_POST:
    return "POST";
  case HTTP_PUT:
    return "PUT";
  case HTTP_DELETE:
    return "DELETE";
  case HTTP_CONNECT:
    return "CONNECT";
  case HTTP_OPTIONS:
    return "OPTIONS";
  case HTTP_TRACE:
    return "TRACE";
  case HTTP_PATCH:
    return "PATCH";
  default:
    return "NONE";
  }
}

std::string getResponseName(HTTPResponseCode code) {
  switch (code) {
  case HTTP_CONTINUE:
    return "Continue";
  case HTTP_SWITCHING_PROTOCOLS:
    return "Switching Protocols";
  case HTTP_EARLY_HINTS:
    return "Early Hints";
  case HTTP_OK:
    return "OK";
  case HTTP_CREATED:
    return "Created";
  case HTTP_ACCEPTED:
    return "Accepted";
  case HTTP_NON_AUTHORITATIVE_INFORMATION:
    return "Non Authoritative Information";
  case HTTP_NO_CONTENT:
    return "No Content";
  case HTTP_RESET_CONTENT:
    return "Reset Content";
  case HTTP_PARTIAL_CONTENT:
    return "Partial Content";
  case HTTP_MULTIPLE_CHOICE:
    return "Multiple Choice";
  case HTTP_MOVED_PERMANENTLY:
    return "Moved Permanently";
  case HTTP_FOUND:
    return "Found";
  case HTTP_SEE_OTHER:
    return "See Other";
  case HTTP_NOT_MODIFIED:
    return "Not Modified";
  case HTTP_TEMPORARY_REDIRECT:
    return "Temporary Redirect";
  case HTTP_PERMANENT_REDIRECT:
    return "Permanent Redirect";
  case HTTP_BAD_REQUEST:
    return "Bad Request";
  case HTTP_UNAUTHORIZED:
    return "Unauthorized";
  case HTTP_PAYMENT_REQUIRED:
    return "Payment Required";
  case HTTP_FORBIDDEN:
    return "Forbidden";
  case HTTP_NOT_FOUND:
    return "Not Found";
  case HTTP_METHOD_NOT_ALLOWED:
    return "Method Not Allowed";
  case HTTP_NOT_ACCEPTABLE:
    return "Not Acceptable";
  case HTTP_PROXY_AUTHENTICATION_REQUIRED:
    return "Proxy Authentication Required";
  case HTTP_REQUEST_TIMEOUT:
    return "Request Timeout";
  case HTTP_CONFLICT:
    return "Conflict";
  case HTTP_GONE:
    return "Gone";
  case HTTP_LENGTH_REQUIRED:
    return "Length Required";
  case HTTP_PRECONDITION_FAILED:
    return "Precondition Failed";
  case HTTP_PAYLOAD_TOO_LARGE:
    return "Payload Too Large";
  case HTTP_URI_TOO_LONG:
    return "Uri Too Long";
  case HTTP_UNSUPPORTED_MEDIA_TYPE:
    return "Unsupported Media Type";
  case HTTP_RANGE_NOT_STATISFIABLE:
    return "Range Not Statisfiable";
  case HTTP_EXPECTATION_FAILED:
    return "Expectation Failed";
  case HTTP_UNPROCESSABLE_ENTITY:
    return "Unprocessable Entity";
  case HTTP_TOO_EARLY:
    return "Too Early";
  case HTTP_UPGRADE_REQUIRED:
    return "Upgrade Required";
  case HTTP_PRECONDITION_REQUIRED:
    return "Precondition Required";
  case HTTP_TOO_MANY_REQUESTS:
    return "Too Many Requests";
  case HTTP_REQUEST_HEADER_FIELDS_TOO_LARGE:
    return "Request Header Fields Too Large";
  case HTTP_UNAVAILABLE_FOR_LEGAL_REASONS:
    return "Unavailable For Legal Reasons";
  case HTTP_INTERNAL_SERVER_ERROR:
    return "Internal Server Error";
  case HTTP_NOT_IMPLEMENTED:
    return "Not Implemented";
  case HTTP_BAD_GATEWAY:
    return "Bad Gateway";
  case HTTP_SERVICE_UNAVAILABLE:
    return "Service Unavailable";
  case HTTP_GATEWAY_TIMEOUT:
    return "Gateway Timeout";
  case HTTP_VERSION_NOT_SUPPORTED:
    return "Version Not Supported";
  case HTTP_VARIANT_ALSO_NEGOTIATES:
    return "Variant Also Negotiates";
  case HTTP_INSUFFICIENT_STORAGE:
    return "Insufficient Storage";
  case HTTP_LOOP_DETECTED:
    return "Loop Detected";
  case HTTP_NOT_EXTENDED:
    return "Not Extended";
  case HTTP_NETWORK_AUTHENTICATION_REQUIRED:
    return "Network Authentication Required";
  default:
    return "Unknown Response Code";
  }
}

void HTTPResponse::setHeader(std::string name, std::string value) {
  this->headers.push_back(HTTPHeader{name, value});
}

void HTTPHandler::handleRequest(HTTPRequest &req, HTTPResponse &res) {}

void HTTPHandler::handleNotFound(HTTPRequest &req, HTTPResponse &res) {
  res.code = HTTP_NOT_FOUND;
  res.body = "<h1>404 - Not Found</h1>";
}

void HTTPRouter::handleRequest(HTTPRequest &req, HTTPResponse &res) {
  for (std::pair<std::string, HTTPHandler *> pair : this->handlers) {
    if (req.resource.starts_with(pair.first)) {
      req.resource.erase(0, pair.first.length());
      pair.second->handleRequest(req, res);
      return;
    }
  }

  this->handleNotFound(req, res);
}

void HTTPRouter::addRoute(std::string path, HTTPHandler *handler) {
  this->handlers[path] = handler;
}

HTTPServer::HTTPServer(const char *addr, int port, int maxClients) {
  this->address = addr;
  this->port = port;
  this->maxClients = maxClients;

  std::srand(std::time(0));
}

void HTTPServer::setHandler(HTTPHandler *handler) { this->handler = handler; }

void HTTPServer::serveForever() {
  if (!this->setupSocket())
    return;

  while (this->running) {
    // handle request
    int client = this->acceptConnection();
    if (client < 0) {
      eputs("couldn't accept client connection");
      return;
    }

    // create thread and handle client
    // this->handleClient(client);
    std::thread t(&HTTPServer::handleClient, this, client);
    t.detach();
  }
}

void HTTPServer::serveOnce() {
  if (!this->setupSocket())
    return;

  // handle request
  int client = this->acceptConnection();
  if (client < 0) {
    eputs("couldn't accept client connection");
    return;
  }

  this->handleClient(client);

  this->closeSocket();
}

void HTTPServer::shutdown() { this->running = false; }

bool HTTPServer::setupSocket() {
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

void HTTPServer::closeSocket() { close(this->socketDiscriptor); }

int HTTPServer::acceptConnection() {
  return accept(this->socketDiscriptor, nullptr, nullptr);
}

void HTTPServer::handleClient(int client) {
  HTTPRequest req;
  HTTPResponse res;

  int clientNum = client ^ std::rand() + std::rand();

  char ip[INET_ADDRSTRLEN];
  getIpAddr(client, ip);

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

  close(client);
}

bool HTTPServer::readRequest(int client, HTTPRequest &req) {
  char *buffer = (char *)malloc(4096);
  memset(buffer, 0, 4096);

  read(client, buffer, 4096);

  return parseHttpRequest(buffer, req);
}

void HTTPServer::writeResponse(int client, HTTPResponse &res) {
  std::string response = "HTTP/" + std::to_string(res.major) + "." +
                         std::to_string(res.minor) + " " +
                         std::to_string(res.code) + "  \r\n";

  for (HTTPHeader &header : res.headers) {
    response += header.name + ": " + header.value + "\r\n";
  }

  response += "\r\n" + res.body;

  write(client, response.c_str(), response.length());
}