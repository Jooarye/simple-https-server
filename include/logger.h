#pragma once

class Logger {
public:
  void log(const char *, ...);

private:
  char *getClientHash(int);
};

extern Logger LOGGER;