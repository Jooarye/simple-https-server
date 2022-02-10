#include "http.h"
#include <json/forwards.h>
#include <json/json.h>

class JSONRequest {
public:
  HTTPMethod method;
  std::string resource, fullResource;
  int major, minor;

  std::vector<HTTPHeader> headers;
  std::vector<HTTPParam> params;
  Json::Value body;
  bool parsingSuccessfull;

  void fromHTTPRequest(HTTPRequest &);
};

class JSONResponse {
public:
  int major = 1, minor = 1;
  HTTPResponseCode code;

  std::vector<HTTPHeader> headers;
  Json::Value body;

  void setHeader(std::string, std::string);
  void toHTTPReponse(HTTPResponse &);
};

class JSONHandler : public HTTPHandler {
public:
  virtual void handleJsonRequest(JSONRequest &, JSONResponse &) = 0;
  void handleRequest(HTTPRequest &, HTTPResponse &);
};
