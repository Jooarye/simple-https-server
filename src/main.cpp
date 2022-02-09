#include "basic.h"
#include "http.h"
#include "https.h"
#include "json.h"
#include "logger.h"
#include <json/forwards.h>

class APIHandler : public JSONHandler {
public:
  void handleJsonRequest(JSONRequest &req, JSONResponse &res) {
    res.body["first"] = "Bob";
    res.body["last"] = "Maier";
    res.body["age"] = 22;
    res.body["city"] = "Berlin";
    res.body["occupation"] = "Computer Sience Student";

    for (HTTPParam &param : req.params) {
      res.body["test"][param.name] = param.value;
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

  router.addRoute("/day", &handler1);
  router.addRoute("/night", &handler2);
  router.addRoute("/static/", &fileHandler);
  router.addRoute("/api", &apiHandler);

  server.setHandler(&router);
  server.serveForever();

  return 0;
}