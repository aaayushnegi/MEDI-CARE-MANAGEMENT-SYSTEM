#include "patient.h"
#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ==============================================================
// MediCare - Patient Module Implementation
// Demonstrates C File Handling: fopen, fclose, fread, fwrite
// ==============================================================

// Helper to get patients file path
static void getFilePath(char* path, int maxLen) {
    getDataFilePath("patients.dat", path, maxLen);
}

// ==============================================================
// Function: getNextPatientId
// Purpose: Scans the file to find the highest ID, returns highest + 1
// ==============================================================
int getNextPatientId() {
    char path[256];
    getFilePath(path, sizeof(path));

    // fopen: "rb" opens file in read binary mode
    FILE* fp = fopen(path, "rb");
    if (!fp) {
        return 101; // First patient starts at ID 101
    }

    Patient p;
    int maxId = 100;
    // fread: reads 1 Patient structure at a time
    while (fread(&p, sizeof(Patient), 1, fp) == 1) {
        if (p.id > maxId) {
            maxId = p.id;
        }
    }
    // fclose: closes file stream to free operating system resources
    fclose(fp);
    return maxId + 1;
}

// ==============================================================
// Function: addPatient
// Purpose: Appends a new patient record to patients.dat
// Returns: 1 on success, 0 on failure
// ==============================================================
int addPatient(Patient p) {
    char path[256];
    getFilePath(path, sizeof(path));

    // Auto-assign ID if not set
    if (p.id <= 0) {
        p.id = getNextPatientId();
    }

    // fopen: "ab" opens file in append binary mode
    FILE* fp = fopen(path, "ab");
    if (!fp) {
        printf("Error: Could not open file %s for appending.\n", path);
        return 0;
    }

    // fwrite: writes the Patient structure to disk
    size_t written = fwrite(&p, sizeof(Patient), 1, fp);
    fclose(fp);

    return (written == 1) ? 1 : 0;
}

// ==============================================================
// Function: getAllPatients
// Purpose: Reads all patient records from patients.dat into array
// Returns: Number of patients read
// ==============================================================
int getAllPatients(Patient arr[], int max) {
    char path[256];
    getFilePath(path, sizeof(path));

    FILE* fp = fopen(path, "rb");
    if (!fp) {
        return 0;
    }

    int count = 0;
    while (count < max && fread(&arr[count], sizeof(Patient), 1, fp) == 1) {
        count++;
    }

    fclose(fp);
    return count;
}

// ==============================================================
// Function: getPatientById
// Purpose: Finds a patient by their unique ID
// Returns: 1 if found, 0 if not found
// ==============================================================
int getPatientById(int id, Patient* p) {
    char path[256];
    getFilePath(path, sizeof(path));

    FILE* fp = fopen(path, "rb");
    if (!fp) return 0;

    Patient temp;
    while (fread(&temp, sizeof(Patient), 1, fp) == 1) {
        if (temp.id == id) {
            *p = temp;
            fclose(fp);
            return 1;
        }
    }

    fclose(fp);
    return 0;
}

// ==============================================================
// Function: updatePatient
// Purpose: Updates an existing patient record matching p.id
// Returns: 1 on success, 0 on failure
// ==============================================================
int updatePatient(Patient p) {
    char path[256];
    getFilePath(path, sizeof(path));

    Patient list[MAX_PATIENTS];
    int count = getAllPatients(list, MAX_PATIENTS);
    int found = 0;

    for (int i = 0; i < count; i++) {
        if (list[i].id == p.id) {
            list[i] = p; // update record in memory
            found = 1;
            break;
        }
    }

    if (!found) return 0;

    // fopen: "wb" truncates file and writes updated records
    FILE* fp = fopen(path, "wb");
    if (!fp) return 0;

    for (int i = 0; i < count; i++) {
        fwrite(&list[i], sizeof(Patient), 1, fp);
    }

    fclose(fp);
    return 1;
}

// ==============================================================
// Function: deletePatient
// Purpose: Deletes a patient record matching id
// Returns: 1 on success, 0 on failure
// ==============================================================
int deletePatient(int id) {
    char path[256];
    getFilePath(path, sizeof(path));

    Patient list[MAX_PATIENTS];
    int count = getAllPatients(list, MAX_PATIENTS);
    int found = 0;

    // Check if ID exists
    for (int i = 0; i < count; i++) {
        if (list[i].id == id) {
            found = 1;
            break;
        }
    }

    if (!found) return 0;

    FILE* fp = fopen(path, "wb");
    if (!fp) return 0;

    // Write all records except the one being deleted
    for (int i = 0; i < count; i++) {
        if (list[i].id != id) {
            fwrite(&list[i], sizeof(Patient), 1, fp);
        }
    }

    fclose(fp);
    return 1;
}

// ==============================================================
// Function: initSamplePatients
// Purpose: Populates 10 fictional patients if file is empty
// ==============================================================
void initSamplePatients() {
    char path[256];
    getFilePath(path, sizeof(path));

    FILE* fp = fopen(path, "rb");
    if (fp) {
        fseek(fp, 0, SEEK_END);
        long size = ftell(fp);
        fclose(fp);
        if (size > 0) return; // Already populated
    }

    Patient samples[10] = {
        {101, "Rahul Sharma", 32, "Male", "9876543210", "45 Park Street, Mumbai", "B+", "Hypertension", "9876543211"},
        {102, "Priya Patel", 28, "Female", "9823456789", "12 Ring Road, Delhi", "O+", "Type 2 Diabetes", "9823456780"},
        {103, "Amit Verma", 45, "Male", "9712345678", "88 MG Road, Bengaluru", "A+", "Asthma", "9712345670"},
        {104, "Sneha Kulkarni", 24, "Female", "9934567890", "23 Deccan Gymkhana, Pune", "AB+", "Migraine", "9934567899"},
        {105, "Vikram Malhotra", 52, "Male", "9845612345", "101 Jubilee Hills, Hyderabad", "O-", "Chest Pain", "9845612340"},
        {106, "Ananya Roy", 19, "Female", "9871234560", "14 Salt Lake, Kolkata", "B-", "Viral Fever", "9871234561"},
        {107, "Rajesh Nair", 61, "Male", "9765432109", "5 Boat Jetty, Kochi", "A-", "Arthritis", "9765432100"},
        {108, "Pooja Mehta", 35, "Female", "9812378945", "77 Civil Lines, Jaipur", "B+", "Hypothyroidism", "9812378940"},
        {109, "Karan Singh", 29, "Male", "9898765432", "19 Sector 17, Chandigarh", "O+", "Skin Rash / Eczema", "9898765430"},
        {110, "Meera Joshi", 42, "Female", "9745612389", "62 Alkapuri, Vadodara", "AB-", "Lumbar Disc Pain", "9745612380"}
    };

    fp = fopen(path, "wb");
    if (fp) {
        for (int i = 0; i < 10; i++) {
            fwrite(&samples[i], sizeof(Patient), 1, fp);
        }
        fclose(fp);
    }
}
