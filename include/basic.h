#include "http.h"
#include "logger.h"
#include <string>
#include <sys/stat.h>
#include <unistd.h>

class BasicFileHandler : public HTTPHandler {
public:
  void handleRequest(HTTPRequest &req, HTTPResponse &res) {
    std::string path = "www/" + req.resource;
    FILE *input_file = fopen(path.c_str(), "r");

    if (input_file == nullptr) {
      this->handleNotFound(req, res);
      return;
    }

    struct stat sb {};
    stat(path.c_str(), &sb);

    res.code = HTTP_OK;
    res.body.resize(sb.st_size);
    fread(const_cast<char *>(res.body.data()), sb.st_size, 1, input_file);

    fclose(input_file);
  }
};

class BasicHandler : public HTTPHandler {
public:
  int version = 0;

  void handleRequest(HTTPRequest &req, HTTPResponse &res) override {
    res.code = HTTP_OK;
    std::string msg =
        this->version == 0 ? "<h1>Hello, Day!</h1>" : "<h1>Hello, Night!</h1>";

    res.body = msg + "<p>" + req.resource + "</p>";

    res.setHeader("Content-Type", "text/html");
  }
};