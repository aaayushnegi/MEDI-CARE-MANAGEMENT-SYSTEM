#include "prescription.h"
#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ==============================================================
// MediCare - Prescription Module Implementation
// Demonstrates C File Handling: fopen, fclose, fread, fwrite
// ==============================================================

static void getFilePath(char* path, int maxLen) {
    getDataFilePath("prescriptions.dat", path, maxLen);
}

// ==============================================================
// Function: getNextPrescriptionId
// Purpose: Finds next auto-increment ID for prescriptions
// ==============================================================
int getNextPrescriptionId() {
    char path[256];
    getFilePath(path, sizeof(path));

    FILE* fp = fopen(path, "rb");
    if (!fp) return 501; // First prescription starts at ID 501

    Prescription p;
    int maxId = 500;
    while (fread(&p, sizeof(Prescription), 1, fp) == 1) {
        if (p.id > maxId) maxId = p.id;
    }
    fclose(fp);
    return maxId + 1;
}

// ==============================================================
// Function: addPrescription
// Purpose: Saves a new prescription to prescriptions.dat
// ==============================================================
int addPrescription(Prescription p) {
    if (p.id <= 0) {
        p.id = getNextPrescriptionId();
    }

    char path[256];
    getFilePath(path, sizeof(path));

    FILE* fp = fopen(path, "ab");
    if (!fp) return 0;

    size_t written = fwrite(&p, sizeof(Prescription), 1, fp);
    fclose(fp);
    return (written == 1) ? 1 : 0;
}

// ==============================================================
// Function: getAllPrescriptions
// Purpose: Reads all prescriptions from file
// ==============================================================
int getAllPrescriptions(Prescription arr[], int max) {
    char path[256];
    getFilePath(path, sizeof(path));

    FILE* fp = fopen(path, "rb");
    if (!fp) return 0;

    int count = 0;
    while (count < max && fread(&arr[count], sizeof(Prescription), 1, fp) == 1) {
        count++;
    }
    fclose(fp);
    return count;
}

// ==============================================================
// Function: getPrescriptionsByPatient
// Purpose: Returns all prescriptions for a specific patient ID
// ==============================================================
int getPrescriptionsByPatient(int patientId, Prescription arr[], int max) {
    char path[256];
    getFilePath(path, sizeof(path));

    FILE* fp = fopen(path, "rb");
    if (!fp) return 0;

    Prescription temp;
    int count = 0;
    while (count < max && fread(&temp, sizeof(Prescription), 1, fp) == 1) {
        if (temp.patientId == patientId) {
            arr[count++] = temp;
        }
    }
    fclose(fp);
    return count;
}

// ==============================================================
// Function: deletePrescription
// Purpose: Deletes a prescription by ID
// ==============================================================
int deletePrescription(int id) {
    char path[256];
    getFilePath(path, sizeof(path));

    Prescription list[MAX_PRESCRIPTIONS];
    int count = getAllPrescriptions(list, MAX_PRESCRIPTIONS);
    int found = 0;

    for (int i = 0; i < count; i++) {
        if (list[i].id == id) {
            found = 1;
            break;
        }
    }

    if (!found) return 0;

    FILE* fp = fopen(path, "wb");
    if (!fp) return 0;

    for (int i = 0; i < count; i++) {
        if (list[i].id != id) {
            fwrite(&list[i], sizeof(Prescription), 1, fp);
        }
    }
    fclose(fp);
    return 1;
}

// ==============================================================
// Function: initSamplePrescriptions
// Purpose: Populates 5 sample prescriptions if file is empty
// ==============================================================
void initSamplePrescriptions() {
    char path[256];
    getFilePath(path, sizeof(path));

    FILE* fp = fopen(path, "rb");
    if (fp) {
        fseek(fp, 0, SEEK_END);
        long size = ftell(fp);
        fclose(fp);
        if (size > 0) return;
    }

    Prescription samples[5] = {
        {501, 101, 201, "Tab. Amlodipine", "5 mg", "Once Daily (Morning)", "30 Days", "Take with warm water after breakfast. Monitor BP weekly."},
        {502, 102, 201, "Tab. Metformin HCl", "500 mg", "Twice Daily (1-0-1)", "30 Days", "Take strictly after meals to prevent stomach upset."},
        {503, 103, 201, "Levosalbutamol Rotacaps", "100 mcg", "SOS / Twice Daily", "15 Days", "Inhale using rotahaler when breathing difficulty occurs."},
        {504, 104, 205, "Tab. Naproxen", "500 mg", "Once as needed (SOS)", "7 Days", "Take immediately at onset of headache with milk or food."},
        {505, 107, 203, "Tab. Paracetamol & Tramadol", "325mg / 37.5mg", "Twice Daily", "5 Days", "Do not drive after taking medicine. Take with full glass of water."}
    };

    fp = fopen(path, "wb");
    if (fp) {
        for (int i = 0; i < 5; i++) {
            fwrite(&samples[i], sizeof(Prescription), 1, fp);
        }
        fclose(fp);
    }
}
