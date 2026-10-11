#ifndef PRESCRIPTION_H
#define PRESCRIPTION_H

// ==============================================================
// MediCare - Prescription Module Header
// BCA 1st Year Project: Demonstrates C Structures & File Handling
// ==============================================================

#define MAX_PRESCRIPTIONS 200

// Structure representing a Prescription
typedef struct {
    int id;
    int patientId;
    int doctorId;
    char medicineName[100];
    char dosage[50];
    char frequency[50];
    char duration[50];
    char instructions[200];
} Prescription;

// Function prototypes for prescription operations
int addPrescription(Prescription p);
int getAllPrescriptions(Prescription arr[], int max);
int getPrescriptionsByPatient(int patientId, Prescription arr[], int max);
int deletePrescription(int id);
int getNextPrescriptionId();
void initSamplePrescriptions();

#endif // PRESCRIPTION_H
