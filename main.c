#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <windows.h>
#include <ctype.h>

#include "common.h"
#include "patient.h"
#include "doctor.h"
#include "appointment.h"
#include "records.h"
#include "prescription.h"
#include "billing.h"
#include "beds.h"

#define PORT 8080
#define BUFFER_SIZE 65536

// ==============================================================
// MediCare - Medical Management System
// Backend Server in C using Winsock2 and C File Handling
// Designed for BCA 1st Year Group Project
// ==============================================================

// Helper to resolve frontend file path (handles running from root or backend/)
static int getFrontendFilePath(const char* filename, char* outPath, int maxLen) {
    snprintf(outPath, maxLen, "frontend/%s", filename);
    FILE* fp = fopen(outPath, "rb");
    if (fp) { fclose(fp); return 1; }

    snprintf(outPath, maxLen, "../frontend/%s", filename);
    fp = fopen(outPath, "rb");
    if (fp) { fclose(fp); return 1; }

    snprintf(outPath, maxLen, "%s", filename);
    fp = fopen(outPath, "rb");
    if (fp) { fclose(fp); return 1; }

    return 0;
}

// Send standard HTTP JSON response with CORS headers
static void send_json_response(SOCKET client_socket, int statusCode, const char* jsonBody) {
    char header[1024];
    const char* statusText = (statusCode == 200) ? "OK" :
                            (statusCode == 201) ? "Created" :
                            (statusCode == 400) ? "Bad Request" :
                            (statusCode == 404) ? "Not Found" : "Internal Server Error";

    int bodyLen = strlen(jsonBody);
    snprintf(header, sizeof(header),
        "HTTP/1.1 %d %s\r\n"
        "Content-Type: application/json; charset=utf-8\r\n"
        "Content-Length: %d\r\n"
        "Access-Control-Allow-Origin: *\r\n"
        "Access-Control-Allow-Methods: GET, POST, PUT, DELETE, OPTIONS\r\n"
        "Access-Control-Allow-Headers: Content-Type, Authorization\r\n"
        "Connection: close\r\n\r\n",
        statusCode, statusText, bodyLen);

    send(client_socket, header, strlen(header), 0);
    if (bodyLen > 0) {
        send(client_socket, jsonBody, bodyLen, 0);
    }
}

// Send static file (HTML, CSS, JS) to browser
static void send_file_response(SOCKET client_socket, const char* filePath, const char* mimeType) {
    FILE* fp = fopen(filePath, "rb");
    if (!fp) {
        const char* notFound = "{\"error\":\"File not found\"}";
        send_json_response(client_socket, 404, notFound);
        return;
    }

    fseek(fp, 0, SEEK_END);
    long fileSize = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    char header[512];
    snprintf(header, sizeof(header),
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: %s; charset=utf-8\r\n"
        "Content-Length: %ld\r\n"
        "Access-Control-Allow-Origin: *\r\n"
        "Connection: close\r\n\r\n",
        mimeType, fileSize);

    send(client_socket, header, strlen(header), 0);

    char fileBuf[4096];
    size_t bytesRead;
    while ((bytesRead = fread(fileBuf, 1, sizeof(fileBuf), fp)) > 0) {
        send(client_socket, fileBuf, bytesRead, 0);
    }

    fclose(fp);
}

// Extract query parameter integer e.g. "?id=101"
static int get_query_int(const char* url, const char* param) {
    char pattern[64];
    snprintf(pattern, sizeof(pattern), "%s=", param);
    const char* p = strstr(url, pattern);
    if (p) {
        return atoi(p + strlen(pattern));
    }
    return 0;
}

// Case-insensitive search for Content-Length
static int parse_content_length(const char* request) {
    const char* cur = request;
    while (*cur) {
        if (strnicmp(cur, "Content-Length:", 15) == 0) {
            cur += 15;
            while (*cur == ' ' || *cur == '\t') cur++;
            return atoi(cur);
        }
        cur++;
    }
    return 0;
}

// Process client HTTP request
void handle_client(SOCKET client_socket) {
    char* recvBuffer = (char*)malloc(BUFFER_SIZE);
    char* jsonBuffer = (char*)malloc(BUFFER_SIZE);

    if (!recvBuffer || !jsonBuffer) {
        if (recvBuffer) free(recvBuffer);
        if (jsonBuffer) free(jsonBuffer);
        closesocket(client_socket);
        return;
    }

    int totalReceived = 0;
    int headerEndPos = -1;
    int contentLen = 0;

    // Read headers
    while (totalReceived < BUFFER_SIZE - 1) {
        int n = recv(client_socket, recvBuffer + totalReceived, BUFFER_SIZE - 1 - totalReceived, 0);
        if (n <= 0) break;
        totalReceived += n;
        recvBuffer[totalReceived] = '\0';

        char* sep = strstr(recvBuffer, "\r\n\r\n");
        if (sep) {
            headerEndPos = (int)(sep - recvBuffer) + 4;
            contentLen = parse_content_length(recvBuffer);
            break;
        }
    }

    if (headerEndPos == -1 && totalReceived == 0) {
        free(recvBuffer);
        free(jsonBuffer);
        closesocket(client_socket);
        return;
    }

    // Read remaining body if Content-Length specified
    if (headerEndPos != -1 && contentLen > 0) {
        int bodyReceived = totalReceived - headerEndPos;
        while (bodyReceived < contentLen && totalReceived < BUFFER_SIZE - 1) {
            int toRead = contentLen - bodyReceived;
            if (toRead > (BUFFER_SIZE - 1 - totalReceived)) {
                toRead = BUFFER_SIZE - 1 - totalReceived;
            }
            int n = recv(client_socket, recvBuffer + totalReceived, toRead, 0);
            if (n <= 0) break;
            totalReceived += n;
            bodyReceived += n;
            recvBuffer[totalReceived] = '\0';
        }
    }

    // Parse HTTP method and URI
    char method[16] = {0};
    char url[256] = {0};
    sscanf(recvBuffer, "%15s %255s", method, url);

    // Locate body
    char* body = "";
    if (headerEndPos != -1 && headerEndPos < totalReceived) {
        body = recvBuffer + headerEndPos;
    }

    // Handle CORS preflight requests
    if (strcmp(method, "OPTIONS") == 0) {
        const char* corsHeader =
            "HTTP/1.1 200 OK\r\n"
            "Access-Control-Allow-Origin: *\r\n"
            "Access-Control-Allow-Methods: GET, POST, PUT, DELETE, OPTIONS\r\n"
            "Access-Control-Allow-Headers: Content-Type, Authorization\r\n"
            "Content-Length: 0\r\n"
            "Connection: close\r\n\r\n";
        send(client_socket, corsHeader, strlen(corsHeader), 0);
        shutdown(client_socket, SD_SEND);
        closesocket(client_socket);
        free(recvBuffer);
        free(jsonBuffer);
        return;
    }

    // Strip query string for path matching
    char path[256];
    strncpy(path, url, sizeof(path) - 1);
    path[sizeof(path) - 1] = '\0';
    char* qMark = strchr(path, '?');
    if (qMark) *qMark = '\0';

    // -------------------------------------------------------------
    // API: SYSTEM STATS (Dashboard Overview)
    // -------------------------------------------------------------
    if (strcmp(path, "/stats") == 0 || strcmp(path, "/api/stats") == 0) {
        Patient pArr[MAX_PATIENTS];
        Doctor dArr[MAX_DOCTORS];
        Appointment aArr[MAX_APPOINTMENTS];
        Bed bArr[MAX_BEDS];

        int totalP = getAllPatients(pArr, MAX_PATIENTS);
        int totalD = getAllDoctors(dArr, MAX_DOCTORS);
        int totalA = getAllAppointments(aArr, MAX_APPOINTMENTS);
        int totalB = getAllBeds(bArr, MAX_BEDS);

        int availBeds = 0;
        int occBeds = 0;
        for (int i = 0; i < totalB; i++) {
            if (strcmp(bArr[i].status, "Available") == 0) availBeds++;
            else occBeds++;
        }

        int todayAppointments = 0;
        for (int i = 0; i < totalA; i++) {
            if (strcmp(aArr[i].status, "Scheduled") == 0) todayAppointments++;
        }

        snprintf(jsonBuffer, BUFFER_SIZE,
            "{\"totalPatients\":%d,\"totalDoctors\":%d,\"todayAppointments\":%d,"
            "\"totalBeds\":%d,\"availableBeds\":%d,\"occupiedBeds\":%d}",
            totalP, totalD, todayAppointments, totalB, availBeds, occBeds);
        send_json_response(client_socket, 200, jsonBuffer);
    }

    // -------------------------------------------------------------
    // API: PATIENTS (/patients or /api/patients)
    // -------------------------------------------------------------
    else if (strcmp(path, "/patients") == 0 || strcmp(path, "/api/patients") == 0) {
        if (strcmp(method, "GET") == 0) {
            Patient pArr[MAX_PATIENTS];
            int count = getAllPatients(pArr, MAX_PATIENTS);

            strcpy(jsonBuffer, "[");
            for (int i = 0; i < count; i++) {
                char item[512];
                char escName[128], escAddr[180], escDis[128];
                escape_json(pArr[i].name, escName, sizeof(escName));
                escape_json(pArr[i].address, escAddr, sizeof(escAddr));
                escape_json(pArr[i].disease, escDis, sizeof(escDis));

                snprintf(item, sizeof(item),
                    "%s{\"id\":%d,\"name\":\"%s\",\"age\":%d,\"gender\":\"%s\",\"phone\":\"%s\","
                    "\"address\":\"%s\",\"bloodGroup\":\"%s\",\"disease\":\"%s\",\"emergencyContact\":\"%s\"}",
                    (i > 0 ? "," : ""),
                    pArr[i].id, escName, pArr[i].age, pArr[i].gender, pArr[i].phone,
                    escAddr, pArr[i].bloodGroup, escDis, pArr[i].emergencyContact);
                strcat(jsonBuffer, item);
            }
            strcat(jsonBuffer, "]");
            send_json_response(client_socket, 200, jsonBuffer);
        }
        else if (strcmp(method, "POST") == 0) {
            Patient p;
            memset(&p, 0, sizeof(p));
            get_json_string(body, "name", p.name, sizeof(p.name));
            get_json_int(body, "age", &p.age);
            get_json_string(body, "gender", p.gender, sizeof(p.gender));
            get_json_string(body, "phone", p.phone, sizeof(p.phone));
            get_json_string(body, "address", p.address, sizeof(p.address));
            get_json_string(body, "bloodGroup", p.bloodGroup, sizeof(p.bloodGroup));
            get_json_string(body, "disease", p.disease, sizeof(p.disease));
            get_json_string(body, "emergencyContact", p.emergencyContact, sizeof(p.emergencyContact));

            if (strlen(p.name) == 0) {
                send_json_response(client_socket, 400, "{\"success\":false,\"error\":\"Patient name is required\"}");
            } else {
                p.id = getNextPatientId();
                if (addPatient(p)) {
                    snprintf(jsonBuffer, BUFFER_SIZE, "{\"success\":true,\"message\":\"Patient added successfully!\",\"id\":%d}", p.id);
                    send_json_response(client_socket, 201, jsonBuffer);
                } else {
                    send_json_response(client_socket, 500, "{\"success\":false,\"error\":\"Failed to save patient to file\"}");
                }
            }
        }
        else if (strcmp(method, "PUT") == 0) {
            Patient p;
            memset(&p, 0, sizeof(p));
            get_json_int(body, "id", &p.id);
            get_json_string(body, "name", p.name, sizeof(p.name));
            get_json_int(body, "age", &p.age);
            get_json_string(body, "gender", p.gender, sizeof(p.gender));
            get_json_string(body, "phone", p.phone, sizeof(p.phone));
            get_json_string(body, "address", p.address, sizeof(p.address));
            get_json_string(body, "bloodGroup", p.bloodGroup, sizeof(p.bloodGroup));
            get_json_string(body, "disease", p.disease, sizeof(p.disease));
            get_json_string(body, "emergencyContact", p.emergencyContact, sizeof(p.emergencyContact));

            if (p.id <= 0 || strlen(p.name) == 0) {
                send_json_response(client_socket, 400, "{\"success\":false,\"error\":\"Valid Patient ID and Name required\"}");
            } else if (updatePatient(p)) {
                send_json_response(client_socket, 200, "{\"success\":true,\"message\":\"Patient updated successfully!\"}");
            } else {
                send_json_response(client_socket, 404, "{\"success\":false,\"error\":\"Patient ID not found\"}");
            }
        }
        else if (strcmp(method, "DELETE") == 0) {
            int id = get_query_int(url, "id");
            if (id <= 0) get_json_int(body, "id", &id);

            if (id > 0 && deletePatient(id)) {
                send_json_response(client_socket, 200, "{\"success\":true,\"message\":\"Patient deleted successfully!\"}");
            } else {
                send_json_response(client_socket, 404, "{\"success\":false,\"error\":\"Patient not found or could not be deleted\"}");
            }
        }
    }

    // -------------------------------------------------------------
    // API: DOCTORS (/doctors or /api/doctors)
    // -------------------------------------------------------------
    else if (strcmp(path, "/doctors") == 0 || strcmp(path, "/api/doctors") == 0) {
        if (strcmp(method, "GET") == 0) {
            Doctor dArr[MAX_DOCTORS];
            int count = getAllDoctors(dArr, MAX_DOCTORS);

            strcpy(jsonBuffer, "[");
            for (int i = 0; i < count; i++) {
                char item[512];
                char escName[128], escSpec[128];
                escape_json(dArr[i].name, escName, sizeof(escName));
                escape_json(dArr[i].specialization, escSpec, sizeof(escSpec));

                snprintf(item, sizeof(item),
                    "%s{\"id\":%d,\"name\":\"%s\",\"specialization\":\"%s\",\"phone\":\"%s\","
                    "\"email\":\"%s\",\"availableDays\":\"%s\",\"availableTime\":\"%s\"}",
                    (i > 0 ? "," : ""),
                    dArr[i].id, escName, escSpec, dArr[i].phone,
                    dArr[i].email, dArr[i].availableDays, dArr[i].availableTime);
                strcat(jsonBuffer, item);
            }
            strcat(jsonBuffer, "]");
            send_json_response(client_socket, 200, jsonBuffer);
        }
        else if (strcmp(method, "POST") == 0) {
            Doctor d;
            memset(&d, 0, sizeof(d));
            get_json_string(body, "name", d.name, sizeof(d.name));
            get_json_string(body, "specialization", d.specialization, sizeof(d.specialization));
            get_json_string(body, "phone", d.phone, sizeof(d.phone));
            get_json_string(body, "email", d.email, sizeof(d.email));
            get_json_string(body, "availableDays", d.availableDays, sizeof(d.availableDays));
            get_json_string(body, "availableTime", d.availableTime, sizeof(d.availableTime));

            if (strlen(d.name) == 0 || strlen(d.specialization) == 0) {
                send_json_response(client_socket, 400, "{\"success\":false,\"error\":\"Doctor name and specialization required\"}");
            } else {
                d.id = getNextDoctorId();
                if (addDoctor(d)) {
                    snprintf(jsonBuffer, BUFFER_SIZE, "{\"success\":true,\"message\":\"Doctor added successfully!\",\"id\":%d}", d.id);
                    send_json_response(client_socket, 201, jsonBuffer);
                } else {
                    send_json_response(client_socket, 500, "{\"success\":false,\"error\":\"Failed to save doctor to file\"}");
                }
            }
        }
        else if (strcmp(method, "DELETE") == 0) {
            int id = get_query_int(url, "id");
            if (id <= 0) get_json_int(body, "id", &id);

            if (id > 0 && deleteDoctor(id)) {
                send_json_response(client_socket, 200, "{\"success\":true,\"message\":\"Doctor removed successfully!\"}");
            } else {
                send_json_response(client_socket, 404, "{\"success\":false,\"error\":\"Doctor not found\"}");
            }
        }
    }

    // -------------------------------------------------------------
    // API: APPOINTMENTS (/appointments or /api/appointments)
    // -------------------------------------------------------------
    else if (strcmp(path, "/appointments") == 0 || strcmp(path, "/api/appointments") == 0) {
        if (strcmp(method, "GET") == 0) {
            Appointment aArr[MAX_APPOINTMENTS];
            int count = getAllAppointments(aArr, MAX_APPOINTMENTS);

            strcpy(jsonBuffer, "[");
            for (int i = 0; i < count; i++) {
                char item[512];
                char escPName[128], escDName[128], escReason[180];
                escape_json(aArr[i].patientName, escPName, sizeof(escPName));
                escape_json(aArr[i].doctorName, escDName, sizeof(escDName));
                escape_json(aArr[i].reason, escReason, sizeof(escReason));

                snprintf(item, sizeof(item),
                    "%s{\"id\":%d,\"patientId\":%d,\"patientName\":\"%s\",\"doctorId\":%d,\"doctorName\":\"%s\","
                    "\"date\":\"%s\",\"time\":\"%s\",\"reason\":\"%s\",\"status\":\"%s\"}",
                    (i > 0 ? "," : ""),
                    aArr[i].id, aArr[i].patientId, escPName, aArr[i].doctorId, escDName,
                    aArr[i].date, aArr[i].time, escReason, aArr[i].status);
                strcat(jsonBuffer, item);
            }
            strcat(jsonBuffer, "]");
            send_json_response(client_socket, 200, jsonBuffer);
        }
        else if (strcmp(method, "POST") == 0) {
            Appointment a;
            memset(&a, 0, sizeof(a));
            get_json_int(body, "patientId", &a.patientId);
            get_json_int(body, "doctorId", &a.doctorId);
            get_json_string(body, "patientName", a.patientName, sizeof(a.patientName));
            get_json_string(body, "doctorName", a.doctorName, sizeof(a.doctorName));
            get_json_string(body, "date", a.date, sizeof(a.date));
            get_json_string(body, "time", a.time, sizeof(a.time));
            get_json_string(body, "reason", a.reason, sizeof(a.reason));
            get_json_string(body, "status", a.status, sizeof(a.status));

            if (a.patientId <= 0 || a.doctorId <= 0) {
                send_json_response(client_socket, 400, "{\"success\":false,\"error\":\"Valid Patient ID and Doctor ID required\"}");
            } else if (bookAppointment(a)) {
                send_json_response(client_socket, 201, "{\"success\":true,\"message\":\"Appointment booked successfully!\"}");
            } else {
                send_json_response(client_socket, 400, "{\"success\":false,\"error\":\"Booking failed. Verify Patient ID and Doctor ID exist.\"}");
            }
        }
        else if (strcmp(method, "PUT") == 0) {
            int id = 0;
            char status[32] = {0};
            get_json_int(body, "id", &id);
            get_json_string(body, "status", status, sizeof(status));

            if (id > 0 && strlen(status) > 0 && updateAppointmentStatus(id, status)) {
                send_json_response(client_socket, 200, "{\"success\":true,\"message\":\"Appointment updated successfully!\"}");
            } else {
                send_json_response(client_socket, 400, "{\"success\":false,\"error\":\"Could not update appointment status\"}");
            }
        }
        else if (strcmp(method, "DELETE") == 0) {
            int id = get_query_int(url, "id");
            if (id <= 0) get_json_int(body, "id", &id);

            if (id > 0 && deleteAppointment(id)) {
                send_json_response(client_socket, 200, "{\"success\":true,\"message\":\"Appointment cancelled/deleted!\"}");
            } else {
                send_json_response(client_socket, 404, "{\"success\":false,\"error\":\"Appointment not found\"}");
            }
        }
    }

    // -------------------------------------------------------------
    // API: MEDICAL RECORDS (/records or /api/records)
    // -------------------------------------------------------------
    else if (strcmp(path, "/records") == 0 || strcmp(path, "/api/records") == 0) {
        if (strcmp(method, "GET") == 0) {
            int patientId = get_query_int(url, "patientId");
            MedicalRecord rArr[MAX_RECORDS];
            int count = 0;

            if (patientId > 0) {
                count = getMedicalRecordsByPatient(patientId, rArr, MAX_RECORDS);
            } else {
                count = getAllMedicalRecords(rArr, MAX_RECORDS);
            }

            strcpy(jsonBuffer, "[");
            for (int i = 0; i < count; i++) {
                char item[768];
                char escDiag[160], escSymp[220], escPres[220], escNotes[270];
                escape_json(rArr[i].diagnosis, escDiag, sizeof(escDiag));
                escape_json(rArr[i].symptoms, escSymp, sizeof(escSymp));
                escape_json(rArr[i].prescription, escPres, sizeof(escPres));
                escape_json(rArr[i].notes, escNotes, sizeof(escNotes));

                snprintf(item, sizeof(item),
                    "%s{\"id\":%d,\"patientId\":%d,\"doctorId\":%d,\"diagnosis\":\"%s\","
                    "\"symptoms\":\"%s\",\"prescription\":\"%s\",\"date\":\"%s\",\"notes\":\"%s\"}",
                    (i > 0 ? "," : ""),
                    rArr[i].id, rArr[i].patientId, rArr[i].doctorId,
                    escDiag, escSymp, escPres, rArr[i].date, escNotes);
                strcat(jsonBuffer, item);
            }
            strcat(jsonBuffer, "]");
            send_json_response(client_socket, 200, jsonBuffer);
        }
        else if (strcmp(method, "POST") == 0) {
            MedicalRecord r;
            memset(&r, 0, sizeof(r));
            get_json_int(body, "patientId", &r.patientId);
            get_json_int(body, "doctorId", &r.doctorId);
            get_json_string(body, "diagnosis", r.diagnosis, sizeof(r.diagnosis));
            get_json_string(body, "symptoms", r.symptoms, sizeof(r.symptoms));
            get_json_string(body, "prescription", r.prescription, sizeof(r.prescription));
            get_json_string(body, "date", r.date, sizeof(r.date));
            get_json_string(body, "notes", r.notes, sizeof(r.notes));

            if (r.patientId <= 0 || strlen(r.diagnosis) == 0) {
                send_json_response(client_socket, 400, "{\"success\":false,\"error\":\"Patient ID and Diagnosis required\"}");
            } else {
                r.id = getNextRecordId();
                if (addMedicalRecord(r)) {
                    send_json_response(client_socket, 201, "{\"success\":true,\"message\":\"Medical record saved successfully!\"}");
                } else {
                    send_json_response(client_socket, 500, "{\"success\":false,\"error\":\"Failed to save medical record\"}");
                }
            }
        }
    }

    // -------------------------------------------------------------
    // API: PRESCRIPTIONS (/prescriptions or /api/prescriptions)
    // -------------------------------------------------------------
    else if (strcmp(path, "/prescriptions") == 0 || strcmp(path, "/api/prescriptions") == 0) {
        if (strcmp(method, "GET") == 0) {
            int patientId = get_query_int(url, "patientId");
            Prescription pArr[MAX_PRESCRIPTIONS];
            int count = 0;

            if (patientId > 0) {
                count = getPrescriptionsByPatient(patientId, pArr, MAX_PRESCRIPTIONS);
            } else {
                count = getAllPrescriptions(pArr, MAX_PRESCRIPTIONS);
            }

            strcpy(jsonBuffer, "[");
            for (int i = 0; i < count; i++) {
                char item[512];
                char escMed[128], escInst[220];
                escape_json(pArr[i].medicineName, escMed, sizeof(escMed));
                escape_json(pArr[i].instructions, escInst, sizeof(escInst));

                snprintf(item, sizeof(item),
                    "%s{\"id\":%d,\"patientId\":%d,\"doctorId\":%d,\"medicineName\":\"%s\","
                    "\"dosage\":\"%s\",\"frequency\":\"%s\",\"duration\":\"%s\",\"instructions\":\"%s\"}",
                    (i > 0 ? "," : ""),
                    pArr[i].id, pArr[i].patientId, pArr[i].doctorId,
                    escMed, pArr[i].dosage, pArr[i].frequency, pArr[i].duration, escInst);
                strcat(jsonBuffer, item);
            }
            strcat(jsonBuffer, "]");
            send_json_response(client_socket, 200, jsonBuffer);
        }
        else if (strcmp(method, "POST") == 0) {
            Prescription p;
            memset(&p, 0, sizeof(p));
            get_json_int(body, "patientId", &p.patientId);
            get_json_int(body, "doctorId", &p.doctorId);
            get_json_string(body, "medicineName", p.medicineName, sizeof(p.medicineName));
            get_json_string(body, "dosage", p.dosage, sizeof(p.dosage));
            get_json_string(body, "frequency", p.frequency, sizeof(p.frequency));
            get_json_string(body, "duration", p.duration, sizeof(p.duration));
            get_json_string(body, "instructions", p.instructions, sizeof(p.instructions));

            if (p.patientId <= 0 || strlen(p.medicineName) == 0) {
                send_json_response(client_socket, 400, "{\"success\":false,\"error\":\"Patient ID and Medicine Name required\"}");
            } else {
                p.id = getNextPrescriptionId();
                if (addPrescription(p)) {
                    send_json_response(client_socket, 201, "{\"success\":true,\"message\":\"Prescription created successfully!\"}");
                } else {
                    send_json_response(client_socket, 500, "{\"success\":false,\"error\":\"Failed to save prescription\"}");
                }
            }
        }
        else if (strcmp(method, "DELETE") == 0) {
            int id = get_query_int(url, "id");
            if (id <= 0) get_json_int(body, "id", &id);

            if (id > 0 && deletePrescription(id)) {
                send_json_response(client_socket, 200, "{\"success\":true,\"message\":\"Prescription deleted successfully!\"}");
            } else {
                send_json_response(client_socket, 404, "{\"success\":false,\"error\":\"Prescription not found\"}");
            }
        }
    }

    // -------------------------------------------------------------
    // API: BILLING (/bills or /api/bills)
    // -------------------------------------------------------------
    else if (strcmp(path, "/bills") == 0 || strcmp(path, "/api/bills") == 0) {
        if (strcmp(method, "GET") == 0) {
            Bill bArr[MAX_BILLS];
            int count = getAllBills(bArr, MAX_BILLS);

            strcpy(jsonBuffer, "[");
            for (int i = 0; i < count; i++) {
                char item[256];
                snprintf(item, sizeof(item),
                    "%s{\"id\":%d,\"patientId\":%d,\"consultationFee\":%.2f,\"medicineCharges\":%.2f,"
                    "\"testCharges\":%.2f,\"otherCharges\":%.2f,\"totalAmount\":%.2f,\"paymentStatus\":\"%s\"}",
                    (i > 0 ? "," : ""),
                    bArr[i].id, bArr[i].patientId, bArr[i].consultationFee, bArr[i].medicineCharges,
                    bArr[i].testCharges, bArr[i].otherCharges, bArr[i].totalAmount, bArr[i].paymentStatus);
                strcat(jsonBuffer, item);
            }
            strcat(jsonBuffer, "]");
            send_json_response(client_socket, 200, jsonBuffer);
        }
        else if (strcmp(method, "POST") == 0) {
            Bill b;
            memset(&b, 0, sizeof(b));
            get_json_int(body, "patientId", &b.patientId);
            get_json_float(body, "consultationFee", &b.consultationFee);
            get_json_float(body, "medicineCharges", &b.medicineCharges);
            get_json_float(body, "testCharges", &b.testCharges);
            get_json_float(body, "otherCharges", &b.otherCharges);
            get_json_string(body, "paymentStatus", b.paymentStatus, sizeof(b.paymentStatus));

            // Validation: non-negative charges and existing patient
            if (b.patientId <= 0 || b.consultationFee < 0 || b.medicineCharges < 0 || b.testCharges < 0 || b.otherCharges < 0) {
                send_json_response(client_socket, 400, "{\"success\":false,\"error\":\"Invalid Patient ID or negative charges entered\"}");
            } else if (generateBill(b)) {
                send_json_response(client_socket, 201, "{\"success\":true,\"message\":\"Bill generated successfully!\"}");
            } else {
                send_json_response(client_socket, 400, "{\"success\":false,\"error\":\"Failed to generate bill. Patient ID must exist.\"}");
            }
        }
        else if (strcmp(method, "PUT") == 0) {
            int id = 0;
            char status[32] = {0};
            get_json_int(body, "id", &id);
            get_json_string(body, "paymentStatus", status, sizeof(status));

            if (id > 0 && strlen(status) > 0 && updateBillStatus(id, status)) {
                send_json_response(client_socket, 200, "{\"success\":true,\"message\":\"Payment status updated!\"}");
            } else {
                send_json_response(client_socket, 400, "{\"success\":false,\"error\":\"Could not update payment status\"}");
            }
        }
    }

    // -------------------------------------------------------------
    // API: BEDS (/beds or /api/beds)
    // -------------------------------------------------------------
    else if (strcmp(path, "/beds") == 0 || strcmp(path, "/api/beds") == 0) {
        if (strcmp(method, "GET") == 0) {
            Bed bArr[MAX_BEDS];
            int count = getAllBeds(bArr, MAX_BEDS);

            strcpy(jsonBuffer, "[");
            for (int i = 0; i < count; i++) {
                char item[256];
                char escWard[64];
                escape_json(bArr[i].ward, escWard, sizeof(escWard));

                snprintf(item, sizeof(item),
                    "%s{\"id\":%d,\"ward\":\"%s\",\"bedNumber\":\"%s\",\"patientId\":%d,\"status\":\"%s\"}",
                    (i > 0 ? "," : ""),
                    bArr[i].id, escWard, bArr[i].bedNumber, bArr[i].patientId, bArr[i].status);
                strcat(jsonBuffer, item);
            }
            strcat(jsonBuffer, "]");
            send_json_response(client_socket, 200, jsonBuffer);
        }
        else if (strcmp(method, "POST") == 0) {
            char action[32] = {0};
            int bedId = 0, patientId = 0;
            get_json_string(body, "action", action, sizeof(action));
            get_json_int(body, "bedId", &bedId);
            get_json_int(body, "patientId", &patientId);

            if (strcmp(action, "assign") == 0) {
                if (bedId <= 0 || patientId <= 0) {
                    send_json_response(client_socket, 400, "{\"success\":false,\"error\":\"Bed ID and Patient ID are required\"}");
                } else if (assignBed(bedId, patientId)) {
                    send_json_response(client_socket, 200, "{\"success\":true,\"message\":\"Bed assigned successfully!\"}");
                } else {
                    send_json_response(client_socket, 400, "{\"success\":false,\"error\":\"Cannot assign bed. It is already occupied or Patient ID does not exist.\"}");
                }
            } else if (strcmp(action, "release") == 0) {
                if (bedId > 0 && releaseBed(bedId)) {
                    send_json_response(client_socket, 200, "{\"success\":true,\"message\":\"Bed released successfully!\"}");
                } else {
                    send_json_response(client_socket, 400, "{\"success\":false,\"error\":\"Failed to release bed\"}");
                }
            } else {
                send_json_response(client_socket, 400, "{\"success\":false,\"error\":\"Unknown bed action\"}");
            }
        }
    }

    // -------------------------------------------------------------
    // STATIC FILE SERVING (Frontend UI)
    // -------------------------------------------------------------
    else if (strcmp(path, "/") == 0 || strcmp(path, "/index.html") == 0) {
        char filePath[256];
        if (getFrontendFilePath("index.html", filePath, sizeof(filePath))) {
            send_file_response(client_socket, filePath, "text/html");
        } else {
            send_json_response(client_socket, 404, "{\"error\":\"index.html not found\"}");
        }
    }
    else if (strcmp(path, "/style.css") == 0) {
        char filePath[256];
        if (getFrontendFilePath("style.css", filePath, sizeof(filePath))) {
            send_file_response(client_socket, filePath, "text/css");
        } else {
            send_json_response(client_socket, 404, "{\"error\":\"style.css not found\"}");
        }
    }
    else if (strcmp(path, "/script.js") == 0) {
        char filePath[256];
        if (getFrontendFilePath("script.js", filePath, sizeof(filePath))) {
            send_file_response(client_socket, filePath, "application/javascript");
        } else {
            send_json_response(client_socket, 404, "{\"error\":\"script.js not found\"}");
        }
    }
    else {
        send_json_response(client_socket, 404, "{\"error\":\"Endpoint not found\"}");
    }

    // Gracefully finish sending and close socket
    shutdown(client_socket, SD_SEND);
    closesocket(client_socket);

    free(recvBuffer);
    free(jsonBuffer);
}

// ==============================================================
// Main Server Entry Point
// ==============================================================
int main() {
    printf("==============================================================\n");
    printf("   MediCare -- Medical Management System (C Backend)          \n");
    printf("   BCA 1st Year Group Project                                 \n");
    printf("==============================================================\n");

    // Initialize Sample Data in binary .dat files if not already present
    printf("[1/3] Checking and initializing database files...\n");
    initSamplePatients();
    initSampleDoctors();
    initSampleAppointments();
    initSampleRecords();
    initSamplePrescriptions();
    initSampleBills();
    initSampleBeds();
    printf("      -> All sample records verified and loaded in data/*.dat\n");

    // Initialize Winsock
    printf("[2/3] Initializing Windows Sockets (Winsock2)...\n");
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        printf("Error: WSAStartup failed.\n");
        return 1;
    }

    SOCKET server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == INVALID_SOCKET) {
        printf("Error: Could not create socket.\n");
        WSACleanup();
        return 1;
    }

    // Set SO_REUSEADDR
    char opt = 1;
    setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        printf("Error: Bind failed on port %d. Error code: %d\n", PORT, WSAGetLastError());
        closesocket(server_socket);
        WSACleanup();
        return 1;
    }

    if (listen(server_socket, 15) == SOCKET_ERROR) {
        printf("Error: Listen failed.\n");
        closesocket(server_socket);
        WSACleanup();
        return 1;
    }

    printf("[3/3] MediCare Server is ONLINE!\n");
    printf("--------------------------------------------------------------\n");
    printf(">> Open your web browser and go to: http://localhost:%d/\n", PORT);
    printf(">> REST API Endpoints active at:     http://localhost:%d/api/...\n", PORT);
    printf(">> Demo Login: Username: admin | Password: admin123\n");
    printf(">> Press Ctrl+C in this console window to stop server.\n");
    printf("--------------------------------------------------------------\n\n");

    // Server loop: accept incoming browser / API requests
    while (1) {
        struct sockaddr_in client_addr;
        int client_addr_len = sizeof(client_addr);
        SOCKET client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_addr_len);

        if (client_socket != INVALID_SOCKET) {
            handle_client(client_socket);
        }
    }

    closesocket(server_socket);
    WSACleanup();
    return 0;
}
