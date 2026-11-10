#ifndef UTILS_H
#define UTILS_H

#include "models.h"

/* Screen and visual console UI */
void clearScreen(void);
void pausePrompt(void);
void printHeader(const char* title);
void printDivider(char ch, int length);

/* Safe and validated console user input */
void getString(const char* prompt, char* output, int max_len, bool allow_empty);
int getInt(const char* prompt, int min_val, int max_val);
float getFloat(const char* prompt, float min_val, float max_val);
void trimString(char* str);

/* Time, string formatting and lifecycle stage badges */
void getCurrentTimestamp(char* buffer, int max_len);
const char* getStatusName(RequestStatus status);
const char* getStatusColorTag(RequestStatus status);
void printProgressBar(RequestStatus status);
bool isValidEmailOrPhone(const char* str);

#endif /* UTILS_H */
