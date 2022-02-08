#include "http.h"

class BasicFileHandler : public HTTPHandler {
public:
  void handleRequest(HTTPRequest &, HTTPResponse &);
};

class BasicHandler : public HTTPHandler {
public:
  int version = 0;

  void handleRequest(HTTPRequest &, HTTPResponse &);
};