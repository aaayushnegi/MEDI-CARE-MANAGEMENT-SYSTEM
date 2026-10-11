#include "records.h"
#include "patient.h"
#include "doctor.h"
#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ==============================================================
// MediCare - Medical Records Module Implementation
// Demonstrates C File Handling: fopen, fclose, fread, fwrite
// ==============================================================

static void getFilePath(char* path, int maxLen) {
    getDataFilePath("medical_records.dat", path, maxLen);
}

// ==============================================================
// Function: getNextRecordId
// Purpose: Finds next auto-increment ID for medical records
// ==============================================================
int getNextRecordId() {
    char path[256];
    getFilePath(path, sizeof(path));

    FILE* fp = fopen(path, "rb");
    if (!fp) return 401; // First record starts at ID 401

    MedicalRecord r;
    int maxId = 400;
    while (fread(&r, sizeof(MedicalRecord), 1, fp) == 1) {
        if (r.id > maxId) maxId = r.id;
    }
    fclose(fp);
    return maxId + 1;
}

// ==============================================================
// Function: addMedicalRecord
// Purpose: Saves a new medical record to medical_records.dat
// ==============================================================
int addMedicalRecord(MedicalRecord r) {
    if (r.id <= 0) {
        r.id = getNextRecordId();
    }

    char path[256];
    getFilePath(path, sizeof(path));

    FILE* fp = fopen(path, "ab");
    if (!fp) return 0;

    size_t written = fwrite(&r, sizeof(MedicalRecord), 1, fp);
    fclose(fp);
    return (written == 1) ? 1 : 0;
}

// ==============================================================
// Function: getAllMedicalRecords
// Purpose: Reads all medical records from file
// ==============================================================
int getAllMedicalRecords(MedicalRecord arr[], int max) {
    char path[256];
    getFilePath(path, sizeof(path));

    FILE* fp = fopen(path, "rb");
    if (!fp) return 0;

    int count = 0;
    while (count < max && fread(&arr[count], sizeof(MedicalRecord), 1, fp) == 1) {
        count++;
    }
    fclose(fp);
    return count;
}

// ==============================================================
// Function: getMedicalRecordsByPatient
// Purpose: Filters medical records for a specific patient ID
// ==============================================================
int getMedicalRecordsByPatient(int patientId, MedicalRecord arr[], int max) {
    char path[256];
    getFilePath(path, sizeof(path));

    FILE* fp = fopen(path, "rb");
    if (!fp) return 0;

    MedicalRecord temp;
    int count = 0;
    while (count < max && fread(&temp, sizeof(MedicalRecord), 1, fp) == 1) {
        if (temp.patientId == patientId) {
            arr[count++] = temp;
        }
    }
    fclose(fp);
    return count;
}

// ==============================================================
// Function: initSampleRecords
// Purpose: Populates 5 sample medical records if file is empty
// ==============================================================
void initSampleRecords() {
    char path[256];
    getFilePath(path, sizeof(path));

    FILE* fp = fopen(path, "rb");
    if (fp) {
        fseek(fp, 0, SEEK_END);
        long size = ftell(fp);
        fclose(fp);
        if (size > 0) return;
    }

    MedicalRecord samples[5] = {
        {401, 101, 201, "Stage 1 Essential Hypertension", "Headache, dizziness, high BP 145/95", "Amlodipine 5mg once daily", "2026-10-01", "Advised low-sodium diet and 30 mins brisk walking daily."},
        {402, 102, 201, "Type 2 Diabetes Mellitus", "Increased thirst, frequent urination, fatigue", "Metformin 500mg twice daily after meals", "2026-10-01", "HbA1c test scheduled for next week. Avoid refined sugars."},
        {403, 103, 201, "Bronchial Asthma (Moderate)", "Shortness of breath, wheezing during night", "Salbutamol Inhaler 100mcg as needed", "2026-10-02", "Keep inhaler handy at all times. Avoid dust triggers."},
        {404, 104, 205, "Migraine without Aura", "Throbbing unilateral headache, light sensitivity", "Naproxen 500mg SOS, Propranolol 20mg daily", "2026-10-03", "Maintain sleep log and avoid prolonged computer screen exposure."},
        {405, 107, 203, "Osteoarthritis Right Knee", "Joint stiffness in morning, swelling, pain while walking", "Paracetamol 650mg SOS, Glucosamine supplement", "2026-10-03", "Physiotherapy exercises recommended 3 times a week."}
    };

    fp = fopen(path, "wb");
    if (fp) {
        for (int i = 0; i < 5; i++) {
            fwrite(&samples[i], sizeof(MedicalRecord), 1, fp);
        }
        fclose(fp);
    }
}
