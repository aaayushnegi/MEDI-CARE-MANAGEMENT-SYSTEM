#ifndef PATIENT_H
#define PATIENT_H

// ==============================================================
// MediCare - Patient Module Header
// BCA 1st Year Project: Demonstrates C Structures & File Handling
// ==============================================================

#define MAX_PATIENTS 200

// Structure representing a Patient record
typedef struct {
    int id;
    char name[100];
    int age;
    char gender[15];
    char phone[20];
    char address[150];
    char bloodGroup[10];
    char disease[100];
    char emergencyContact[20];
} Patient;

// Function prototypes for patient operations
int addPatient(Patient p);
int getAllPatients(Patient arr[], int max);
int getPatientById(int id, Patient* p);
int updatePatient(Patient p);
int deletePatient(int id);
int getNextPatientId();
void initSamplePatients();

#endif // PATIENT_H
