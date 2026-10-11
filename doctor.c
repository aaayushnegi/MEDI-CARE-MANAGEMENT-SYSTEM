#include "doctor.h"
#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ==============================================================
// MediCare - Doctor Module Implementation
// Demonstrates C File Handling: fopen, fclose, fread, fwrite
// ==============================================================

static void getFilePath(char* path, int maxLen) {
    getDataFilePath("doctors.dat", path, maxLen);
}

// ==============================================================
// Function: getNextDoctorId
// Purpose: Finds next auto-increment ID for doctors
// ==============================================================
int getNextDoctorId() {
    char path[256];
    getFilePath(path, sizeof(path));

    FILE* fp = fopen(path, "rb");
    if (!fp) return 201; // First doctor starts at ID 201

    Doctor d;
    int maxId = 200;
    while (fread(&d, sizeof(Doctor), 1, fp) == 1) {
        if (d.id > maxId) maxId = d.id;
    }
    fclose(fp);
    return maxId + 1;
}

// ==============================================================
// Function: addDoctor
// Purpose: Appends new doctor record to doctors.dat
// ==============================================================
int addDoctor(Doctor d) {
    char path[256];
    getFilePath(path, sizeof(path));

    if (d.id <= 0) {
        d.id = getNextDoctorId();
    }

    FILE* fp = fopen(path, "ab");
    if (!fp) return 0;

    size_t written = fwrite(&d, sizeof(Doctor), 1, fp);
    fclose(fp);
    return (written == 1) ? 1 : 0;
}

// ==============================================================
// Function: getAllDoctors
// Purpose: Reads all doctor records from doctors.dat
// ==============================================================
int getAllDoctors(Doctor arr[], int max) {
    char path[256];
    getFilePath(path, sizeof(path));

    FILE* fp = fopen(path, "rb");
    if (!fp) return 0;

    int count = 0;
    while (count < max && fread(&arr[count], sizeof(Doctor), 1, fp) == 1) {
        count++;
    }
    fclose(fp);
    return count;
}

// ==============================================================
// Function: getDoctorById
// Purpose: Finds doctor by ID
// ==============================================================
int getDoctorById(int id, Doctor* d) {
    char path[256];
    getFilePath(path, sizeof(path));

    FILE* fp = fopen(path, "rb");
    if (!fp) return 0;

    Doctor temp;
    while (fread(&temp, sizeof(Doctor), 1, fp) == 1) {
        if (temp.id == id) {
            *d = temp;
            fclose(fp);
            return 1;
        }
    }
    fclose(fp);
    return 0;
}

// ==============================================================
// Function: updateDoctor
// Purpose: Updates an existing doctor record
// ==============================================================
int updateDoctor(Doctor d) {
    char path[256];
    getFilePath(path, sizeof(path));

    Doctor list[MAX_DOCTORS];
    int count = getAllDoctors(list, MAX_DOCTORS);
    int found = 0;

    for (int i = 0; i < count; i++) {
        if (list[i].id == d.id) {
            list[i] = d;
            found = 1;
            break;
        }
    }

    if (!found) return 0;

    FILE* fp = fopen(path, "wb");
    if (!fp) return 0;

    for (int i = 0; i < count; i++) {
        fwrite(&list[i], sizeof(Doctor), 1, fp);
    }
    fclose(fp);
    return 1;
}

// ==============================================================
// Function: deleteDoctor
// Purpose: Deletes a doctor by ID
// ==============================================================
int deleteDoctor(int id) {
    char path[256];
    getFilePath(path, sizeof(path));

    Doctor list[MAX_DOCTORS];
    int count = getAllDoctors(list, MAX_DOCTORS);
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
            fwrite(&list[i], sizeof(Doctor), 1, fp);
        }
    }
    fclose(fp);
    return 1;
}

// ==============================================================
// Function: initSampleDoctors
// Purpose: Initializes 5 sample doctors if file is empty
// ==============================================================
void initSampleDoctors() {
    char path[256];
    getFilePath(path, sizeof(path));

    FILE* fp = fopen(path, "rb");
    if (fp) {
        fseek(fp, 0, SEEK_END);
        long size = ftell(fp);
        fclose(fp);
        if (size > 0) return;
    }

    Doctor samples[5] = {
        {201, "Dr. Arvind Rao", "General Physician", "9811122233", "arvind.rao@medicare.org", "Mon - Sat", "09:00 AM - 01:00 PM"},
        {202, "Dr. Shalini Saxena", "Cardiologist", "9822233344", "shalini.s@medicare.org", "Mon, Wed, Fri", "10:00 AM - 03:00 PM"},
        {203, "Dr. Ramesh Iyer", "Orthopedic", "9833344455", "ramesh.i@medicare.org", "Tue, Thu, Sat", "11:00 AM - 04:00 PM"},
        {204, "Dr. Fatima Sheikh", "Dermatologist", "9844455566", "fatima.s@medicare.org", "Mon - Fri", "02:00 PM - 06:00 PM"},
        {205, "Dr. Neeraj Khanna", "Neurologist", "9855566677", "neeraj.k@medicare.org", "Tue, Wed, Sat", "01:00 PM - 05:00 PM"}
    };

    fp = fopen(path, "wb");
    if (fp) {
        for (int i = 0; i < 5; i++) {
            fwrite(&samples[i], sizeof(Doctor), 1, fp);
        }
        fclose(fp);
    }
}
