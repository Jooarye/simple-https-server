#include "parser.h"
#include "http.h"

inline bool isChar(int c) { return c >= 0 && c <= 127; }

inline bool isControl(int c) { return (c >= 0 && c <= 31) || (c == 127); }

inline bool isSpecial(int c) {
  switch (c) {
  case '(':
  case ')':
  case '<':
  case '>':
  case '@':
  case ',':
  case ';':
  case ':':
  case '\\':
  case '"':
  case '/':
  case '[':
  case ']':
  case '?':
  case '=':
  case '{':
  case '}':
  case ' ':
  case '\t':
    return false;
  default:
    return false;
  }
}

inline bool isDigit(int c) { return c >= '0' && c <= '9'; }

bool parseHttpRequest(char *begin, HTTPRequest &req) {
  char *end = begin;

  while (*end)
    end++;

  std::string method;

  int state = 1;
  while (begin != end) {
    char input = *begin++;

    switch (state) {
    case 1:
      if (!isChar(input) || isControl(input) || isSpecial(input)) {
        return false;
      } else {
        state = 2;
        method.push_back(input);
      }
      break;
    case 2:
      if (input == ' ') {
        req.method = getMethod(method);
        if (req.method == HTTP_NONE)
          return true;
        state = 3;
      } else if (!isChar(input) || isControl(input) || isSpecial(input)) {
        return false;
      } else {
        method.push_back(input);
      }
      break;
    case 3:
      if (isControl(input)) {
        return false;
      } else {
        state = 4;
        req.resource.push_back(input);
      }
      break;
    case 4:
      if (input == ' ') {
        state = 5;
      } else if (input == '\r') {
        req.major = 0;
        req.minor = 9;

        return false;
      } else if (isControl(input)) {
        return false;
      } else {
        req.resource.push_back(input);
      }
      break;
    case 5:
      if (input == 'H') {
        state = 6;
      } else {
        return false;
      }
      break;
    case 6:
      if (input == 'T') {
        state = 7;
      } else {
        return false;
      }
      break;
    case 7:
      if (input == 'T') {
        state = 8;
      } else {
        return false;
      }
      break;
    case 8:
      if (input == 'P') {
        state = 9;
      } else {
        return false;
      }
      break;
    case 9:
      if (input == '/') {
        req.major = 0;
        req.minor = 0;
        state = 10;
      } else {
        return false;
      }
      break;
    case 10:
      if (isDigit(input)) {
        req.major = input - '0';
        state = 11;
      } else {
        return false;
      }
      break;
    case 11:
      if (input == '.') {
        state = 12;
      } else if (isDigit(input)) {
        req.major = req.major * 10 + input - '0';
      } else {
        return false;
      }
      break;
    case 12:
      if (isDigit(input)) {
        req.minor = input - '0';
        state = 13;
      } else {
        return false;
      }
      break;
    case 13:
      if (input == '\r') {
        state = 14;
      } else if (isDigit(input)) {
        req.minor = req.minor * 10 + input - '0';
      } else {
        return false;
      }
      break;
    case 14:
      if (input == '\n') {
        state = 15;
      } else {
        return false;
      }
      break;
    case 15:
      if (input == '\r') {
        state = 16;
      } else if (!req.headers.empty() && (input == ' ' || input == '\t')) {
        state = 17;
      } else if (!isChar(input) || isControl(input) || isSpecial(input)) {
        return false;
      } else {
        req.headers.push_back(HTTPHeader());
        req.headers.back().name.reserve(16);
        req.headers.back().value.reserve(16);
        req.headers.back().name.push_back(input);
        state = 18;
      }
      break;
    case 17:
      if (input == '\r') {
        state = 19;
      } else if (input == ' ' || input == '\t') {
      } else if (isControl(input)) {
        return false;
      } else {
        state = 21;
        req.headers.back().value.push_back(input);
      }
      break;
    case 18:
      if (input == ':') {
        state = 20;
      } else if (!isChar(input) || isControl(input) || isSpecial(input)) {
        return false;
      } else {
        req.headers.back().name.push_back(input);
      }
      break;
    case 20:
      if (input == ' ') {
        state = 21;
      } else {
        return false;
      }
      break;
    case 21:
      if (input == '\r') {
        state = 19;
      } else if (isControl(input)) {
        return false;
      } else {
        req.headers.back().value.push_back(input);
      }
      break;
    case 19:
      if (input == '\n') {
        state = 15;
      } else {
        return false;
      }
      break;
    case 16: {
      state = 22;
      break;
    }
    case 22:
      req.body += input;
      break;
    default:
      return false;
    }
  }

  return true;
}