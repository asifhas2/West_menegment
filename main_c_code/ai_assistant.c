#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include "ai_assistant.h"

#define MODEL_REL          "AI\\llama.cpp\\models\\Qwen3-1.7B-Q4_K_M.gguf"
#define LLAMA_REL_BIN      "AI\\llama.cpp\\build\\bin\\llama-cli.exe"
#define LLAMA_REL_RELEASE  "AI\\llama.cpp\\build\\bin\\Release\\llama-cli.exe"

#define SYSTEM_PROMPT_REL  "AI\\system_prompt.txt"
#define USER_PROMPT_REL    "AI\\user_prompt.txt"
#define RESPONSE_REL       "AI\\response.txt"

#define MAX_INPUT     2048
#define MAX_PATH_LEN  1024
#define MAX_OUTPUT    65536

static char model_path[MAX_PATH_LEN];
static char llama_path[MAX_PATH_LEN];
static char sys_prompt_path[MAX_PATH_LEN];
static char prompt_path[MAX_PATH_LEN];
static char response_path[MAX_PATH_LEN];
static int  has_sys_prompt = 0;

/* ============================================
   1. INITIALIZE FILE PATHS
   ============================================ */
static int initialize_paths(void)
{
    if (!GetFullPathNameA(MODEL_REL, MAX_PATH_LEN, model_path, NULL))
        return 0;

    if (!GetFullPathNameA(SYSTEM_PROMPT_REL, MAX_PATH_LEN, sys_prompt_path, NULL))
        return 0;

    if (!GetFullPathNameA(USER_PROMPT_REL, MAX_PATH_LEN, prompt_path, NULL))
        return 0;

    if (!GetFullPathNameA(RESPONSE_REL, MAX_PATH_LEN, response_path, NULL))
        return 0;

    if (GetFullPathNameA(LLAMA_REL_BIN, MAX_PATH_LEN, llama_path, NULL) &&
        GetFileAttributesA(llama_path) != INVALID_FILE_ATTRIBUTES) {
        /* Found in build\bin\ */
    } else if (GetFullPathNameA(LLAMA_REL_RELEASE, MAX_PATH_LEN, llama_path, NULL) &&
               GetFileAttributesA(llama_path) != INVALID_FILE_ATTRIBUTES) {
        /* Found in build\bin\Release\ */
    } else {
        printf("[ERROR] llama-cli.exe not found in bin\\ or bin\\Release\\\n");
        return 0;
    }

    if (GetFileAttributesA(model_path) == INVALID_FILE_ATTRIBUTES) {
        printf("[ERROR] Model file not found: %s\n", model_path);
        return 0;
    }

    has_sys_prompt = (GetFileAttributesA(sys_prompt_path) != INVALID_FILE_ATTRIBUTES);
    return 1;
}

/* ============================================
   2. CREATE PROMPT (Plain question + /no_think)
   ============================================ */
static int create_prompt(const char *question)
{
    FILE *fp = fopen(prompt_path, "w");
    if (fp == NULL) {
        printf("[ERROR] Could not create prompt file: %s\n", prompt_path);
        return 0;
    }

    /* No manual ChatML tags; llama-cli applies Qwen3 template automatically */
    fprintf(fp, "%s /no_think\n", question);
    fclose(fp);
    return 1;
}

/* ============================================
   3. RUN QWEN MODEL
   ============================================ */
static int ask_model(void)
{
    STARTUPINFOA si;
    PROCESS_INFORMATION pi;
    SECURITY_ATTRIBUTES sa;
    HANDLE input_file;
    HANDLE output_file;

    char command[4096];
    char working_dir[MAX_PATH_LEN];

    memset(&si, 0, sizeof(si));
    memset(&pi, 0, sizeof(pi));
    memset(&sa, 0, sizeof(sa));

    sa.nLength = sizeof(sa);
    sa.bInheritHandle = TRUE;

    /* Redirect stdin to NUL so llama-cli never waits for keyboard input */
    input_file = CreateFileA(
        "NUL",
        GENERIC_READ,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        &sa,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );

    if (input_file == INVALID_HANDLE_VALUE) {
        printf("\n[ERROR] Could not open NUL for stdin.\n");
        return 0;
    }

    output_file = CreateFileA(
        response_path,
        GENERIC_WRITE,
        FILE_SHARE_READ,
        &sa,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );

    if (output_file == INVALID_HANDLE_VALUE) {
        CloseHandle(input_file);
        printf("\n[ERROR] Could not create response file.\n");
        return 0;
    }

    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESTDHANDLES;
    si.hStdInput  = input_file;
    si.hStdOutput = output_file;
    si.hStdError  = output_file;

    if (has_sys_prompt) {
        snprintf(command, sizeof(command),
            "\"%s\" "
            "-m \"%s\" "
            "-sysf \"%s\" "
            "-f \"%s\" "
            "-c 2048 "
            "-n 256 "
            "-t 4 "
            "--single-turn "
            "--simple-io "
            "--temp 0.2",
            llama_path,
            model_path,
            sys_prompt_path,
            prompt_path
        );
    } else {
        snprintf(command, sizeof(command),
            "\"%s\" "
            "-m \"%s\" "
            "-f \"%s\" "
            "-c 2048 "
            "-n 256 "
            "-t 4 "
            "--single-turn "
            "--simple-io "
            "--temp 0.2",
            llama_path,
            model_path,
            prompt_path
        );
    }

    GetCurrentDirectoryA(sizeof(working_dir), working_dir);

    BOOL success = CreateProcessA(
        llama_path,
        command,
        NULL,
        NULL,
        TRUE,
        CREATE_NO_WINDOW,
        NULL,
        working_dir,
        &si,
        &pi
    );

    CloseHandle(input_file);
    CloseHandle(output_file);

    if (!success) {
        printf("\n[ERROR] Unable to start Qwen. Windows error: %lu\n", GetLastError());
        return 0;
    }

    WaitForSingleObject(pi.hProcess, INFINITE);

    DWORD exit_code = 0;
    GetExitCodeProcess(pi.hProcess, &exit_code);

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    if (exit_code != 0) {
        printf("\n[ERROR] Model execution failed (Exit code: %lu).\n", exit_code);
        printf("Check %s for details.\n", response_path);
        return 0;
    }

    return 1;
}

/* ============================================
   4. CLEAN AND DISPLAY MODEL RESPONSE
   ============================================ */
static void display_response(void)
{
    FILE *fp = fopen(response_path, "rb");
    if (fp == NULL) {
        printf("[ERROR] Unable to read model response.\n");
        return;
    }

    char *buffer = (char *)malloc(MAX_OUTPUT);
    if (buffer == NULL) {
        fclose(fp);
        printf("[ERROR] Memory allocation failed.\n");
        return;
    }

    size_t length = fread(buffer, 1, MAX_OUTPUT - 1, fp);
    buffer[length] = '\0';
    fclose(fp);

    /* Skip llama-cli ASCII banner and echoed "> question" line */
    char *after_cmds = strstr(buffer, "available commands:");
    char *prompt_line = (after_cmds != NULL)
                        ? strstr(after_cmds, "\n> ")
                        : strstr(buffer, "\n> ");

    if (prompt_line != NULL) {
        char *reply_start = strchr(prompt_line + 3, '\n');
        if (reply_start != NULL) {
            memmove(buffer, reply_start + 1, strlen(reply_start + 1) + 1);
        }
    }

    /* Strip Qwen3 thinking blocks if present */
    char *think_end = strstr(buffer, "[End thinking]");
    if (think_end != NULL) {
        think_end += strlen("[End thinking]");
        memmove(buffer, think_end, strlen(think_end) + 1);
    }

    char *xml_think_end = strstr(buffer, "</think>");
    if (xml_think_end != NULL) {
        xml_think_end += strlen("</think>");
        memmove(buffer, xml_think_end, strlen(xml_think_end) + 1);
    }

    /* Cut off llama.cpp timings and exit text */
    char *end;
    if ((end = strstr(buffer, "[ Prompt:")) != NULL)     *end = '\0';
    if ((end = strstr(buffer, "Exiting...")) != NULL)    *end = '\0';
    if ((end = strstr(buffer, "<|im_end|>")) != NULL)    *end = '\0';
    if ((end = strstr(buffer, "<|endoftext|>")) != NULL) *end = '\0';

    /* Trim leading whitespace */
    char *clean = buffer;
    while (*clean == '\n' || *clean == '\r' || *clean == ' ' || *clean == '\t') {
        clean++;
    }

    /* Trim trailing whitespace */
    size_t n = strlen(clean);
    while (n > 0 && (clean[n - 1] == '\n' || clean[n - 1] == '\r' || clean[n - 1] == ' ')) {
        clean[--n] = '\0';
    }

    if (strlen(clean) > 0) {
        printf("\nEcoBot: %s\n", clean);
    } else {
        printf("\nEcoBot: No response generated.\n");
    }

    free(buffer);
}

/* ============================================
   5. MAIN AI ASSISTANT MODULE
   ============================================ */
void handleAiAssistantModule(void)
{
    char question[MAX_INPUT];

    printf("\n====================================\n");
    printf("     ECOBOT AI WASTE ASSISTANT      \n");
    printf("====================================\n");
    printf("Type 'exit' to return to the menu.\n");

    if (!initialize_paths()) {
        printf("\nAI Assistant initialization failed.\n");
        return;
    }

    printf("\nEcoBot is ready!\n");

    while (1) {
        printf("\nYou: ");
        fflush(stdout);

        if (fgets(question, sizeof(question), stdin) == NULL)
            break;

        question[strcspn(question, "\r\n")] = '\0';

        if (strlen(question) == 0)
            continue;

        if (strcmp(question, "exit") == 0)
            break;

        if (!create_prompt(question))
            continue;

        printf("EcoBot is thinking...");
        fflush(stdout);

        if (ask_model()) {
            printf("\r                     \r");
            display_response();
        }
    }

    printf("\nChat session ended.\n");
}