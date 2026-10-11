#ifndef COMMON_H
#define COMMON_H

// ==============================================================
// MediCare - Common Utilities Header
// Helper functions for file path resolution and JSON processing
// ==============================================================

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Resolve file path to data folder (e.g. data/patients.dat or ../data/patients.dat)
void getDataFilePath(const char* filename, char* outPath, int maxLen);

// JSON helper functions for parsing incoming requests and escaping outgoing strings
void escape_json(const char* input, char* output, int maxLen);
int get_json_string(const char* json, const char* key, char* out, int maxLen);
int get_json_int(const char* json, const char* key, int* out);
int get_json_float(const char* json, const char* key, float* out);

#endif // COMMON_H
