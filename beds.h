#ifndef BEDS_H
#define BEDS_H

// ==============================================================
// MediCare - Bed Management Module Header
// BCA 1st Year Project: Demonstrates C Structures & File Handling
// ==============================================================

#define MAX_BEDS 50

// Structure representing a Hospital Bed
typedef struct {
    int id;
    char ward[50];
    char bedNumber[20];
    int patientId; // 0 if unassigned / available
    char status[20]; // "Available", "Occupied"
} Bed;

// Function prototypes for bed operations
int getAllBeds(Bed arr[], int max);
int assignBed(int bedId, int patientId);
int releaseBed(int bedId);
int addBed(Bed b);
int getNextBedId();
void initSampleBeds();

#endif // BEDS_H
