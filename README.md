# MediCare — Medical Management System

**A Complete College Group Project for 1st-Year BCA (Bachelor of Computer Applications)**

---

## 1. Project Objective

**MediCare** is an interactive, modern healthcare management application designed to automate day-to-day operations of a hospital clinic. It bridges the gap between traditional procedural programming in the **C language** and modern web-based graphical user interfaces (**HTML, CSS, JavaScript**).

The primary educational objective of this project is to demonstrate:
- Practical mastery of **C Structures (`struct`)** to model complex entities.
- Persistent data storage using **C File Handling** (`fopen`, `fclose`, `fread`, `fwrite`, `"rb"`, `"wb"`, `"ab"`).
- Implementation of standard CRUD operations (Create, Read, Update, Delete) in procedural C.
- How a low-level C program can serve a real-time responsive web dashboard using socket networking (**Winsock2**).

---

## 2. High-Level Architecture

```
USER
  │
  ▼
FRONTEND
(HTML5 + CSS3 + Vanilla JavaScript)
  │  ▲
  │  │  HTTP REST Requests (JSON)
  ▼  │
C BACKEND SERVER
(Modular C Program using Winsock2)
  │
  ▼
C FILE HANDLING
(fopen, fread, fwrite, fclose)
  │
  ▼
DATA STORAGE
(patients.dat, doctors.dat, appointments.dat,
 medical_records.dat, prescriptions.dat, bills.dat, beds.dat)
```

---

## 3. Key Features

1. **Authentication & Session:**
   - Dedicated login page with password reveal toggle and demo credential quick-fill.
   - Session guard preventing unauthorized access.

2. **Interactive Dashboard:**
   - Real-time metric cards: Total Patients, Total Doctors, Today's Scheduled Appointments, and Available Beds.
   - Quick action shortcuts for instant record creation.
   - Recent patients table and upcoming appointment schedule preview.

3. **Patient Management:**
   - Full CRUD: Add new patients, edit demographic details, view detailed medical profiles, and delete records.
   - Multi-criteria live search (by name, phone, or disease).
   - Filter by Blood Group and dynamic sorting by ID, Name, or Age.

4. **Doctor Directory:**
   - Categorized directory by Specialization (General Physician, Cardiologist, Dermatologist, Neurologist, Orthopedic, Pediatrician).
   - Shows doctor contact info, clinic timings, and available days.
   - 1-click shortcut to book an appointment with any doctor.

5. **Appointment Scheduling:**
   - Book appointments with auto-populated patient and doctor selectors.
   - Foreign key validation: checks that Patient ID and Doctor ID exist in `.dat` files before booking.
   - Status updates: Mark as "Completed" or "Cancelled".

6. **Medical Records:**
   - Clinical diagnosis records logging symptoms, diagnosis, treatment, physician notes, and dates.
   - Filterable by Patient ID.

7. **Prescription Management (Rx):**
   - Create itemized prescriptions specifying medication name, dosage, frequency (e.g. 1-0-1), duration, and instructions.
   - Printable Rx slip modal with doctor signature line.

8. **Billing System & Invoice Generator:**
   - Real-time dynamic fee calculation:
     $$\text{Total Amount} = \text{Consultation Fee} + \text{Medicine Charges} + \text{Test Charges} + \text{Other Charges}$$
   - Professional printable hospital invoice with itemized charges.
   - Toggle payment status between `Paid` and `Pending`.

9. **Hospital Bed Management:**
   - Visual bed layout color-coded by occupancy (Green = Available, Red = Occupied).
   - Bed admission workflow: assigns patient to available bed.
   - Strict validation preventing re-assignment of an already occupied bed.
   - Discharge / Release bed feature that marks the bed available again.

---

## 4. Technologies Used

- **Frontend:**
  - **HTML5:** Semantic document structure, clean modals, accessible form elements.
  - **CSS3:** Custom responsive layout, healthcare color palette, card components, animations, and `@media print` invoice styling.
  - **Vanilla JavaScript (ES6+):** Asynchronous `fetch()` API calls, DOM manipulation, live search/filtering, toast notifications.
  - *Zero external CDN dependencies (100% offline support).*

- **Backend:**
  - **C Programming Language (C99 standard):**
    - Modular architecture with clean separation of headers (`.h`) and source (`.c`).
    - Standard C libraries: `<stdio.h>`, `<stdlib.h>`, `<string.h>`.
    - Networking: `<winsock2.h>` (`-lws2_32`) for HTTP REST server implementation.

- **Data Storage:**
  - Binary C file handling (`.dat` files in `/data`).

---

## 5. System Requirements

- **Operating System:** Windows 7 / 8 / 10 / 11.
- **Compiler:** MinGW GCC (GCC 4.8 or later recommended).
- **Web Browser:** Any modern browser (Google Chrome, Microsoft Edge, Mozilla Firefox).
- **RAM:** 512 MB minimum.
- **Disk Space:** Less than 10 MB.

---

## 6. Project Structure

```
MediCare/
├── frontend/
│   ├── index.html        # Single-page web application UI and modals
│   ├── style.css         # Modern medical stylesheet & print styles
│   └── script.js         # Frontend controller and API integration
├── backend/
│   ├── main.c            # Winsock HTTP server, routing, static file server
│   ├── common.h / .c     # Data file path resolver & JSON parsing utilities
│   ├── patient.h / .c    # Patient struct and binary file operations
│   ├── doctor.h / .c     # Doctor struct and file operations
│   ├── appointment.h / .c# Appointment struct, validation, and file operations
│   ├── records.h / .c    # Medical records struct and file operations
│   ├── prescription.h / .c# Prescription struct and file operations
│   ├── billing.h / .c    # Billing calculations, struct, and file operations
│   └── beds.h / .c       # Bed occupancy struct, assignment, and file operations
├── data/
│   ├── patients.dat       # Binary data file for patients
│   ├── doctors.dat        # Binary data file for doctors
│   ├── appointments.dat   # Binary data file for appointments
│   ├── medical_records.dat# Binary data file for medical history
│   ├── prescriptions.dat  # Binary data file for prescriptions
│   ├── bills.dat          # Binary data file for patient bills
│   └── beds.dat           # Binary data file for hospital beds
├── run_medicare.bat      # One-click Windows build and launch script
└── README.md             # Complete documentation and viva guide
```

---

## 7. Frontend Explanation

The frontend is constructed without heavy external libraries or frameworks so that 1st-year students can easily explain every line:
1. **`index.html`:** Contains the Login Screen and Main Application Shell. Different views (Dashboard, Patients, Doctors, etc.) are structured as tab sections that toggle visibility via CSS classes.
2. **`style.css`:** Follows a modern healthcare palette: Slate navy (`#0f172a`), Medical cyan (`#0284c7`), Emerald green (`#10b981`), and Danger red (`#ef4444`). Provides responsive layouts using CSS Flexbox and CSS Grid.
3. **`script.js`:** Uses the browser's native `fetch()` function to send HTTP `GET`, `POST`, `PUT`, and `DELETE` requests to the C backend server at `http://localhost:8080`. When data is received, standard DOM methods dynamically construct table rows, cards, and invoice slips.

---

## 8. C Backend Explanation

The backend demonstrates standard 1st-year C programming concepts:
- **`struct` Definitions:** Used to group related attributes (e.g., `Patient` holds `id`, `name`, `age`, `gender`, `phone`, `disease`, etc.).
- **Loops & Arrays:** `while` and `for` loops iterate over structure records during file reading, linear searching, and sorting.
- **Strings:** Manipulated using standard C library functions (`strcpy`, `strncpy`, `strlen`, `strcmp`, `snprintf`).
- **Winsock2 HTTP Server:** Listens on port `8080`. When a request arrives, it extracts the HTTP method and URL, runs the appropriate C file handling function, formats the response into JSON using `snprintf()`, and sends it back to the client socket.

---

## 9. File Handling Explanation (Viva Guide)

Data is saved on disk in binary format (`.dat` files) inside the `data/` directory.

| Mode | Function Used | Purpose in MediCare |
| :--- | :--- | :--- |
| `"rb"` | `fopen(path, "rb")` | **Read Binary:** Opens file to read records into memory using `fread()`. |
| `"ab"` | `fopen(path, "ab")` | **Append Binary:** Adds a new record to the end of the file using `fwrite()`. |
| `"wb"` | `fopen(path, "wb")` | **Write Binary:** Used when updating or deleting records. The modified record array is rewritten to disk. |
| `fread` | `fread(&struct, sizeof(struct), 1, fp)` | Reads one structure at a time directly into a variable in RAM. |
| `fwrite` | `fwrite(&struct, sizeof(struct), 1, fp)` | Writes the bytes of the structure directly onto the storage disk. |
| `fclose` | `fclose(fp)` | Closes the file handle, flushes buffers, and unlocks the file. |

---

## 10. How to Compile and Run

### Method 1: One-Click Windows Script (Easiest)
Simply double-click:
```cmd
run_medicare.bat
```
This script will compile all C source files with GCC, start `backend\server.exe`, and automatically open your default browser to `http://localhost:8080/`.

---

### Method 2: Manual Terminal Compilation

1. Open PowerShell or Command Prompt.
2. Navigate to the `MediCare/backend` directory:
   ```cmd
   cd C:\Users\Aayush\MediCare\backend
   ```
3. Compile all C modules with MinGW GCC:
   ```cmd
   gcc -Wall -Wextra -o server.exe main.c patient.c doctor.c appointment.c records.c prescription.c billing.c beds.c common.c -lws2_32
   ```
4. Run the executable:
   ```cmd
   .\server.exe
   ```
5. Open your web browser and navigate to:
   ```
   http://localhost:8080/
   ```

---

## 11. Demo Login Credentials

- **Username:** `admin`
- **Password:** `admin123`

*(A quick "Click to auto-fill" helper button is also available on the login page for convenience during viva demonstrations).*

---

## 12. Pre-Populated Sample Data

When the backend starts for the first time, it automatically creates and populates realistic fictional records:
- **10 Patients:** Pre-loaded with realistic Indian names, contact numbers, and medical conditions (Hypertension, Diabetes, Asthma, etc.).
- **5 Doctors:** Covering specialties such as General Physician, Cardiologist, Dermatologist, Orthopedic, and Neurologist.
- **8 Appointments:** Covering different dates, times, and statuses (Scheduled and Completed).
- **5 Medical Records:** Detailed diagnoses, presenting symptoms, and clinical notes.
- **5 Prescriptions:** Dosage instructions, medicine names, and frequencies.
- **5 Bills:** Itemized hospital invoices showing calculated totals and payment statuses.
- **10 Hospital Beds:** Distributed across General Ward A, General Ward B, ICU Ward, Emergency Care, and Semi-Private.

---

## 13. System Limitations

- Single-threaded socket listener: Designed specifically for educational demonstration and single-user local evaluation.
- Basic authentication: Hardcoded demo credential verification suitable for 1st-year syllabus presentation.
- Fixed array buffer limits: Entities use fixed maximum size bounds (`MAX_PATIENTS 200`, `MAX_DOCTORS 100`, etc.) suitable for academic project scope.

---

## 14. Future Enhancements

- Multi-threaded client handling using POSIX threads (`pthreads`) or Windows worker threads.
- Role-based login (Doctor, Receptionist, Patient, Pharmacist).
- Integration with an enterprise relational database (e.g. SQLite / MySQL) for production scaling.
- SMS and Email notification dispatch for upcoming appointments.

---

## 15. BCA 1st Year Viva Quick Reference

> **Question: Why did you use C file handling instead of a database like MySQL?**  
> *Answer:* "In our 1st year BCA curriculum, mastering C structures and low-level file handling (`fopen`, `fread`, `fwrite`, `fclose`) is a fundamental core concept. By using `.dat` binary files, our program directly demonstrates binary stream serialization and manual disk persistence without relying on external third-party software."

> **Question: How does the web browser communicate with the C backend?**  
> *Answer:* "Our C backend creates a TCP stream socket using Windows Sockets (`winsock2.h`) listening on port 8080. When the user interacts with the web page, JavaScript's `fetch()` API sends an HTTP request. Our C program parses the request, performs the required file operation, and formats the output into standard JSON, which the browser renders into the UI."

> **Question: How do you prevent invalid appointments or occupied bed re-assignment?**  
> *Answer:* "Before writing an appointment to `appointments.dat`, our `bookAppointment()` function reads `patients.dat` and `doctors.dat` to verify that both IDs exist. Similarly, `assignBed()` reads `beds.dat` and checks if `status == 'Occupied'`; if occupied, it rejects the operation with an error."
