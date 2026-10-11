#ifndef DOCTOR_H
#define DOCTOR_H

// ==============================================================
// MediCare - Doctor Module Header
// BCA 1st Year Project: Demonstrates C Structures & File Handling
// ==============================================================

#define MAX_DOCTORS 100

// Structure representing a Doctor record
typedef struct {
    int id;
    char name[100];
    char specialization[100];
    char phone[20];
    char email[100];
    char availableDays[100];
    char availableTime[50];
} Doctor;

// Function prototypes for doctor operations
int addDoctor(Doctor d);
int getAllDoctors(Doctor arr[], int max);
int getDoctorById(int id, Doctor* d);
int updateDoctor(Doctor d);
int deleteDoctor(int id);
int getNextDoctorId();
void initSampleDoctors();

#endif // DOCTOR_H
