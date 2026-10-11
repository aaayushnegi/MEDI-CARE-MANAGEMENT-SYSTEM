#ifndef RECORDS_H
#define RECORDS_H

// ==============================================================
// MediCare - Medical Records Module Header
// BCA 1st Year Project: Demonstrates C Structures & File Handling
// ==============================================================

#define MAX_RECORDS 200

// Structure representing a Medical Record
typedef struct {
    int id;
    int patientId;
    int doctorId;
    char diagnosis[150];
    char symptoms[200];
    char prescription[200];
    char date[20];
    char notes[250];
} MedicalRecord;

// Function prototypes for medical records operations
int addMedicalRecord(MedicalRecord r);
int getAllMedicalRecords(MedicalRecord arr[], int max);
int getMedicalRecordsByPatient(int patientId, MedicalRecord arr[], int max);
int getNextRecordId();
void initSampleRecords();

#endif // RECORDS_H
