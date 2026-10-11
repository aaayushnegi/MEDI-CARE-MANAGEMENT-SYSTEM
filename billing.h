#ifndef BILLING_H
#define BILLING_H

// ==============================================================
// MediCare - Billing Module Header
// BCA 1st Year Project: Demonstrates C Structures & File Handling
// ==============================================================

#define MAX_BILLS 200

// Structure representing a Hospital Bill
typedef struct {
    int id;
    int patientId;
    float consultationFee;
    float medicineCharges;
    float testCharges;
    float otherCharges;
    float totalAmount;
    char paymentStatus[20]; // "Paid", "Pending"
} Bill;

// Function prototypes for billing operations
int generateBill(Bill b);
int getAllBills(Bill arr[], int max);
int getBillById(int id, Bill* b);
int updateBillStatus(int id, const char* status);
int getNextBillId();
void initSampleBills();

#endif // BILLING_H
