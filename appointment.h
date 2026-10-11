#ifndef APPOINTMENT_H
#define APPOINTMENT_H

// ==============================================================
// MediCare - Appointment Module Header
// BCA 1st Year Project: Demonstrates C Structures & File Handling
// ==============================================================

#define MAX_APPOINTMENTS 300

// Structure representing an Appointment record
typedef struct {
    int id;
    int patientId;
    char patientName[100];
    int doctorId;
    char doctorName[100];
    char date[20];
    char time[20];
    char reason[150];
    char status[20]; // "Scheduled", "Completed", "Cancelled"
} Appointment;

// Function prototypes for appointment operations
int bookAppointment(Appointment a);
int getAllAppointments(Appointment arr[], int max);
int getAppointmentById(int id, Appointment* a);
int updateAppointmentStatus(int id, const char* status);
int deleteAppointment(int id);
int getNextAppointmentId();
void initSampleAppointments();

#endif // APPOINTMENT_H
