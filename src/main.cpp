#include "basic.h"
#include "http.h"
#include "https.h"
#include "logger.h"

int main(int argc, char *argv[]) {
  HTTPSServer server("RootCA.crt", "RootCA.key", "0.0.0.0", 1337, 16);

  HTTPRouter router;

  BasicHandler handler1;
  BasicHandler handler2;
  handler2.version = 1;

  BasicFileHandler fileHandler;

  router.addRoute("/day", &handler1);
  router.addRoute("/night", &handler2);
  router.addRoute("/static/", &fileHandler);

  server.setHandler(&router);
  server.serveForever();

  return 0;
}