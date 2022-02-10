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

  int state = PARSING_BEGIN;
  while (begin != end) {
    char input = *begin++;

    switch (state) {
    case PARSING_BEGIN:
      if (!isChar(input) || isControl(input) || isSpecial(input)) {
        return false;
      } else {
        state = PARSING_METHOD;
        method.push_back(input);
      }
      break;
    case PARSING_METHOD:
      if (input == ' ') {
        req.method = getMethod(method);
        if (req.method == HTTP_NONE)
          return true;
        state = PARSING_RESOURCE_BEGIN;
      } else if (!isChar(input) || isControl(input) || isSpecial(input)) {
        return false;
      } else {
        method.push_back(input);
      }
      break;
    case PARSING_RESOURCE_BEGIN:
      if (isControl(input)) {
        return false;
      } else {
        state = PARSING_RESOURCE;
        req.resource.push_back(input);
      }
      break;
    case PARSING_RESOURCE:
      if (input == ' ') {
        state = PARSING_VERSION_H;
      } else if (input == '?') {
        state = PARSING_PARAMS_BEGIN;
      } else if (input == '\r') {
        req.major = 0;
        req.minor = 9;

        return true;
      } else if (isControl(input)) {
        return false;
      } else {
        req.resource.push_back(input);
      }
      break;
    case PARSING_PARAMS_BEGIN:
      if (isControl(input)) {
        return false;
      } else if (input == ' ') {
        state = PARSING_VERSION_H;
      } else {
        req.params.push_back(HTTPParam());
        req.params.back().name.reserve(16);
        req.params.back().value.reserve(16);
        req.params.back().name.push_back(input);
        state = PARSING_PARAMS_NAME;
      }
      break;
    case PARSING_PARAMS_NAME:
      if (input == '=') {
        state = PARSING_PARAMS_VALUE;
      } else if (input == '&') {
        state = PARSING_PARAMS_BEGIN;
      } else if (input == ' ') {
        state = PARSING_VERSION_H;
      } else if (isControl(input)) {
        return false;
      } else {
        req.params.back().name.push_back(input);
      }
      break;
    case PARSING_PARAMS_VALUE:
      if (input == '&') {
        state = PARSING_PARAMS_BEGIN;
      } else if (input == ' ') {
        state = PARSING_VERSION_H;
      } else if (isControl(input)) {
        return false;
      } else {
        req.params.back().value.push_back(input);
      }
      break;
    case PARSING_VERSION_H:
      if (input == 'H') {
        state = PARSING_VERSION_HT;
      } else {
        return false;
      }
      break;
    case PARSING_VERSION_HT:
      if (input == 'T') {
        state = PARSING_VERSION_HTT;
      } else {
        return false;
      }
      break;
    case PARSING_VERSION_HTT:
      if (input == 'T') {
        state = PARSING_VERSION_HTTP;
      } else {
        return false;
      }
      break;
    case PARSING_VERSION_HTTP:
      if (input == 'P') {
        state = PARSING_VERSION_HTTPS;
      } else {
        return false;
      }
      break;
    case PARSING_VERSION_HTTPS:
      if (input == '/') {
        req.major = 0;
        req.minor = 0;
        state = PARSING_VERSION_MAJOR;
      } else {
        return false;
      }
      break;
    case PARSING_VERSION_MAJOR:
      if (isDigit(input)) {
        req.major = input - '0';
        state = PARSING_VERSION_MAJOR_LONG;
      } else {
        return false;
      }
      break;
    case PARSING_VERSION_MAJOR_LONG:
      if (input == '.') {
        state = PARSING_VERSION_MINOR;
      } else if (isDigit(input)) {
        req.major = req.major * 10 + input - '0';
      } else {
        return false;
      }
      break;
    case PARSING_VERSION_MINOR:
      if (isDigit(input)) {
        req.minor = input - '0';
        state = PARSING_VERSION_MINOR_LONG;
      } else {
        return false;
      }
      break;
    case PARSING_VERSION_MINOR_LONG:
      if (input == '\r') {
        state = PARSING_NEW_LINE;
      } else if (isDigit(input)) {
        req.minor = req.minor * 10 + input - '0';
      } else {
        return false;
      }
      break;
    case PARSING_NEW_LINE:
      if (input == '\n') {
        state = PARSING_HEADER_BEGIN;
      } else {
        return false;
      }
      break;
    case PARSING_HEADER_BEGIN:
      if (input == '\r') {
        state = PARSING_BODY_BEGIN;
      } else if (!req.headers.empty() && (input == ' ' || input == '\t')) {
        state = PARSING_HEADER_SKIP;
      } else if (!isChar(input) || isControl(input) || isSpecial(input)) {
        return false;
      } else {
        req.headers.push_back(HTTPHeader());
        req.headers.back().name.reserve(16);
        req.headers.back().value.reserve(16);
        req.headers.back().name.push_back(input);
        state = PARSING_HEADER_NAME;
      }
      break;
    case PARSING_HEADER_SKIP:
      if (input == '\r') {
        state = PARSING_HEADER_END;
      } else if (input == ' ' || input == '\t') {
      } else if (isControl(input)) {
        return false;
      } else {
        state = 21;
        req.headers.back().value.push_back(input);
      }
      break;
    case PARSING_HEADER_NAME:
      if (input == ':') {
        state = PARSING_HEADER_SEPERATOR;
      } else if (!isChar(input) || isControl(input) || isSpecial(input)) {
        return false;
      } else {
        req.headers.back().name.push_back(input);
      }
      break;
    case PARSING_HEADER_SEPERATOR:
      if (input == ' ') {
        state = PARSING_HEADER_VALUE;
      } else {
        return false;
      }
      break;
    case PARSING_HEADER_VALUE:
      if (input == '\r') {
        state = PARSING_HEADER_END;
      } else if (isControl(input)) {
        return false;
      } else {
        req.headers.back().value.push_back(input);
      }
      break;
    case PARSING_HEADER_END:
      if (input == '\n') {
        state = PARSING_HEADER_BEGIN;
      } else {
        return false;
      }
      break;
    case PARSING_BODY_BEGIN: {
      state = PARSING_BODY;
      break;
    }
    case PARSING_BODY:
      req.body.push_back(input);
      break;
    default:
      return false;
    }
  }

  req.fullResource = req.resource;

  for (HTTPParam &p : req.params) {
    p.name = decodeURIComponent(p.name);
    p.value = decodeURIComponent(p.value);
  }

  return true;
}

std::string decodeURIComponent(std::string s) {
  char *src = (char *)s.c_str();
  std::string result;

  while (*src) {
    if (*src == '%' && src[1] && src[2] && isxdigit(src[1]) &&
        isxdigit(src[2])) {
      src[1] -= src[1] <= '9' ? '0' : (src[1] <= 'F' ? 'A' : 'a') - 10;
      src[2] -= src[2] <= '9' ? '0' : (src[2] <= 'F' ? 'A' : 'a') - 10;
      result.push_back(16 * src[1] + src[2]);
      src += 3;
      continue;
    }
    result.push_back(*src++);
  }

  return result;
}