/** @file */

#include <stdarg.h>

/**
*   @brief enum contains HTML-COLORS
*
*   @param YELLOW       - "Gold"
*   @param RED          - "DarkRed"
*   @param GREEN        - "LimeGreen"
*   @param BLUE         - "MediumBlue"
*   @param POISON_COLOR - "Olive"
*   @param USUAL        - ""
*/

enum COLOR
{
    YELLOW,
    RED,
    GREEN,
    BLUE,
    POISON_COLOR,
    USUAL
};

const char *COLOR_NAMES[] = 
{
    "Gold",
    "DarkRed",
    "LimeGreen",
    "MediumBlue",
    "Olive",
    ""
};

const char *LOG_FILE_NAME = "../LOGS/log.html";
FILE       *LOG_STREAM    = nullptr;

/**
*   @brief Closes log-file. Called by using atexit().
*
*   @return 1 if closing is OK. Does abort() if an ERROR found.
*/

void CLOSE_LOG_STREAM()
{
    assert(LOG_STREAM != nullptr);

    fprintf(LOG_STREAM, "\"%s\" CLOSING IS OK\n\n", LOG_FILE_NAME);
    fclose( LOG_STREAM);
}

/**
*   @brief Opens log-file. Ckecks if opening is OK and in this case prints message in the log-file.
*   @brief Uses atexit() to call CLOSE_LOG_STREAM() after program end.
*
*   @return 1 if checking is OK. Does abort() if an ERROR found.
*/

int OPEN_LOG_STREAM()
{
    LOG_STREAM = fopen(LOG_FILE_NAME, "w");

    assert(LOG_STREAM != nullptr);

    setvbuf(LOG_STREAM,   nullptr, _IONBF, 0);

    fprintf(LOG_STREAM, "<pre>\n""\"%s\" OPENING IS OK\n\n", LOG_FILE_NAME);

    atexit(CLOSE_LOG_STREAM);
    return 1;
}

int  _OPEN_CLOSE_LOG_STREAM = OPEN_LOG_STREAM();

void log_message(COLOR col, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);

    fprintf (LOG_STREAM, "<font color=%s>", COLOR_NAMES[col]);
    vfprintf(LOG_STREAM, fmt, ap);
    fprintf (LOG_STREAM, "</font>");
}

#define log_func_end(err)                                           \
        _log_func_end(__PRETTY_FUNCTION__, err);

void _log_func_end(const char *function_name, unsigned err)
{
    log_message(USUAL, "%s returns %d\n\n", function_name, err);
}
