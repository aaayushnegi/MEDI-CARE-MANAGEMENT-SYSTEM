#include "appointment.h"
#include "patient.h"
#include "doctor.h"
#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ==============================================================
// MediCare - Appointment Module Implementation
// Demonstrates C File Handling: fopen, fclose, fread, fwrite
// ==============================================================

static void getFilePath(char* path, int maxLen) {
    getDataFilePath("appointments.dat", path, maxLen);
}

// ==============================================================
// Function: getNextAppointmentId
// Purpose: Finds next auto-increment ID for appointments
// ==============================================================
int getNextAppointmentId() {
    char path[256];
    getFilePath(path, sizeof(path));

    FILE* fp = fopen(path, "rb");
    if (!fp) return 301; // First appointment starts at ID 301

    Appointment a;
    int maxId = 300;
    while (fread(&a, sizeof(Appointment), 1, fp) == 1) {
        if (a.id > maxId) maxId = a.id;
    }
    fclose(fp);
    return maxId + 1;
}

// ==============================================================
// Function: bookAppointment
// Purpose: Validates patient & doctor and stores appointment
// Returns: 1 on success, 0 on validation/file failure
// ==============================================================
int bookAppointment(Appointment a) {
    // Validate Patient ID
    Patient p;
    if (!getPatientById(a.patientId, &p)) {
        printf("Validation Error: Patient ID %d does not exist.\n", a.patientId);
        return 0;
    }
    // Copy patient name if empty
    if (strlen(a.patientName) == 0) {
        strncpy(a.patientName, p.name, sizeof(a.patientName) - 1);
    }

    // Validate Doctor ID
    Doctor d;
    if (!getDoctorById(a.doctorId, &d)) {
        printf("Validation Error: Doctor ID %d does not exist.\n", a.doctorId);
        return 0;
    }
    // Copy doctor name if empty
    if (strlen(a.doctorName) == 0) {
        strncpy(a.doctorName, d.name, sizeof(a.doctorName) - 1);
    }

    // Assign default status if empty
    if (strlen(a.status) == 0) {
        strcpy(a.status, "Scheduled");
    }

    if (a.id <= 0) {
        a.id = getNextAppointmentId();
    }

    char path[256];
    getFilePath(path, sizeof(path));

    FILE* fp = fopen(path, "ab");
    if (!fp) return 0;

    size_t written = fwrite(&a, sizeof(Appointment), 1, fp);
    fclose(fp);
    return (written == 1) ? 1 : 0;
}

// ==============================================================
// Function: getAllAppointments
// Purpose: Reads all appointments from appointments.dat
// ==============================================================
int getAllAppointments(Appointment arr[], int max) {
    char path[256];
    getFilePath(path, sizeof(path));

    FILE* fp = fopen(path, "rb");
    if (!fp) return 0;

    int count = 0;
    while (count < max && fread(&arr[count], sizeof(Appointment), 1, fp) == 1) {
        count++;
    }
    fclose(fp);
    return count;
}

// ==============================================================
// Function: getAppointmentById
// Purpose: Finds appointment by ID
// ==============================================================
int getAppointmentById(int id, Appointment* a) {
    char path[256];
    getFilePath(path, sizeof(path));

    FILE* fp = fopen(path, "rb");
    if (!fp) return 0;

    Appointment temp;
    while (fread(&temp, sizeof(Appointment), 1, fp) == 1) {
        if (temp.id == id) {
            *a = temp;
            fclose(fp);
            return 1;
        }
    }
    fclose(fp);
    return 0;
}

// ==============================================================
// Function: updateAppointmentStatus
// Purpose: Updates appointment status (e.g. Completed, Cancelled)
// ==============================================================
int updateAppointmentStatus(int id, const char* status) {
    char path[256];
    getFilePath(path, sizeof(path));

    Appointment list[MAX_APPOINTMENTS];
    int count = getAllAppointments(list, MAX_APPOINTMENTS);
    int found = 0;

    for (int i = 0; i < count; i++) {
        if (list[i].id == id) {
            strncpy(list[i].status, status, sizeof(list[i].status) - 1);
            list[i].status[sizeof(list[i].status) - 1] = '\0';
            found = 1;
            break;
        }
    }

    if (!found) return 0;

    FILE* fp = fopen(path, "wb");
    if (!fp) return 0;

    for (int i = 0; i < count; i++) {
        fwrite(&list[i], sizeof(Appointment), 1, fp);
    }
    fclose(fp);
    return 1;
}

// ==============================================================
// Function: deleteAppointment
// Purpose: Deletes appointment by ID
// ==============================================================
int deleteAppointment(int id) {
    char path[256];
    getFilePath(path, sizeof(path));

    Appointment list[MAX_APPOINTMENTS];
    int count = getAllAppointments(list, MAX_APPOINTMENTS);
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
            fwrite(&list[i], sizeof(Appointment), 1, fp);
        }
    }
    fclose(fp);
    return 1;
}

// ==============================================================
// Function: initSampleAppointments
// Purpose: Populates 8 sample appointments if file is empty
// ==============================================================
void initSampleAppointments() {
    char path[256];
    getFilePath(path, sizeof(path));

    FILE* fp = fopen(path, "rb");
    if (fp) {
        fseek(fp, 0, SEEK_END);
        long size = ftell(fp);
        fclose(fp);
        if (size > 0) return;
    }

    Appointment samples[8] = {
        {301, 101, "Rahul Sharma", 201, "Dr. Arvind Rao", "2026-10-04", "09:30 AM", "Routine Blood Pressure Checkup", "Scheduled"},
        {302, 102, "Priya Patel", 201, "Dr. Arvind Rao", "2026-10-04", "10:30 AM", "Diabetes Fasting Review", "Scheduled"},
        {303, 105, "Vikram Malhotra", 202, "Dr. Shalini Saxena", "2026-10-04", "11:00 AM", "Chest Pain & ECG Evaluation", "Scheduled"},
        {304, 107, "Rajesh Nair", 203, "Dr. Ramesh Iyer", "2026-10-05", "11:30 AM", "Knee Joint Arthritis Pain", "Scheduled"},
        {305, 109, "Karan Singh", 204, "Dr. Fatima Sheikh", "2026-10-05", "02:30 PM", "Allergic Skin Rash Checkup", "Scheduled"},
        {306, 110, "Meera Joshi", 205, "Dr. Neeraj Khanna", "2026-10-06", "01:30 PM", "Lower Spine Radiating Pain", "Scheduled"},
        {307, 103, "Amit Verma", 201, "Dr. Arvind Rao", "2026-10-02", "10:00 AM", "Asthma Inhaler Renewal", "Completed"},
        {308, 104, "Sneha Kulkarni", 205, "Dr. Neeraj Khanna", "2026-10-03", "02:00 PM", "Severe Migraine Consultation", "Completed"}
    };

    fp = fopen(path, "wb");
    if (fp) {
        for (int i = 0; i < 8; i++) {
            fwrite(&samples[i], sizeof(Appointment), 1, fp);
        }
        fclose(fp);
    }
}
