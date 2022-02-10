#include "basic.h"
#include "http.h"
#include "https.h"
#include "json.h"
#include "logger.h"
#include <json/forwards.h>

class APIHandler : public JSONHandler {
public:
  void handleJsonRequest(JSONRequest &req, JSONResponse &res) {
    res.body["method"] = getMethodName(req.method);
    res.body["resource"]["endpoint"] = req.resource;
    res.body["resource"]["full"] = req.fullResource;
    res.body["version"]["major"] = req.major;
    res.body["version"]["minor"] = req.minor;

    for (HTTPParam &param : req.params) {
      res.body["req"]["params"][param.name] = param.value;
    }

    for (HTTPHeader &header : req.headers) {
      res.body["headers"][header.name] = header.value;
    }

    if (req.method == HTTP_POST) {
      res.body["req"]["body"] = req.body;
    }

    res.code = HTTP_OK;
  }
};

int main(int argc, char *argv[]) {
  HTTPSServer server("RootCA.crt", "RootCA.key", "127.0.0.1", 1337, 16);

  HTTPRouter router;

  BasicHandler handler1;
  BasicHandler handler2;
  handler2.version = 1;

  BasicFileHandler fileHandler;
  APIHandler apiHandler;

  HTTPRouter second;

  second.addRoute("/v1", &apiHandler);

  router.addRoute("/day", &handler1);
  router.addRoute("/night", &handler2);
  router.addRoute("/static/", &fileHandler);
  router.addRoute("/api/", &second);

  server.setHandler(&router);
  server.serveForever();

  return 0;
}