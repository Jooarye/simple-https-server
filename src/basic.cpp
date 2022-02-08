#include "basic.h"
#include "http.h"
#include <dirent.h>
#include <string>
#include <sys/stat.h>
#include <unistd.h>

void BasicFileHandler::handleRequest(HTTPRequest &req, HTTPResponse &res) {
  std::string path = "www/" + req.resource;
  FILE *input_file = fopen(path.c_str(), "r");

  if (input_file == nullptr) {
    this->handleNotFound(req, res);
    return;
  }

  struct stat sb {};
  stat(path.c_str(), &sb);

  if (S_ISREG(sb.st_mode)) {
    res.code = HTTP_OK;
    res.body.resize(sb.st_size);
    fread(const_cast<char *>(res.body.data()), sb.st_size, 1, input_file);

    fclose(input_file);
  } else {
    DIR *dir;
    struct dirent *result;

    if ((dir = opendir(path.c_str())) == nullptr) {
      HTTPHandler::handleNotFound(req, res);
      return;
    }

    res.body += "<ul>";
    while ((result = readdir(dir)) != nullptr) {
      if (result->d_name[0] != '.') {
        res.body += "<li>";
        res.body += result->d_name;
        res.body += "</li>";
      }
    }
    res.body += "</ul>";

    res.code = HTTP_OK;
    res.setHeader("Content-Type", "text/html; charset=UTF-8");
  }
}

void BasicHandler::handleRequest(HTTPRequest &req, HTTPResponse &res) {
  res.code = HTTP_OK;
  std::string msg =
      this->version == 0 ? "<h1>Hello, Day!</h1>" : "<h1>Hello, Night!</h1>";

  res.body = msg + "<p>" + req.resource + "</p>";

  res.setHeader("Content-Type", "text/html");
}