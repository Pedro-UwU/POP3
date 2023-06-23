/**
 * MIT License - 2023
 * Copyright 2023 - Lopez Guzman, Zahnd
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the “Software”), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is furnished to do
 * so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#ifndef __logger_h_
#define __logger_h_
#include <stdio.h>
#include <stdlib.h>

/* 
*  Macros y funciones simples para log de errores.
*  EL log se hace en forma simple
*  Alternativa: usar syslog para un log mas completo. Ver sección 13.4 del libro de  Stevens
*/

typedef enum { DEBUG = 0, INFO, ERROR, FATAL } LOG_LEVEL;

extern LOG_LEVEL current_level;

/**
*  Minimo nivel de log a registrar. Cualquier llamada a log con un nivel mayor a newLevel sera ignorada
**/
void setLogLevel(LOG_LEVEL newLevel);

char *levelDescription(LOG_LEVEL level);

// Debe ser una macro para poder obtener nombre y linea de archivo.
#define log(level, fmt, ...)                                                              \
        {                                                                                 \
                if (level >= current_level) {                                             \
                        fprintf(stderr, "%s: %s:%d, ", levelDescription(level), __FILE__, \
                                __LINE__);                                                \
                        fprintf(stderr, fmt, ##__VA_ARGS__);                              \
                        fprintf(stderr, "\n");                                            \
                }                                                                         \
                if (level == FATAL)                                                       \
                        exit(1);                                                          \
        }
#endif
