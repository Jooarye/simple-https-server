#pragma once
#include <openssl/err.h>
#include <openssl/ssl.h>

#include "http.h"

class HTTPSServer {
public:
  HTTPSServer(const char *, const char *, const char *, int, int);

  void setHandler(HTTPHandler *);

  void serveForever();
  void serveOnce();

  void shutdown();

private:
  bool setupSocket();
  bool setupSSLContext();

  void closeSSLContext();
  void closeSocket();

  SSL *acceptConnection();
  void handleClient(SSL *);

  bool readRequest(SSL *, HTTPRequest &);
  void writeResponse(SSL *, HTTPResponse &);

  const char *certificateFile;
  const char *keyFile;

  SSL_CTX *sslContext;

  const char *address;
  int port;
  int maxClients;

  bool running = true;
  int socketDiscriptor;

  HTTPHandler *handler;
};