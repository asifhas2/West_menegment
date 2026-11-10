#include "utils.h"

void clearScreen(void) {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

void pausePrompt(void) {
    printf("\nPress [ENTER] to continue...");
    int c;
    while ((c = getchar()) != '\n' && c != EOF) {}
}

void printDivider(char ch, int length) {
    for (int i = 0; i < length; i++) {
        putchar(ch);
    }
    putchar('\n');
}

void printHeader(const char* title) {
    printDivider('=', 70);
    printf("   %s\n", title);
    printDivider('=', 70);
}

void trimString(char* str) {
    if (!str) return;
    /* Trim trailing spaces, tabs, newlines and carriage returns */
    int len = (int)strlen(str);
    while (len > 0 && (str[len - 1] == ' ' || str[len - 1] == '\t' || str[len - 1] == '\n' || str[len - 1] == '\r')) {
        str[len - 1] = '\0';
        len--;
    }
    /* Trim leading whitespace */
    int start = 0;
    while (str[start] == ' ' || str[start] == '\t') {
        start++;
    }
    if (start > 0) {
        memmove(str, str + start, strlen(str + start) + 1);
    }
}

void getString(const char* prompt, char* output, int max_len, bool allow_empty) {
    char buffer[512];
    while (1) {
        if (prompt && strlen(prompt) > 0) {
            printf("%s", prompt);
        }
        if (!fgets(buffer, sizeof(buffer), stdin)) {
            buffer[0] = '\0';
            output[0] = '\0';
            return;
        }
        trimString(buffer);

        if (!allow_empty && strlen(buffer) == 0) {
            printf("[!] Input cannot be empty. Please enter a valid value.\n");
            continue;
        }

        if ((int)strlen(buffer) >= max_len) {
            buffer[max_len - 1] = '\0';
        }
        strncpy(output, buffer, max_len);
        output[max_len - 1] = '\0';
        break;
    }
}

int getInt(const char* prompt, int min_val, int max_val) {
    char buffer[64];
    int val;
    while (1) {
        printf("%s", prompt);
        if (!fgets(buffer, sizeof(buffer), stdin)) {
            return min_val;
        }
        trimString(buffer);
        if (strlen(buffer) == 0) {
            printf("[!] Please enter a number.\n");
            continue;
        }

        char* endptr;
        val = (int)strtol(buffer, &endptr, 10);
        if (*endptr != '\0') {
            printf("[!] Invalid input. Please enter an integer between %d and %d.\n", min_val, max_val);
            continue;
        }

        if (val < min_val || val > max_val) {
            printf("[!] Value must be between %d and %d. Please try again.\n", min_val, max_val);
            continue;
        }
        return val;
    }
}

float getFloat(const char* prompt, float min_val, float max_val) {
    char buffer[64];
    float val;
    while (1) {
        printf("%s", prompt);
        if (!fgets(buffer, sizeof(buffer), stdin)) {
            return min_val;
        }
        trimString(buffer);
        if (strlen(buffer) == 0) {
            printf("[!] Please enter a valid number.\n");
            continue;
        }

        char* endptr;
        val = strtof(buffer, &endptr);
        if (*endptr != '\0') {
            printf("[!] Invalid input. Please enter a valid numerical value.\n");
            continue;
        }

        if (val < min_val || val > max_val) {
            printf("[!] Value must be between %.2f and %.2f kg.\n", min_val, max_val);
            continue;
        }
        return val;
    }
}

void getCurrentTimestamp(char* buffer, int max_len) {
    time_t now = time(NULL);
    struct tm* t = localtime(&now);
    if (t) {
        snprintf(buffer, max_len, "%04d-%02d-%02d %02d:%02d",
                 t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
                 t->tm_hour, t->tm_min);
    } else {
        snprintf(buffer, max_len, "2026-01-01 10:00");
    }
}

const char* getStatusName(RequestStatus status) {
    switch (status) {
        case REQ_STATUS_PENDING:       return "Pending";
        case REQ_STATUS_APPROVED:      return "Approved";
        case REQ_STATUS_ASSIGNED:      return "Assigned";
        case REQ_STATUS_IN_PROGRESS:   return "In Progress";
        case REQ_STATUS_COLLECTED:     return "Collected";
        case REQ_STATUS_FINAL_OUTCOME: return "Final Outcome Recorded";
        default:                       return "Unknown";
    }
}

const char* getStatusColorTag(RequestStatus status) {
    switch (status) {
        case REQ_STATUS_PENDING:       return "[PENDING]";
        case REQ_STATUS_APPROVED:      return "[APPROVED]";
        case REQ_STATUS_ASSIGNED:      return "[ASSIGNED]";
        case REQ_STATUS_IN_PROGRESS:   return "[IN PROGRESS]";
        case REQ_STATUS_COLLECTED:     return "[COLLECTED]";
        case REQ_STATUS_FINAL_OUTCOME: return "[OUTCOME RECORDED]";
        default:                       return "[UNKNOWN]";
    }
}

void printProgressBar(RequestStatus status) {
    const char* steps[] = {
        "1. Pending",
        "2. Approved",
        "3. Assigned",
        "4. In Progress",
        "5. Collected",
        "6. Final Outcome"
    };

    printf(" Lifecycle Progress:\n ");
    for (int i = 0; i < 6; i++) {
        if (i <= (int)status) {
            printf("[%s]", steps[i]);
        } else {
            printf("(%s)", steps[i]);
        }
        if (i < 5) printf(" -> ");
    }
    printf("\n");
}

bool isValidEmailOrPhone(const char* str) {
    if (!str || strlen(str) < 3) return false;
    bool has_digit = false;
    bool has_alpha = false;
    bool has_at = false;

    for (int i = 0; str[i]; i++) {
        if (isdigit((unsigned char)str[i])) has_digit = true;
        if (isalpha((unsigned char)str[i])) has_alpha = true;
        if (str[i] == '@') has_at = true;
    }

    /* Valid email requires '@' and alphanumeric characters */
    if (has_at && has_alpha) return true;
    /* Valid phone requires digits and no '@' symbol */
    if (has_digit && !has_at) return true;

    return false;
}
