#include "beds.h"
#include "patient.h"
#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ==============================================================
// MediCare - Bed Management Module Implementation
// Demonstrates C File Handling: fopen, fclose, fread, fwrite
// ==============================================================

static void getFilePath(char* path, int maxLen) {
    getDataFilePath("beds.dat", path, maxLen);
}

// ==============================================================
// Function: getNextBedId
// Purpose: Finds next auto-increment ID for beds
// ==============================================================
int getNextBedId() {
    char path[256];
    getFilePath(path, sizeof(path));

    FILE* fp = fopen(path, "rb");
    if (!fp) return 701; // First bed starts at ID 701

    Bed b;
    int maxId = 700;
    while (fread(&b, sizeof(Bed), 1, fp) == 1) {
        if (b.id > maxId) maxId = b.id;
    }
    fclose(fp);
    return maxId + 1;
}

// ==============================================================
// Function: addBed
// Purpose: Adds a new bed to the hospital
// ==============================================================
int addBed(Bed b) {
    if (b.id <= 0) {
        b.id = getNextBedId();
    }
    if (strlen(b.status) == 0) {
        strcpy(b.status, "Available");
    }

    char path[256];
    getFilePath(path, sizeof(path));

    FILE* fp = fopen(path, "ab");
    if (!fp) return 0;

    size_t written = fwrite(&b, sizeof(Bed), 1, fp);
    fclose(fp);
    return (written == 1) ? 1 : 0;
}

// ==============================================================
// Function: getAllBeds
// Purpose: Reads all bed records from beds.dat
// ==============================================================
int getAllBeds(Bed arr[], int max) {
    char path[256];
    getFilePath(path, sizeof(path));

    FILE* fp = fopen(path, "rb");
    if (!fp) return 0;

    int count = 0;
    while (count < max && fread(&arr[count], sizeof(Bed), 1, fp) == 1) {
        count++;
    }
    fclose(fp);
    return count;
}

// ==============================================================
// Function: assignBed
// Purpose: Assigns an available bed to a patient. Prevents re-assignment.
// Returns: 1 on success, 0 on failure (e.g. invalid patient or already occupied)
// ==============================================================
int assignBed(int bedId, int patientId) {
    // Validate patient exists
    Patient p;
    if (!getPatientById(patientId, &p)) {
        printf("Validation Error: Patient ID %d does not exist.\n", patientId);
        return 0;
    }

    char path[256];
    getFilePath(path, sizeof(path));

    Bed list[MAX_BEDS];
    int count = getAllBeds(list, MAX_BEDS);
    int found = 0;

    for (int i = 0; i < count; i++) {
        if (list[i].id == bedId) {
            // Check if already occupied
            if (strcmp(list[i].status, "Occupied") == 0) {
                printf("Validation Error: Bed %d is already occupied.\n", bedId);
                return 0;
            }
            strcpy(list[i].status, "Occupied");
            list[i].patientId = patientId;
            found = 1;
            break;
        }
    }

    if (!found) return 0;

    FILE* fp = fopen(path, "wb");
    if (!fp) return 0;

    for (int i = 0; i < count; i++) {
        fwrite(&list[i], sizeof(Bed), 1, fp);
    }
    fclose(fp);
    return 1;
}

// ==============================================================
// Function: releaseBed
// Purpose: Marks an occupied bed as Available and clears patientId
// ==============================================================
int releaseBed(int bedId) {
    char path[256];
    getFilePath(path, sizeof(path));

    Bed list[MAX_BEDS];
    int count = getAllBeds(list, MAX_BEDS);
    int found = 0;

    for (int i = 0; i < count; i++) {
        if (list[i].id == bedId) {
            strcpy(list[i].status, "Available");
            list[i].patientId = 0;
            found = 1;
            break;
        }
    }

    if (!found) return 0;

    FILE* fp = fopen(path, "wb");
    if (!fp) return 0;

    for (int i = 0; i < count; i++) {
        fwrite(&list[i], sizeof(Bed), 1, fp);
    }
    fclose(fp);
    return 1;
}

// ==============================================================
// Function: initSampleBeds
// Purpose: Populates 10 sample hospital beds if file is empty
// ==============================================================
void initSampleBeds() {
    char path[256];
    getFilePath(path, sizeof(path));

    FILE* fp = fopen(path, "rb");
    if (fp) {
        fseek(fp, 0, SEEK_END);
        long size = ftell(fp);
        fclose(fp);
        if (size > 0) return;
    }

    Bed samples[10] = {
        {701, "General Ward A", "GW-101", 101, "Occupied"},
        {702, "General Ward A", "GW-102", 102, "Occupied"},
        {703, "General Ward A", "GW-103", 0,   "Available"},
        {704, "General Ward B", "GW-201", 105, "Occupied"},
        {705, "General Ward B", "GW-202", 0,   "Available"},
        {706, "ICU Ward",        "ICU-01", 0,   "Available"},
        {707, "ICU Ward",        "ICU-02", 107, "Occupied"},
        {708, "Emergency Care",  "EMG-01", 0,   "Available"},
        {709, "Emergency Care",  "EMG-02", 0,   "Available"},
        {710, "Semi-Private",    "SP-301", 0,   "Available"}
    };

    fp = fopen(path, "wb");
    if (fp) {
        for (int i = 0; i < 10; i++) {
            fwrite(&samples[i], sizeof(Bed), 1, fp);
        }
        fclose(fp);
    }
}
