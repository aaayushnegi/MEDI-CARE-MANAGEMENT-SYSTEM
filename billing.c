#include "billing.h"
#include "patient.h"
#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ==============================================================
// MediCare - Billing Module Implementation
// Demonstrates C File Handling: fopen, fclose, fread, fwrite
// ==============================================================

static void getFilePath(char* path, int maxLen) {
    getDataFilePath("bills.dat", path, maxLen);
}

// ==============================================================
// Function: getNextBillId
// Purpose: Finds next auto-increment ID for bills
// ==============================================================
int getNextBillId() {
    char path[256];
    getFilePath(path, sizeof(path));

    FILE* fp = fopen(path, "rb");
    if (!fp) return 601; // First bill starts at ID 601

    Bill b;
    int maxId = 600;
    while (fread(&b, sizeof(Bill), 1, fp) == 1) {
        if (b.id > maxId) maxId = b.id;
    }
    fclose(fp);
    return maxId + 1;
}

// ==============================================================
// Function: generateBill
// Purpose: Computes total = fee + medicine + test + other and saves bill
// Returns: 1 on success, 0 on failure
// ==============================================================
int generateBill(Bill b) {
    // Validate Patient ID
    Patient p;
    if (!getPatientById(b.patientId, &p)) {
        printf("Validation Error: Patient ID %d does not exist.\n", b.patientId);
        return 0;
    }

    // Auto-calculate Total Amount
    b.totalAmount = b.consultationFee + b.medicineCharges + b.testCharges + b.otherCharges;

    if (b.id <= 0) {
        b.id = getNextBillId();
    }

    if (strlen(b.paymentStatus) == 0) {
        strcpy(b.paymentStatus, "Pending");
    }

    char path[256];
    getFilePath(path, sizeof(path));

    FILE* fp = fopen(path, "ab");
    if (!fp) return 0;

    size_t written = fwrite(&b, sizeof(Bill), 1, fp);
    fclose(fp);
    return (written == 1) ? 1 : 0;
}

// ==============================================================
// Function: getAllBills
// Purpose: Reads all bills from bills.dat
// ==============================================================
int getAllBills(Bill arr[], int max) {
    char path[256];
    getFilePath(path, sizeof(path));

    FILE* fp = fopen(path, "rb");
    if (!fp) return 0;

    int count = 0;
    while (count < max && fread(&arr[count], sizeof(Bill), 1, fp) == 1) {
        count++;
    }
    fclose(fp);
    return count;
}

// ==============================================================
// Function: getBillById
// Purpose: Finds a bill by ID
// ==============================================================
int getBillById(int id, Bill* b) {
    char path[256];
    getFilePath(path, sizeof(path));

    FILE* fp = fopen(path, "rb");
    if (!fp) return 0;

    Bill temp;
    while (fread(&temp, sizeof(Bill), 1, fp) == 1) {
        if (temp.id == id) {
            *b = temp;
            fclose(fp);
            return 1;
        }
    }
    fclose(fp);
    return 0;
}

// ==============================================================
// Function: updateBillStatus
// Purpose: Toggles/updates payment status (Paid / Pending)
// ==============================================================
int updateBillStatus(int id, const char* status) {
    char path[256];
    getFilePath(path, sizeof(path));

    Bill list[MAX_BILLS];
    int count = getAllBills(list, MAX_BILLS);
    int found = 0;

    for (int i = 0; i < count; i++) {
        if (list[i].id == id) {
            strncpy(list[i].paymentStatus, status, sizeof(list[i].paymentStatus) - 1);
            list[i].paymentStatus[sizeof(list[i].paymentStatus) - 1] = '\0';
            found = 1;
            break;
        }
    }

    if (!found) return 0;

    FILE* fp = fopen(path, "wb");
    if (!fp) return 0;

    for (int i = 0; i < count; i++) {
        fwrite(&list[i], sizeof(Bill), 1, fp);
    }
    fclose(fp);
    return 1;
}

// ==============================================================
// Function: initSampleBills
// Purpose: Populates 5 sample bills if file is empty
// ==============================================================
void initSampleBills() {
    char path[256];
    getFilePath(path, sizeof(path));

    FILE* fp = fopen(path, "rb");
    if (fp) {
        fseek(fp, 0, SEEK_END);
        long size = ftell(fp);
        fclose(fp);
        if (size > 0) return;
    }

    Bill samples[5] = {
        {601, 101, 500.0f, 320.0f, 450.0f, 100.0f, 1370.0f, "Paid"},
        {602, 102, 500.0f, 650.0f, 1200.0f, 150.0f, 2500.0f, "Paid"},
        {603, 103, 600.0f, 850.0f, 0.0f, 50.0f, 1500.0f, "Pending"},
        {604, 105, 800.0f, 1100.0f, 2500.0f, 300.0f, 4700.0f, "Pending"},
        {605, 107, 700.0f, 450.0f, 1800.0f, 200.0f, 3150.0f, "Paid"}
    };

    fp = fopen(path, "wb");
    if (fp) {
        for (int i = 0; i < 5; i++) {
            fwrite(&samples[i], sizeof(Bill), 1, fp);
        }
        fclose(fp);
    }
}
