#pragma once

#include <arpa/inet.h>
#include <map>
#include <netinet/in.h>
#include <string>
#include <vector>

#define eputs(msg) LOGGER.log("%rERROR: %s\n", msg);

void getIpAddr(int, char[INET_ADDRSTRLEN]);

struct HTTPHeader {
  std::string name;
  std::string value;
};

struct HTTPParam {
  std::string name;
  std::string value;
};

enum HTTPMethod {
  HTTP_NONE,
  HTTP_GET,
  HTTP_HEAD,
  HTTP_POST,
  HTTP_PUT,
  HTTP_DELETE,
  HTTP_CONNECT,
  HTTP_OPTIONS,
  HTTP_TRACE,
  HTTP_PATCH
};

HTTPMethod getMethod(std::string);

std::string getMethodName(HTTPMethod);

class HTTPRequest {
public:
  HTTPMethod method;
  std::string resource, fullResource;
  int major, minor;

  std::vector<HTTPHeader> headers;
  std::vector<HTTPParam> params;
  std::string body;
};

enum HTTPResponseCode {
  HTTP_CONTINUE = 100,
  HTTP_SWITCHING_PROTOCOLS = 101,
  HTTP_EARLY_HINTS = 103,

  HTTP_OK = 200,
  HTTP_CREATED = 201,
  HTTP_ACCEPTED = 202,
  HTTP_NON_AUTHORITATIVE_INFORMATION = 203,
  HTTP_NO_CONTENT = 204,
  HTTP_RESET_CONTENT = 205,
  HTTP_PARTIAL_CONTENT = 206,

  HTTP_MULTIPLE_CHOICE = 300,
  HTTP_MOVED_PERMANENTLY = 301,
  HTTP_FOUND = 302,
  HTTP_SEE_OTHER = 303,
  HTTP_NOT_MODIFIED = 304,
  HTTP_TEMPORARY_REDIRECT = 307,
  HTTP_PERMANENT_REDIRECT = 308,

  HTTP_BAD_REQUEST = 400,
  HTTP_UNAUTHORIZED = 401,
  HTTP_PAYMENT_REQUIRED = 402,
  HTTP_FORBIDDEN = 403,
  HTTP_NOT_FOUND = 404,
  HTTP_METHOD_NOT_ALLOWED = 405,
  HTTP_NOT_ACCEPTABLE = 406,
  HTTP_PROXY_AUTHENTICATION_REQUIRED = 407,
  HTTP_REQUEST_TIMEOUT = 408,
  HTTP_CONFLICT = 409,
  HTTP_GONE = 410,
  HTTP_LENGTH_REQUIRED = 411,
  HTTP_PRECONDITION_FAILED = 412,
  HTTP_PAYLOAD_TOO_LARGE = 413,
  HTTP_URI_TOO_LONG = 414,
  HTTP_UNSUPPORTED_MEDIA_TYPE = 415,
  HTTP_RANGE_NOT_STATISFIABLE = 416,
  HTTP_EXPECTATION_FAILED = 417,
  HTTP_UNPROCESSABLE_ENTITY = 422,
  HTTP_TOO_EARLY = 425,
  HTTP_UPGRADE_REQUIRED = 426,
  HTTP_PRECONDITION_REQUIRED = 428,
  HTTP_TOO_MANY_REQUESTS = 429,
  HTTP_REQUEST_HEADER_FIELDS_TOO_LARGE = 431,
  HTTP_UNAVAILABLE_FOR_LEGAL_REASONS = 451,

  HTTP_INTERNAL_SERVER_ERROR = 500,
  HTTP_NOT_IMPLEMENTED = 501,
  HTTP_BAD_GATEWAY = 502,
  HTTP_SERVICE_UNAVAILABLE = 503,
  HTTP_GATEWAY_TIMEOUT = 504,
  HTTP_VERSION_NOT_SUPPORTED = 505,
  HTTP_VARIANT_ALSO_NEGOTIATES = 506,
  HTTP_INSUFFICIENT_STORAGE = 507,
  HTTP_LOOP_DETECTED = 508,
  HTTP_NOT_EXTENDED = 510,
  HTTP_NETWORK_AUTHENTICATION_REQUIRED = 511,
};

std::string getResponseName(HTTPResponseCode);

class HTTPResponse {
public:
  int major = 1, minor = 1;
  HTTPResponseCode code;

  std::vector<HTTPHeader> headers;
  std::string body;

  void setHeader(std::string, std::string);
};

class HTTPHandler {
public:
  virtual void handleRequest(HTTPRequest &, HTTPResponse &) = 0;

  void handleNotFound(HTTPRequest &, HTTPResponse &);
};

class HTTPRouter : public HTTPHandler {
public:
  void handleRequest(HTTPRequest &, HTTPResponse &) override;
  void addRoute(std::string, HTTPHandler *);

private:
  std::map<std::string, HTTPHandler *> handlers;
};

class HTTPServer {
public:
  HTTPServer(const char *, int, int);

  void setHandler(HTTPHandler *);

  void serveForever();
  void serveOnce();

  void shutdown();

private:
  bool setupSocket();
  void closeSocket();

  int acceptConnection();
  void handleClient(int);

  bool readRequest(int, HTTPRequest &);
  void writeResponse(int, HTTPResponse &);

  const char *address;
  int port;
  int maxClients;

  bool running = true;
  int socketDiscriptor;

  HTTPHandler *handler;
};