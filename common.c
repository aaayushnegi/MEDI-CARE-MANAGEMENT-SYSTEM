#include "common.h"
#include <direct.h> // for _mkdir on Windows

// ==============================================================
// Function: getDataFilePath
// Purpose: Finds the proper path for .dat files in the data directory
// ==============================================================
void getDataFilePath(const char* filename, char* outPath, int maxLen) {
    // Check if data/ directory is directly accessible
    FILE* fp = fopen("data/check.tmp", "w");
    if (fp != NULL) {
        fclose(fp);
        remove("data/check.tmp");
        snprintf(outPath, maxLen, "data/%s", filename);
        return;
    }

    // Check if ../data/ directory is accessible (e.g. running from backend/)
    fp = fopen("../data/check.tmp", "w");
    if (fp != NULL) {
        fclose(fp);
        remove("../data/check.tmp");
        snprintf(outPath, maxLen, "../data/%s", filename);
        return;
    }

    // If neither exists, attempt to create ./data/ directory
    _mkdir("data");
    snprintf(outPath, maxLen, "data/%s", filename);
}

// ==============================================================
// Function: escape_json
// Purpose: Escapes quotes, backslashes, and newlines for JSON safety
// ==============================================================
void escape_json(const char* input, char* output, int maxLen) {
    if (!input || !output || maxLen <= 0) return;
    int j = 0;
    for (int i = 0; input[i] != '\0' && j < maxLen - 2; i++) {
        if (input[i] == '"') {
            if (j < maxLen - 3) { output[j++] = '\\'; output[j++] = '"'; }
        } else if (input[i] == '\\') {
            if (j < maxLen - 3) { output[j++] = '\\'; output[j++] = '\\'; }
        } else if (input[i] == '\n') {
            if (j < maxLen - 3) { output[j++] = '\\'; output[j++] = 'n'; }
        } else if (input[i] == '\r') {
            // skip carriage return
        } else if (input[i] == '\t') {
            if (j < maxLen - 3) { output[j++] = '\\'; output[j++] = 't'; }
        } else {
            output[j++] = input[i];
        }
    }
    output[j] = '\0';
}

// ==============================================================
// Function: get_json_string
// Purpose: Extracts string value for a given key from JSON string
// Example: searching for "name" in {"name":"Aayush Sharma"}
// ==============================================================
int get_json_string(const char* json, const char* key, char* out, int maxLen) {
    if (!json || !key || !out || maxLen <= 0) return 0;
    out[0] = '\0';

    char searchPattern[128];
    snprintf(searchPattern, sizeof(searchPattern), "\"%s\"", key);
    const char* pos = strstr(json, searchPattern);
    if (!pos) return 0;

    pos += strlen(searchPattern);
    // Skip spaces and colon
    while (*pos == ' ' || *pos == ':' || *pos == '\t' || *pos == '\r' || *pos == '\n') pos++;

    if (*pos == '\"') {
        pos++; // Skip opening quote
        int i = 0;
        while (*pos != '\0' && *pos != '\"' && i < maxLen - 1) {
            if (*pos == '\\' && *(pos + 1) != '\0') {
                pos++;
                if (*pos == 'n') out[i++] = '\n';
                else if (*pos == 't') out[i++] = '\t';
                else out[i++] = *pos;
            } else {
                out[i++] = *pos;
            }
            pos++;
        }
        out[i] = '\0';
        return 1;
    }
    return 0;
}

// ==============================================================
// Function: get_json_int
// Purpose: Extracts integer value for a given key from JSON string
// ==============================================================
int get_json_int(const char* json, const char* key, int* out) {
    if (!json || !key || !out) return 0;
    char searchPattern[128];
    snprintf(searchPattern, sizeof(searchPattern), "\"%s\"", key);
    const char* pos = strstr(json, searchPattern);
    if (!pos) return 0;

    pos += strlen(searchPattern);
    while (*pos == ' ' || *pos == ':' || *pos == '\t' || *pos == '\r' || *pos == '\n') pos++;

    // Could be quoted "123" or unquoted 123
    if (*pos == '\"') pos++;

    *out = atoi(pos);
    return 1;
}

// ==============================================================
// Function: get_json_float
// Purpose: Extracts float value for a given key from JSON string
// ==============================================================
int get_json_float(const char* json, const char* key, float* out) {
    if (!json || !key || !out) return 0;
    char searchPattern[128];
    snprintf(searchPattern, sizeof(searchPattern), "\"%s\"", key);
    const char* pos = strstr(json, searchPattern);
    if (!pos) return 0;

    pos += strlen(searchPattern);
    while (*pos == ' ' || *pos == ':' || *pos == '\t' || *pos == '\r' || *pos == '\n') pos++;

    if (*pos == '\"') pos++;

    *out = (float)atof(pos);
    return 1;
}
