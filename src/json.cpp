#include "json.h"
#include "http.h"
#include <json/reader.h>
#include <json/writer.h>

void JSONRequest::fromHTTPRequest(HTTPRequest &req) {
  Json::Reader reader;

  this->minor = req.minor;
  this->major = req.major;
  this->method = req.method;
  this->params = req.params;
  this->headers = req.headers;
  this->resource = req.resource;
  this->parsingSuccessfull = reader.parse(req.body, this->body);
}

void JSONResponse::setHeader(std::string name, std::string value) {
  this->headers.push_back({name, value});
}

void JSONResponse::toHTTPReponse(HTTPResponse &res) {
  Json::FastWriter writer;

  res.minor = this->minor;
  res.major = this->major;
  res.code = this->code;
  res.headers = this->headers;
  res.body = writer.write(this->body);

  res.setHeader("Content-Type", "application/json");
}

void JSONHandler::handleRequest(HTTPRequest &req, HTTPResponse &res) {
  JSONRequest jreq;
  JSONResponse jres;

  jreq.fromHTTPRequest(req);
  this->handleJsonRequest(jreq, jres);
  jres.toHTTPReponse(res);
}