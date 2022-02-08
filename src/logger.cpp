#include "logger.h"
#include <cstdarg>
#include <stdio.h>
#include <stdlib.h>
#include <string>

Logger LOGGER;

void Logger::log(const char *format, ...) {
  va_list args;
  va_start(args, format);

  while (*format) {
    if (*format == '%') {
      format++;
      switch (*format) {
      case '%':
        putchar('%');
        break;
      case 'r':
        fputs("\u001b[31m", stdout);
        break;
      case 'g':
        fputs("\u001b[32m", stdout);
        break;
      case 'y':
        fputs("\u001b[33m", stdout);
        break;
      case 'b':
        fputs("\u001b[34m", stdout);
        break;
      case 'm':
        fputs("\u001b[35m", stdout);
        break;
      case 'c':
        fputs("\u001b[36m", stdout);
        break;
      case 'w':
        fputs("\u001b[37m", stdout);
        break;
      case 'n':
        fputs("\u001b[0m", stdout);
        break;
      case 'i': {
        int num = va_arg(args, int);

        if (num > 199 && num < 300)
          fputs("\u001b[32m", stdout);
        else if (num > 299 && num < 400)
          fputs("\u001b[33m", stdout);
        else if (num < 200)
          fputs("\u001b[34m", stdout);
        else
          fputs("\u001b[31m", stdout);

        fputs(std::to_string(num).c_str(), stdout);
        break;
      }
      case 'a': {
        int client = (va_arg(args, int) ^ 0x6621bb4ffefda690);

        fputs("\u001b[3", stdout);
        putchar("23456"[(client & 0xff) % 5]);
        putchar('m');

        for (int i = 0; i < 4; i++) {
          putchar("0123456789abcdef"[client & 0xf]);
          client >>= 4;
        }

        break;
      }
      case 'd': {
        int num = va_arg(args, int);
        fputs(std::to_string(num).c_str(), stdout);
        break;
      }
      case 's':
        char *str = va_arg(args, char *);
        fputs(str, stdout);
        break;
      }
    } else {
      putchar(*format);
    }

    format++;
  }

  va_end(args);
}