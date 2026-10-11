// ==============================================================
// MediCare - Medical Management System
// Frontend Controller (Vanilla JavaScript)
// BCA 1st Year College Project
// ==============================================================

// Base URL configuration (works whether served directly from C backend or opened via file://)
const API_BASE = window.location.origin && window.location.origin.startsWith('http')
    ? window.location.origin
    : 'http://localhost:8080';

// Global Application State (in-memory cache synced with C backend)
let appState = {
    patients: [],
    doctors: [],
    appointments: [],
    records: [],
    prescriptions: [],
    bills: [],
    beds: [],
    stats: {
        totalPatients: 0,
        totalDoctors: 0,
        todayAppointments: 0,
        totalBeds: 0,
        availableBeds: 0,
        occupiedBeds: 0
    }
};

// ==============================================================
// 1. INITIALIZATION & AUTHENTICATION
// ==============================================================
document.addEventListener('DOMContentLoaded', () => {
    startClock();
    checkExistingAuth();
    
    // Set default date inputs to today
    const todayStr = new Date().toISOString().split('T')[0];
    const dateInputs = ['appDate', 'recDate'];
    dateInputs.forEach(id => {
        const el = document.getElementById(id);
        if (el) el.value = todayStr;
    });
});

// Live clock update
function startClock() {
    function update() {
        const now = new Date();
        const clockEl = document.getElementById('systemClock');
        if (clockEl) {
            clockEl.textContent = now.toLocaleDateString('en-US', {
                month: 'short',
                day: 'numeric',
                year: 'numeric'
            }) + ' | ' + now.toLocaleTimeString('en-US', {
                hour: '2-digit',
                minute: '2-digit',
                second: '2-digit'
            });
        }
    }
    update();
    setInterval(update, 1000);
}

// Check if user is already logged in
function checkExistingAuth() {
    const isLoggedIn = sessionStorage.getItem('medicare_auth') === 'true';
    if (isLoggedIn) {
        showAppShell();
        fetchAllData();
    } else {
        showLoginScreen();
    }
}

// Toggle password visibility in login form
function togglePasswordVisibility() {
    const pwdInput = document.getElementById('password');
    if (pwdInput.type === 'password') {
        pwdInput.type = 'text';
    } else {
        pwdInput.type = 'password';
    }
}

// Quick fill demo credentials
function fillDemoCredentials() {
    document.getElementById('username').value = 'admin';
    document.getElementById('password').value = 'admin123';
    showToast('Demo credentials filled!', 'info');
}

// Handle login submission
function handleLogin(event) {
    event.preventDefault();
    const user = document.getElementById('username').value.trim();
    const pass = document.getElementById('password').value;
    const errBox = document.getElementById('loginError');

    // Simple college demonstration credential check
    if (user === 'admin' && pass === 'admin123') {
        sessionStorage.setItem('medicare_auth', 'true');
        errBox.style.display = 'none';
        showToast('Login successful! Welcome Dr. Administrator.', 'success');
        showAppShell();
        fetchAllData();
    } else {
        errBox.style.display = 'block';
        showToast('Invalid credentials. Use admin / admin123', 'error');
    }
}

// Logout handler
function handleLogout() {
    openConfirmModal(
        'Confirm Logout',
        'Are you sure you want to end your current session?',
        () => {
            sessionStorage.removeItem('medicare_auth');
            showLoginScreen();
            showToast('You have been logged out safely.', 'info');
        }
    );
}

function showLoginScreen() {
    document.getElementById('loginScreen').style.display = 'flex';
    document.getElementById('appShell').style.display = 'none';
}

function showAppShell() {
    document.getElementById('loginScreen').style.display = 'none';
    document.getElementById('appShell').style.display = 'flex';
}

// Mobile sidebar toggle
function toggleMobileSidebar() {
    const sidebar = document.querySelector('.sidebar');
    if (sidebar) {
        sidebar.classList.toggle('mobile-open');
    }
}

// ==============================================================
// 2. NAVIGATION & TABS
// ==============================================================
function switchTab(tabName) {
    // Update sidebar navigation active state
    const navItems = document.querySelectorAll('.nav-item');
    navItems.forEach(item => {
        item.classList.remove('active');
        if (item.getAttribute('onclick') && item.getAttribute('onclick').includes(tabName)) {
            item.classList.add('active');
        }
    });

    // Update tab content displays
    const tabs = document.querySelectorAll('.tab-content');
    tabs.forEach(tab => tab.classList.remove('active'));

    const activeTab = document.getElementById(`tab-${tabName}`);
    if (activeTab) {
        activeTab.classList.add('active');
    }

    // Update page header title and breadcrumb
    const titleMap = {
        'dashboard': 'Hospital Dashboard',
        'patients': 'Patient Management',
        'doctors': 'Doctor Directory',
        'appointments': 'Appointments Schedule',
        'records': 'Medical Health Records',
        'prescriptions': 'Prescription Management',
        'billing': 'Hospital Billing & Invoices',
        'beds': 'Bed Occupancy & Management'
    };

    document.getElementById('pageTitle').textContent = titleMap[tabName] || 'Hospital Management';
    document.getElementById('pageBreadcrumb').textContent = `MediCare / ${tabName.charAt(0).toUpperCase() + tabName.slice(1)}`;

    // Refresh data if switching to specific tabs
    if (tabName === 'dashboard') {
        renderDashboard();
    }

    // Close mobile sidebar if open
    const sidebar = document.querySelector('.sidebar');
    if (sidebar) {
        sidebar.classList.remove('mobile-open');
    }
}

// ==============================================================
// 3. API CLIENT (FETCH DATA FROM C BACKEND)
// ==============================================================
async function apiCall(endpoint, method = 'GET', data = null) {
    try {
        const options = {
            method: method,
            headers: {
                'Content-Type': 'application/json'
            }
        };
        if (data && (method === 'POST' || method === 'PUT' || method === 'DELETE')) {
            options.body = JSON.stringify(data);
        }

        const res = await fetch(`${API_BASE}${endpoint}`, options);
        if (!res.ok) {
            const errJson = await res.json().catch(() => ({}));
            throw new Error(errJson.error || `Server responded with status ${res.status}`);
        }
        return await res.json();
    } catch (err) {
        console.error(`API Error on ${method} ${endpoint}:`, err);
        throw err;
    }
}

// Fetch all data from C backend
async function fetchAllData() {
    try {
        const [stats, patients, doctors, appointments, records, prescriptions, bills, beds] = await Promise.all([
            apiCall('/stats').catch(() => null),
            apiCall('/patients'),
            apiCall('/doctors'),
            apiCall('/appointments'),
            apiCall('/records'),
            apiCall('/prescriptions'),
            apiCall('/bills'),
            apiCall('/beds')
        ]);

        if (patients) appState.patients = patients;
        if (doctors) appState.doctors = doctors;
        if (appointments) appState.appointments = appointments;
        if (records) appState.records = records;
        if (prescriptions) appState.prescriptions = prescriptions;
        if (bills) appState.bills = bills;
        if (beds) appState.beds = beds;
        if (stats) appState.stats = stats;

        // Render all views
        renderDashboard();
        renderPatients();
        renderDoctors();
        renderAppointments();
        renderRecords();
        renderPrescriptions();
        renderBills();
        renderBeds();
    } catch (err) {
        showToast('Could not sync with C backend: ' + err.message, 'error');
    }
}

// ==============================================================
// 4. DASHBOARD MODULE
// ==============================================================
function renderDashboard() {
    const totalP = appState.patients.length;
    const totalD = appState.doctors.length;
    const totalAppt = appState.appointments.filter(a => a.status === 'Scheduled').length;
    const totalB = appState.beds.length;
    const availB = appState.beds.filter(b => b.status === 'Available').length;
    const occB = totalB - availB;

    document.getElementById('statTotalPatients').textContent = totalP;
    document.getElementById('statTotalDoctors').textContent = totalD;
    document.getElementById('statTodayAppts').textContent = totalAppt;
    document.getElementById('statAvailableBeds').textContent = availB;
    document.getElementById('statBedRatio').textContent = `${occB} / ${totalB} occupied`;

    // Render Recent Patients Table (up to 5)
    const recentTable = document.getElementById('dashboardRecentPatientsTable');
    const recentPatients = [...appState.patients].slice(-5).reverse();
    if (recentPatients.length === 0) {
        recentTable.innerHTML = '<tr><td colspan="5" class="text-center text-muted">No patients registered yet.</td></tr>';
    } else {
        recentTable.innerHTML = recentPatients.map(p => `
            <tr>
                <td><strong>#${p.id}</strong></td>
                <td>${escapeHTML(p.name)}</td>
                <td>${p.age} yrs (${p.gender})</td>
                <td>${escapeHTML(p.disease)}</td>
                <td>
                    <button class="btn-action-icon" title="View Profile" onclick="viewPatientProfile(${p.id})">
                        <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><path d="M1 12s4-8 11-8 11 8 11 8-4 8-11 8-11-8-11-8z"></path><circle cx="12" cy="12" r="3"></circle></svg>
                    </button>
                </td>
            </tr>
        `).join('');
    }

    // Render Upcoming Appointments (up to 4)
    const upcomingContainer = document.getElementById('dashboardUpcomingAppts');
    const scheduledAppts = appState.appointments
        .filter(a => a.status === 'Scheduled')
        .slice(0, 4);

    if (scheduledAppts.length === 0) {
        upcomingContainer.innerHTML = '<p class="text-center text-muted">No scheduled appointments for today.</p>';
    } else {
        upcomingContainer.innerHTML = scheduledAppts.map(a => `
            <div class="appt-item-card">
                <div class="appt-item-info">
                    <h5>${escapeHTML(a.patientName)}</h5>
                    <p>with ${escapeHTML(a.doctorName)} • Reason: ${escapeHTML(a.reason)}</p>
                </div>
                <div class="appt-item-time">
                    ${escapeHTML(a.date)} ${escapeHTML(a.time)}
                </div>
            </div>
        `).join('');
    }
}

// ==============================================================
// 5. PATIENTS MODULE
// ==============================================================
function renderPatients(patientList = null) {
    const list = patientList || appState.patients;
    const tbody = document.getElementById('patientsTableBody');

    if (list.length === 0) {
        tbody.innerHTML = '<tr><td colspan="8" class="text-center text-muted" style="padding: 24px;">No patient records found.</td></tr>';
        return;
    }

    tbody.innerHTML = list.map(p => `
        <tr>
            <td><strong>#${p.id}</strong></td>
            <td><strong>${escapeHTML(p.name)}</strong></td>
            <td>${p.age}</td>
            <td>${p.gender}</td>
            <td><span class="badge badge-blood">${p.bloodGroup}</span></td>
            <td>${escapeHTML(p.disease)}</td>
            <td>${escapeHTML(p.phone)}</td>
            <td>
                <div class="table-actions">
                    <button class="btn-action-icon" title="View Patient" onclick="viewPatientProfile(${p.id})">
                        <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><path d="M1 12s4-8 11-8 11 8 11 8-4 8-11 8-11-8-11-8z"></path><circle cx="12" cy="12" r="3"></circle></svg>
                    </button>
                    <button class="btn-action-icon" title="Edit Patient" onclick="openEditPatientModal(${p.id})">
                        <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><path d="M11 4H4a2 2 0 0 0-2 2v14a2 2 0 0 0 2 2h14a2 2 0 0 0 2-2v-7"></path><path d="M18.5 2.5a2.121 2.121 0 0 1 3 3L12 15l-4 1 1-4 9.5-9.5z"></path></svg>
                    </button>
                    <button class="btn-action-icon btn-action-danger" title="Delete Patient" onclick="confirmDeletePatient(${p.id})">
                        <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><polyline points="3 6 5 6 21 6"></polyline><path d="M19 6v14a2 2 0 0 1-2 2H7a2 2 0 0 1-2-2V6m3 0V4a2 2 0 0 1 2-2h4a2 2 0 0 1 2 2v2"></path></svg>
                    </button>
                </div>
            </td>
        </tr>
    `).join('');
}

function filterPatients() {
    const search = document.getElementById('patientSearchInput').value.toLowerCase().trim();
    const blood = document.getElementById('patientBloodFilter').value;
    const sortBy = document.getElementById('patientSortBy').value;

    let filtered = appState.patients.filter(p => {
        const matchSearch = p.name.toLowerCase().includes(search) ||
                            p.phone.includes(search) ||
                            p.disease.toLowerCase().includes(search) ||
                            p.id.toString().includes(search);
        const matchBlood = !blood || p.bloodGroup === blood;
        return matchSearch && matchBlood;
    });

    // Sorting
    if (sortBy === 'id_asc') filtered.sort((a, b) => a.id - b.id);
    else if (sortBy === 'id_desc') filtered.sort((a, b) => b.id - a.id);
    else if (sortBy === 'name_asc') filtered.sort((a, b) => a.name.localeCompare(b.name));
    else if (sortBy === 'age_asc') filtered.sort((a, b) => a.age - b.age);
    else if (sortBy === 'age_desc') filtered.sort((a, b) => b.age - a.age);

    renderPatients(filtered);
}

function openAddPatientModal() {
    document.getElementById('patientForm').reset();
    document.getElementById('patientEditId').value = "0";
    document.getElementById('patientModalTitle').textContent = "Add New Patient";
    document.getElementById('btnSavePatient').textContent = "Save Patient";
    openModal('patientModal');
}

function openEditPatientModal(id) {
    const p = appState.patients.find(x => x.id === id);
    if (!p) return;

    document.getElementById('patientEditId').value = p.id;
    document.getElementById('pName').value = p.name;
    document.getElementById('pAge').value = p.age;
    document.getElementById('pGender').value = p.gender;
    document.getElementById('pBloodGroup').value = p.bloodGroup;
    document.getElementById('pPhone').value = p.phone;
    document.getElementById('pEmergency').value = p.emergencyContact;
    document.getElementById('pAddress').value = p.address;
    document.getElementById('pDisease').value = p.disease;

    document.getElementById('patientModalTitle').textContent = `Edit Patient #${p.id}`;
    document.getElementById('btnSavePatient').textContent = "Update Patient";
    openModal('patientModal');
}

function clearPatientForm() {
    document.getElementById('patientForm').reset();
}

async function savePatient(event) {
    event.preventDefault();
    const editId = parseInt(document.getElementById('patientEditId').value);
    const payload = {
        id: editId > 0 ? editId : 0,
        name: document.getElementById('pName').value.trim(),
        age: parseInt(document.getElementById('pAge').value),
        gender: document.getElementById('pGender').value,
        bloodGroup: document.getElementById('pBloodGroup').value,
        phone: document.getElementById('pPhone').value.trim(),
        emergencyContact: document.getElementById('pEmergency').value.trim(),
        address: document.getElementById('pAddress').value.trim(),
        disease: document.getElementById('pDisease').value.trim()
    };

    try {
        if (editId > 0) {
            await apiCall('/patients', 'PUT', payload);
            showToast('Patient updated successfully!', 'success');
        } else {
            await apiCall('/patients', 'POST', payload);
            showToast('Patient added successfully to C database!', 'success');
        }
        closeModal('patientModal');
        await fetchAllData();
    } catch (err) {
        showToast('Error saving patient: ' + err.message, 'error');
    }
}

function confirmDeletePatient(id) {
    const p = appState.patients.find(x => x.id === id);
    const name = p ? p.name : `#${id}`;
    openConfirmModal(
        'Delete Patient Record',
        `Are you sure you want to permanently delete patient "${name}"? This action modifies the patients.dat file.`,
        async () => {
            try {
                await apiCall(`/patients?id=${id}`, 'DELETE');
                showToast('Patient deleted successfully!', 'success');
                await fetchAllData();
            } catch (err) {
                showToast('Could not delete patient: ' + err.message, 'error');
            }
        }
    );
}

function viewPatientProfile(id) {
    const p = appState.patients.find(x => x.id === id);
    if (!p) return;

    const patientRecords = appState.records.filter(r => r.patientId === id);
    const patientPrescriptions = appState.prescriptions.filter(rx => rx.patientId === id);
    const patientBills = appState.bills.filter(b => b.patientId === id);

    const bodyEl = document.getElementById('patientProfileContent');
    bodyEl.innerHTML = `
        <div style="margin-bottom: 20px;">
            <div style="display: flex; align-items: center; justify-content: space-between; margin-bottom: 12px;">
                <h4 style="font-size: 18px; color: var(--secondary);">${escapeHTML(p.name)}</h4>
                <span class="tag-id">Patient ID: #${p.id}</span>
            </div>
            <div style="display: grid; grid-template-columns: repeat(auto-fit, minmax(200px, 1fr)); gap: 12px; font-size: 13px;">
                <div><strong>Age / Gender:</strong> ${p.age} years / ${p.gender}</div>
                <div><strong>Blood Group:</strong> <span class="badge badge-blood">${p.bloodGroup}</span></div>
                <div><strong>Phone Number:</strong> ${escapeHTML(p.phone)}</div>
                <div><strong>Emergency Contact:</strong> ${escapeHTML(p.emergencyContact)}</div>
                <div style="grid-column: 1 / -1;"><strong>Address:</strong> ${escapeHTML(p.address)}</div>
                <div style="grid-column: 1 / -1;"><strong>Primary Medical Problem:</strong> <span style="color: var(--primary-hover); font-weight: 600;">${escapeHTML(p.disease)}</span></div>
            </div>
        </div>

        <div style="border-top: 1px solid var(--border-color); padding-top: 14px; margin-top: 14px;">
            <h5 style="margin-bottom: 10px; font-size: 14px;">Quick Medical Summary</h5>
            <div style="display: flex; gap: 12px; font-size: 12px;">
                <span style="background: #f1f5f9; padding: 6px 12px; border-radius: 6px;">🩺 Medical Records: <strong>${patientRecords.length}</strong></span>
                <span style="background: #f1f5f9; padding: 6px 12px; border-radius: 6px;">💊 Prescriptions: <strong>${patientPrescriptions.length}</strong></span>
                <span style="background: #f1f5f9; padding: 6px 12px; border-radius: 6px;">🧾 Total Invoices: <strong>${patientBills.length}</strong></span>
            </div>
        </div>
    `;
    openModal('patientViewModal');
}

// ==============================================================
// 6. DOCTORS MODULE
// ==============================================================
function renderDoctors(doctorList = null) {
    const list = doctorList || appState.doctors;
    const grid = document.getElementById('doctorsGrid');

    if (list.length === 0) {
        grid.innerHTML = '<p class="text-center text-muted" style="grid-column: 1 / -1;">No doctors found.</p>';
        return;
    }

    grid.innerHTML = list.map(d => {
        const initials = d.name.replace('Dr.', '').trim().split(' ').map(n => n[0]).join('').substring(0, 2);
        return `
            <div class="doctor-card">
                <div class="doc-header">
                    <div class="doc-avatar">${initials}</div>
                    <div class="doc-title-wrap">
                        <h4>${escapeHTML(d.name)}</h4>
                        <span class="badge-specialization">${escapeHTML(d.specialization)}</span>
                    </div>
                </div>

                <div class="doc-details">
                    <div class="doc-detail-row">
                        <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><path d="M22 16.92v3a2 2 0 0 1-2.18 2 19.79 19.79 0 0 1-8.63-3.07 19.5 19.5 0 0 1-6-6 19.79 19.79 0 0 1-3.07-8.67A2 2 0 0 1 4.11 2h3a2 2 0 0 1 2 1.72 12.84 12.84 0 0 0 .7 2.81 2 2 0 0 1-.45 2.11L8.09 9.91a16 16 0 0 0 6 6l1.27-1.27a2 2 0 0 1 2.11-.45 12.84 12.84 0 0 0 2.81.7A2 2 0 0 1 22 16.92z"></path></svg>
                        <span>${escapeHTML(d.phone)}</span>
                    </div>
                    <div class="doc-detail-row">
                        <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><path d="M4 4h16c1.1 0 2 .9 2 2v12c0 1.1-.9 2-2 2H4c-1.1 0-2-.9-2-2V6c0-1.1.9-2 2-2z"></path><polyline points="22,6 12,13 2,6"></polyline></svg>
                        <span>${escapeHTML(d.email)}</span>
                    </div>
                    <div class="doc-detail-row">
                        <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><rect x="3" y="4" width="18" height="18" rx="2" ry="2"></rect><line x1="16" y1="2" x2="16" y2="6"></line><line x1="8" y1="2" x2="8" y2="6"></line></svg>
                        <span>${escapeHTML(d.availableDays)}</span>
                    </div>
                    <div class="doc-detail-row">
                        <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><circle cx="12" cy="12" r="10"></circle><polyline points="12 6 12 12 16 14"></polyline></svg>
                        <span>${escapeHTML(d.availableTime)}</span>
                    </div>
                </div>

                <div class="doc-actions">
                    <button class="btn btn-outline" style="flex: 1;" onclick="quickBookWithDoctor(${d.id})">
                        <span>Book Visit</span>
                    </button>
                    <button class="btn-action-icon btn-action-danger" title="Remove Doctor" onclick="confirmDeleteDoctor(${d.id})">
                        <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><polyline points="3 6 5 6 21 6"></polyline><path d="M19 6v14a2 2 0 0 1-2 2H7a2 2 0 0 1-2-2V6m3 0V4a2 2 0 0 1 2-2h4a2 2 0 0 1 2 2v2"></path></svg>
                    </button>
                </div>
            </div>
        `;
    }).join('');
}

function filterDoctors() {
    const search = document.getElementById('doctorSearchInput').value.toLowerCase().trim();
    const spec = document.getElementById('doctorSpecFilter').value;

    const filtered = appState.doctors.filter(d => {
        const matchSearch = d.name.toLowerCase().includes(search) || d.email.toLowerCase().includes(search);
        const matchSpec = !spec || d.specialization === spec;
        return matchSearch && matchSpec;
    });

    renderDoctors(filtered);
}

function openAddDoctorModal() {
    document.getElementById('doctorForm').reset();
    openModal('doctorModal');
}

async function saveDoctor(event) {
    event.preventDefault();
    const payload = {
        name: document.getElementById('dName').value.trim(),
        specialization: document.getElementById('dSpec').value,
        phone: document.getElementById('dPhone').value.trim(),
        email: document.getElementById('dEmail').value.trim(),
        availableDays: document.getElementById('dDays').value.trim(),
        availableTime: document.getElementById('dTime').value.trim()
    };

    try {
        await apiCall('/doctors', 'POST', payload);
        showToast('Doctor added successfully!', 'success');
        closeModal('doctorModal');
        await fetchAllData();
    } catch (err) {
        showToast('Error adding doctor: ' + err.message, 'error');
    }
}

function confirmDeleteDoctor(id) {
    const d = appState.doctors.find(x => x.id === id);
    const name = d ? d.name : `#${id}`;
    openConfirmModal(
        'Remove Doctor',
        `Are you sure you want to remove "${name}" from hospital records?`,
        async () => {
            try {
                await apiCall(`/doctors?id=${id}`, 'DELETE');
                showToast('Doctor removed successfully!', 'success');
                await fetchAllData();
            } catch (err) {
                showToast('Could not delete doctor: ' + err.message, 'error');
            }
        }
    );
}

// ==============================================================
// 7. APPOINTMENTS MODULE
// ==============================================================
function renderAppointments(apptList = null) {
    const list = apptList || appState.appointments;
    const tbody = document.getElementById('appointmentsTableBody');

    if (list.length === 0) {
        tbody.innerHTML = '<tr><td colspan="7" class="text-center text-muted" style="padding: 24px;">No appointments found.</td></tr>';
        return;
    }

    tbody.innerHTML = list.map(a => {
        let badgeClass = 'badge-scheduled';
        if (a.status === 'Completed') badgeClass = 'badge-completed';
        if (a.status === 'Cancelled') badgeClass = 'badge-cancelled';

        return `
            <tr>
                <td><strong>#${a.id}</strong></td>
                <td>
                    <strong>${escapeHTML(a.patientName)}</strong>
                    <div style="font-size: 11.5px; color: var(--text-muted);">Patient ID: #${a.patientId}</div>
                </td>
                <td>
                    <strong>${escapeHTML(a.doctorName)}</strong>
                    <div style="font-size: 11.5px; color: var(--text-muted);">Doctor ID: #${a.doctorId}</div>
                </td>
                <td>
                    <div>${escapeHTML(a.date)}</div>
                    <div style="font-size: 12px; color: var(--primary);">${escapeHTML(a.time)}</div>
                </td>
                <td>${escapeHTML(a.reason)}</td>
                <td><span class="badge ${badgeClass}">${a.status}</span></td>
                <td>
                    <div class="table-actions">
                        ${a.status === 'Scheduled' ? `
                            <button class="btn-action-icon" title="Mark as Completed" onclick="updateApptStatus(${a.id}, 'Completed')">
                                <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><polyline points="20 6 9 17 4 12"></polyline></svg>
                            </button>
                            <button class="btn-action-icon" title="Cancel Appointment" onclick="updateApptStatus(${a.id}, 'Cancelled')">
                                <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><line x1="18" y1="6" x2="6" y2="18"></line><line x1="6" y1="6" x2="18" y2="18"></line></svg>
                            </button>
                        ` : ''}
                        <button class="btn-action-icon btn-action-danger" title="Delete" onclick="deleteAppointment(${a.id})">
                            <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><polyline points="3 6 5 6 21 6"></polyline><path d="M19 6v14a2 2 0 0 1-2 2H7a2 2 0 0 1-2-2V6m3 0V4a2 2 0 0 1 2-2h4a2 2 0 0 1 2 2v2"></path></svg>
                        </button>
                    </div>
                </td>
            </tr>
        `;
    }).join('');
}

function filterAppointments() {
    const search = document.getElementById('apptSearchInput').value.toLowerCase().trim();
    const status = document.getElementById('apptStatusFilter').value;

    const filtered = appState.appointments.filter(a => {
        const matchSearch = a.patientName.toLowerCase().includes(search) ||
                            a.doctorName.toLowerCase().includes(search) ||
                            a.reason.toLowerCase().includes(search) ||
                            a.date.includes(search);
        const matchStatus = !status || a.status === status;
        return matchSearch && matchStatus;
    });

    renderAppointments(filtered);
}

function populateDropdownsForAppointments() {
    // Populate Patients Dropdown
    const pSelect = document.getElementById('appPatientSelect');
    pSelect.innerHTML = '<option value="">-- Choose Registered Patient --</option>' +
        appState.patients.map(p => `<option value="${p.id}">${p.name} (ID: #${p.id})</option>`).join('');

    // Populate Doctors Dropdown
    const dSelect = document.getElementById('appDoctorSelect');
    dSelect.innerHTML = '<option value="">-- Choose Available Doctor --</option>' +
        appState.doctors.map(d => `<option value="${d.id}">${d.name} (${d.specialization})</option>`).join('');
}

function openBookApptModal() {
    populateDropdownsForAppointments();
    document.getElementById('appointmentForm').reset();
    document.getElementById('appDate').value = new Date().toISOString().split('T')[0];
    document.getElementById('appTime').value = '10:00 AM';
    openModal('appointmentModal');
}

function quickBookWithDoctor(doctorId) {
    openBookApptModal();
    const dSelect = document.getElementById('appDoctorSelect');
    if (dSelect) dSelect.value = doctorId;
}

function handlePatientSelectChange() {
    // optional helper
}

async function saveAppointment(event) {
    event.preventDefault();
    const patientId = parseInt(document.getElementById('appPatientSelect').value);
    const doctorId = parseInt(document.getElementById('appDoctorSelect').value);

    const patient = appState.patients.find(p => p.id === patientId);
    const doctor = appState.doctors.find(d => d.id === doctorId);

    if (!patient || !doctor) {
        showToast('Please select valid patient and doctor', 'error');
        return;
    }

    const payload = {
        patientId: patientId,
        patientName: patient.name,
        doctorId: doctorId,
        doctorName: doctor.name,
        date: document.getElementById('appDate').value,
        time: document.getElementById('appTime').value.trim(),
        reason: document.getElementById('appReason').value.trim(),
        status: 'Scheduled'
    };

    try {
        await apiCall('/appointments', 'POST', payload);
        showToast('Appointment booked successfully!', 'success');
        closeModal('appointmentModal');
        await fetchAllData();
    } catch (err) {
        showToast('Booking failed: ' + err.message, 'error');
    }
}

async function updateApptStatus(id, newStatus) {
    try {
        await apiCall('/appointments', 'PUT', { id: id, status: newStatus });
        showToast(`Appointment marked as ${newStatus}!`, 'success');
        await fetchAllData();
    } catch (err) {
        showToast('Could not update status: ' + err.message, 'error');
    }
}

function deleteAppointment(id) {
    openConfirmModal(
        'Delete Appointment',
        `Are you sure you want to remove appointment #${id}?`,
        async () => {
            try {
                await apiCall(`/appointments?id=${id}`, 'DELETE');
                showToast('Appointment deleted!', 'success');
                await fetchAllData();
            } catch (err) {
                showToast('Error deleting appointment: ' + err.message, 'error');
            }
        }
    );
}

// ==============================================================
// 8. MEDICAL RECORDS MODULE
// ==============================================================
function renderRecords(recordList = null) {
    const list = recordList || appState.records;
    const container = document.getElementById('recordsList');

    if (list.length === 0) {
        container.innerHTML = '<p class="text-center text-muted" style="padding: 20px;">No medical records found.</p>';
        return;
    }

    container.innerHTML = list.map(r => {
        const patient = appState.patients.find(p => p.id === r.patientId);
        const doctor = appState.doctors.find(d => d.id === r.doctorId);
        const pName = patient ? patient.name : `Patient #${r.patientId}`;
        const dName = doctor ? doctor.name : `Doctor #${r.doctorId}`;

        return `
            <div class="record-card">
                <div class="record-header">
                    <div class="record-tags">
                        <span class="tag-id">Record #${r.id}</span>
                        <strong>${escapeHTML(pName)} (ID: #${r.patientId})</strong>
                    </div>
                    <span class="record-date">📅 ${escapeHTML(r.date)} • Attended by: <strong>${escapeHTML(dName)}</strong></span>
                </div>

                <div class="record-body">
                    <div class="record-field">
                        <label>Symptoms Reported</label>
                        <p>${escapeHTML(r.symptoms)}</p>
                    </div>
                    <div class="record-field">
                        <label>Clinical Diagnosis</label>
                        <p style="font-weight: 600; color: var(--primary-hover);">${escapeHTML(r.diagnosis)}</p>
                    </div>
                    <div class="record-field">
                        <label>Treatment / Prescribed Drugs</label>
                        <p>${escapeHTML(r.prescription)}</p>
                    </div>
                    <div class="record-field">
                        <label>Doctor Notes</label>
                        <p>${escapeHTML(r.notes || 'Routine follow-up')}</p>
                    </div>
                </div>
            </div>
        `;
    }).join('');
}

function filterRecords() {
    const searchId = parseInt(document.getElementById('recordPatientSearch').value);
    if (!searchId || isNaN(searchId)) {
        renderRecords(appState.records);
        return;
    }
    const filtered = appState.records.filter(r => r.patientId === searchId);
    renderRecords(filtered);
}

function openAddRecordModal() {
    const pSelect = document.getElementById('recPatientSelect');
    pSelect.innerHTML = '<option value="">-- Select Patient --</option>' +
        appState.patients.map(p => `<option value="${p.id}">${p.name} (ID: #${p.id})</option>`).join('');

    const dSelect = document.getElementById('recDoctorSelect');
    dSelect.innerHTML = '<option value="">-- Select Doctor --</option>' +
        appState.doctors.map(d => `<option value="${d.id}">${d.name} (${d.specialization})</option>`).join('');

    document.getElementById('recordForm').reset();
    document.getElementById('recDate').value = new Date().toISOString().split('T')[0];
    openModal('recordModal');
}

async function saveMedicalRecord(event) {
    event.preventDefault();
    const payload = {
        patientId: parseInt(document.getElementById('recPatientSelect').value),
        doctorId: parseInt(document.getElementById('recDoctorSelect').value),
        symptoms: document.getElementById('recSymptoms').value.trim(),
        diagnosis: document.getElementById('recDiagnosis').value.trim(),
        prescription: document.getElementById('recPrescription').value.trim(),
        date: document.getElementById('recDate').value,
        notes: document.getElementById('recNotes').value.trim()
    };

    try {
        await apiCall('/records', 'POST', payload);
        showToast('Medical record recorded in database!', 'success');
        closeModal('recordModal');
        await fetchAllData();
    } catch (err) {
        showToast('Error saving record: ' + err.message, 'error');
    }
}

// ==============================================================
// 9. PRESCRIPTIONS MODULE
// ==============================================================
function renderPrescriptions(rxList = null) {
    const list = rxList || appState.prescriptions;
    const grid = document.getElementById('prescriptionsGrid');

    if (list.length === 0) {
        grid.innerHTML = '<p class="text-center text-muted" style="grid-column: 1 / -1; padding: 20px;">No prescriptions found.</p>';
        return;
    }

    grid.innerHTML = list.map(rx => {
        const patient = appState.patients.find(p => p.id === rx.patientId);
        const doctor = appState.doctors.find(d => d.id === rx.doctorId);
        const pName = patient ? patient.name : `Patient #${rx.patientId}`;
        const dName = doctor ? doctor.name : `Doctor #${rx.doctorId}`;

        return `
            <div class="rx-card">
                <span class="rx-watermark">Rx</span>
                <div class="rx-header">
                    <span class="rx-badge">℞ Prescribed</span>
                    <span class="tag-id">Rx #${rx.id}</span>
                </div>

                <h4 class="rx-med-name">${escapeHTML(rx.medicineName)}</h4>

                <div class="rx-info-grid">
                    <div><strong>Patient:</strong> ${escapeHTML(pName)} (ID: #${rx.patientId})</div>
                    <div><strong>Dosage:</strong> ${escapeHTML(rx.dosage)}</div>
                    <div><strong>Frequency:</strong> ${escapeHTML(rx.frequency)}</div>
                    <div><strong>Duration:</strong> ${escapeHTML(rx.duration)}</div>
                    <div><strong>Prescribed By:</strong> ${escapeHTML(dName)}</div>
                </div>

                <div class="rx-instructions">
                    <strong>Instructions:</strong> ${escapeHTML(rx.instructions)}
                </div>

                <div class="rx-footer">
                    <button class="btn btn-outline" style="font-size: 12px; padding: 6px 12px;" onclick="printSinglePrescription(${rx.id})">
                        <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" style="width: 14px; height: 14px;"><polyline points="6 9 6 2 18 2 18 9"></polyline><path d="M6 18H4a2 2 0 0 1-2-2v-5a2 2 0 0 1 2-2h16a2 2 0 0 1 2 2v5a2 2 0 0 1-2 2h-2"></path><rect x="6" y="14" width="12" height="8"></rect></svg>
                        <span>Print</span>
                    </button>
                    <button class="btn-action-icon btn-action-danger" title="Delete Prescription" onclick="deletePrescription(${rx.id})">
                        <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><polyline points="3 6 5 6 21 6"></polyline><path d="M19 6v14a2 2 0 0 1-2 2H7a2 2 0 0 1-2-2V6m3 0V4a2 2 0 0 1 2-2h4a2 2 0 0 1 2 2v2"></path></svg>
                    </button>
                </div>
            </div>
        `;
    }).join('');
}

function filterPrescriptions() {
    const searchId = parseInt(document.getElementById('prescriptionPatientSearch').value);
    if (!searchId || isNaN(searchId)) {
        renderPrescriptions(appState.prescriptions);
        return;
    }
    const filtered = appState.prescriptions.filter(rx => rx.patientId === searchId);
    renderPrescriptions(filtered);
}

function openAddPrescriptionModal() {
    const pSelect = document.getElementById('rxPatientSelect');
    pSelect.innerHTML = '<option value="">-- Select Patient --</option>' +
        appState.patients.map(p => `<option value="${p.id}">${p.name} (ID: #${p.id})</option>`).join('');

    const dSelect = document.getElementById('rxDoctorSelect');
    dSelect.innerHTML = '<option value="">-- Select Doctor --</option>' +
        appState.doctors.map(d => `<option value="${d.id}">${d.name} (${d.specialization})</option>`).join('');

    document.getElementById('prescriptionForm').reset();
    openModal('prescriptionModal');
}

async function savePrescription(event) {
    event.preventDefault();
    const payload = {
        patientId: parseInt(document.getElementById('rxPatientSelect').value),
        doctorId: parseInt(document.getElementById('rxDoctorSelect').value),
        medicineName: document.getElementById('rxMedicine').value.trim(),
        dosage: document.getElementById('rxDosage').value.trim(),
        frequency: document.getElementById('rxFrequency').value.trim(),
        duration: document.getElementById('rxDuration').value.trim(),
        instructions: document.getElementById('rxInstructions').value.trim()
    };

    try {
        await apiCall('/prescriptions', 'POST', payload);
        showToast('Prescription saved to database!', 'success');
        closeModal('prescriptionModal');
        await fetchAllData();
    } catch (err) {
        showToast('Error creating prescription: ' + err.message, 'error');
    }
}

function deletePrescription(id) {
    openConfirmModal(
        'Delete Prescription',
        `Are you sure you want to remove prescription #${id}?`,
        async () => {
            try {
                await apiCall(`/prescriptions?id=${id}`, 'DELETE');
                showToast('Prescription deleted!', 'success');
                await fetchAllData();
            } catch (err) {
                showToast('Error deleting prescription: ' + err.message, 'error');
            }
        }
    );
}

function printSinglePrescription(rxId) {
    const rx = appState.prescriptions.find(x => x.id === rxId);
    if (!rx) return;
    const patient = appState.patients.find(p => p.id === rx.patientId);
    const doctor = appState.doctors.find(d => d.id === rx.doctorId);

    const win = window.open('', '_blank', 'width=700,height=600');
    win.document.write(`
        <html>
        <head>
            <title>MediCare Prescription #${rx.id}</title>
            <style>
                body { font-family: sans-serif; padding: 40px; color: #333; }
                .header { border-bottom: 2px solid #0284c7; padding-bottom: 15px; margin-bottom: 25px; }
                .rx-symbol { font-size: 32px; font-weight: bold; color: #0284c7; margin: 20px 0; }
                .med-name { font-size: 20px; font-weight: bold; margin-bottom: 10px; }
                .detail-line { margin: 8px 0; font-size: 14px; }
                .footer { margin-top: 50px; border-top: 1px solid #ccc; padding-top: 20px; text-align: right; }
            </style>
        </head>
        <body>
            <div class="header">
                <h2>MediCare Hospital & Research Centre</h2>
                <p>Medical Prescription Slip • Rx #${rx.id}</p>
            </div>
            <div>
                <p class="detail-line"><strong>Patient Name:</strong> ${patient ? patient.name : 'ID #' + rx.patientId}</p>
                <p class="detail-line"><strong>Prescribing Doctor:</strong> ${doctor ? doctor.name : 'ID #' + rx.doctorId}</p>
                <p class="detail-line"><strong>Date:</strong> ${new Date().toLocaleDateString()}</p>
            </div>
            <div class="rx-symbol">℞</div>
            <div class="med-name">${escapeHTML(rx.medicineName)} (${escapeHTML(rx.dosage)})</div>
            <p class="detail-line"><strong>Frequency:</strong> ${escapeHTML(rx.frequency)}</p>
            <p class="detail-line"><strong>Duration:</strong> ${escapeHTML(rx.duration)}</p>
            <p class="detail-line"><strong>Instructions:</strong> ${escapeHTML(rx.instructions)}</p>
            <div class="footer">
                <p>Doctor's Authorized Signature: _________________________</p>
            </div>
            <script>window.print();</script>
        </body>
        </html>
    `);
    win.document.close();
}

// ==============================================================
// 10. BILLING MODULE
// ==============================================================
function renderBills(billList = null) {
    const list = billList || appState.bills;
    const tbody = document.getElementById('billingTableBody');

    if (list.length === 0) {
        tbody.innerHTML = '<tr><td colspan="9" class="text-center text-muted" style="padding: 24px;">No hospital bills generated yet.</td></tr>';
        return;
    }

    tbody.innerHTML = list.map(b => {
        const patient = appState.patients.find(p => p.id === b.patientId);
        const pName = patient ? patient.name : `Patient #${b.patientId}`;
        const isPaid = b.paymentStatus === 'Paid';

        return `
            <tr>
                <td><strong>#${b.id}</strong></td>
                <td>
                    <strong>${escapeHTML(pName)}</strong>
                    <div style="font-size: 11.5px; color: var(--text-muted);">Patient ID: #${b.patientId}</div>
                </td>
                <td>₹ ${b.consultationFee.toFixed(2)}</td>
                <td>₹ ${b.medicineCharges.toFixed(2)}</td>
                <td>₹ ${b.testCharges.toFixed(2)}</td>
                <td>₹ ${b.otherCharges.toFixed(2)}</td>
                <td><strong style="color: var(--secondary); font-size: 14px;">₹ ${b.totalAmount.toFixed(2)}</strong></td>
                <td><span class="badge ${isPaid ? 'badge-paid' : 'badge-pending'}">${b.paymentStatus}</span></td>
                <td>
                    <div class="table-actions">
                        <button class="btn-action-icon" title="View Printable Invoice" onclick="viewInvoice(${b.id})">
                            <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><path d="M14 2H6a2 2 0 0 0-2 2v16a2 2 0 0 0 2 2h12a2 2 0 0 0 2-2V8z"></path><polyline points="14 2 14 8 20 8"></polyline><line x1="16" y1="13" x2="8" y2="13"></line><line x1="16" y1="17" x2="8" y2="17"></line></svg>
                        </button>
                        <button class="btn btn-outline" style="font-size: 11.5px; padding: 4px 8px;" onclick="toggleBillStatus(${b.id}, '${isPaid ? 'Pending' : 'Paid'}')">
                            Mark ${isPaid ? 'Pending' : 'Paid'}
                        </button>
                    </div>
                </td>
            </tr>
        `;
    }).join('');
}

function filterBills() {
    const search = document.getElementById('billSearchInput').value.trim();
    const status = document.getElementById('billStatusFilter').value;

    const filtered = appState.bills.filter(b => {
        const matchSearch = !search ||
                            b.id.toString() === search ||
                            b.patientId.toString() === search;
        const matchStatus = !status || b.paymentStatus === status;
        return matchSearch && matchStatus;
    });

    renderBills(filtered);
}

function calculateBillTotal() {
    const fee = parseFloat(document.getElementById('billConsultation').value) || 0;
    const med = parseFloat(document.getElementById('billMedicine').value) || 0;
    const test = parseFloat(document.getElementById('billTests').value) || 0;
    const other = parseFloat(document.getElementById('billOther').value) || 0;
    const total = fee + med + test + other;

    document.getElementById('billCalculatedTotal').textContent = `₹ ${total.toFixed(2)}`;
    return total;
}

function openGenerateBillModal() {
    const pSelect = document.getElementById('billPatientSelect');
    pSelect.innerHTML = '<option value="">-- Select Patient --</option>' +
        appState.patients.map(p => `<option value="${p.id}">${p.name} (ID: #${p.id})</option>`).join('');

    document.getElementById('billForm').reset();
    document.getElementById('billConsultation').value = "500";
    document.getElementById('billMedicine').value = "0";
    document.getElementById('billTests').value = "0";
    document.getElementById('billOther').value = "0";
    document.getElementById('billStatus').value = "Paid";
    calculateBillTotal();

    openModal('billModal');
}

async function saveBill(event) {
    event.preventDefault();
    const payload = {
        patientId: parseInt(document.getElementById('billPatientSelect').value),
        consultationFee: parseFloat(document.getElementById('billConsultation').value) || 0,
        medicineCharges: parseFloat(document.getElementById('billMedicine').value) || 0,
        testCharges: parseFloat(document.getElementById('billTests').value) || 0,
        otherCharges: parseFloat(document.getElementById('billOther').value) || 0,
        paymentStatus: document.getElementById('billStatus').value
    };

    try {
        await apiCall('/bills', 'POST', payload);
        showToast('Hospital bill generated successfully!', 'success');
        closeModal('billModal');
        await fetchAllData();
    } catch (err) {
        showToast('Error generating bill: ' + err.message, 'error');
    }
}

async function toggleBillStatus(id, newStatus) {
    try {
        await apiCall('/bills', 'PUT', { id: id, paymentStatus: newStatus });
        showToast(`Bill marked as ${newStatus}!`, 'success');
        await fetchAllData();
    } catch (err) {
        showToast('Error updating status: ' + err.message, 'error');
    }
}

function viewInvoice(id) {
    const bill = appState.bills.find(b => b.id === id);
    if (!bill) return;
    const patient = appState.patients.find(p => p.id === bill.patientId);

    const invoiceContent = document.getElementById('invoiceContent');
    invoiceContent.innerHTML = `
        <div class="invoice-header">
            <div class="invoice-brand">
                <h2>MediCare Hospital</h2>
                <p>Advanced Medical Healthcare Centre</p>
                <p>Phone: +91 9876543210 • Email: billing@medicare.org</p>
            </div>
            <div class="invoice-meta">
                <h4>INVOICE #${bill.id}</h4>
                <p>Date: ${new Date().toLocaleDateString()}</p>
                <p>Status: <strong style="color: ${bill.paymentStatus === 'Paid' ? '#10b981' : '#f59e0b'};">${bill.paymentStatus}</strong></p>
            </div>
        </div>

        <div class="invoice-patient-box">
            <p><strong>Billed To:</strong> ${patient ? escapeHTML(patient.name) : 'Patient #' + bill.patientId}</p>
            <p><strong>Patient ID:</strong> #${bill.patientId}</p>
            ${patient ? `<p><strong>Address:</strong> ${escapeHTML(patient.address)} | <strong>Phone:</strong> ${escapeHTML(patient.phone)}</p>` : ''}
        </div>

        <table class="invoice-table">
            <thead>
                <tr>
                    <th>Description</th>
                    <th style="text-align: right;">Amount (₹)</th>
                </tr>
            </thead>
            <tbody>
                <tr>
                    <td>Doctor Consultation Fee</td>
                    <td style="text-align: right;">₹ ${bill.consultationFee.toFixed(2)}</td>
                </tr>
                <tr>
                    <td>Pharmacy / Medication Charges</td>
                    <td style="text-align: right;">₹ ${bill.medicineCharges.toFixed(2)}</td>
                </tr>
                <tr>
                    <td>Diagnostic & Laboratory Tests</td>
                    <td style="text-align: right;">₹ ${bill.testCharges.toFixed(2)}</td>
                </tr>
                <tr>
                    <td>Hospital Services & Other Charges</td>
                    <td style="text-align: right;">₹ ${bill.otherCharges.toFixed(2)}</td>
                </tr>
                <tr class="invoice-total-row">
                    <td><strong>TOTAL AMOUNT PAYABLE</strong></td>
                    <td style="text-align: right;"><strong>₹ ${bill.totalAmount.toFixed(2)}</strong></td>
                </tr>
            </tbody>
        </table>

        <div style="font-size: 11.5px; color: var(--text-muted); text-align: center; margin-top: 15px;">
            Thank you for choosing MediCare. For billing queries, contact our finance desk.
        </div>
    `;

    openModal('invoiceModal');
}

// ==============================================================
// 11. BED MANAGEMENT MODULE
// ==============================================================
function renderBeds() {
    const list = appState.beds;
    const total = list.length;
    const avail = list.filter(b => b.status === 'Available').length;
    const occ = total - avail;

    document.getElementById('bedStatTotal').textContent = total;
    document.getElementById('bedStatAvailable').textContent = avail;
    document.getElementById('bedStatOccupied').textContent = occ;

    const grid = document.getElementById('bedsGrid');
    if (total === 0) {
        grid.innerHTML = '<p class="text-center text-muted" style="grid-column: 1 / -1;">No beds configured.</p>';
        return;
    }

    grid.innerHTML = list.map(b => {
        const isOccupied = b.status === 'Occupied';
        const patient = isOccupied ? appState.patients.find(p => p.id === b.patientId) : null;
        const pName = patient ? patient.name : (b.patientId > 0 ? `Patient #${b.patientId}` : '');

        return `
            <div class="bed-card ${isOccupied ? 'occupied' : 'available'}">
                <div class="bed-card-header">
                    <span class="bed-number">${escapeHTML(b.bedNumber)}</span>
                    <span class="dot ${isOccupied ? 'red' : 'green'}"></span>
                </div>
                <div class="bed-ward">${escapeHTML(b.ward)}</div>

                <div class="bed-patient-info">
                    ${isOccupied ? `
                        <div style="color: #991b1b; font-weight: 600;">Admitted:</div>
                        <div style="font-size: 13px;">${escapeHTML(pName)}</div>
                    ` : `
                        <div style="color: #065f46; font-weight: 600;">Bed Available</div>
                        <div style="font-size: 11px; color: var(--text-muted);">Ready for admission</div>
                    `}
                </div>

                <div style="margin-top: 6px;">
                    ${isOccupied ? `
                        <button class="btn btn-outline btn-block" style="font-size: 12px; padding: 6px;" onclick="confirmReleaseBed(${b.id}, '${escapeHTML(b.bedNumber)}')">
                            Release Bed
                        </button>
                    ` : `
                        <button class="btn btn-primary btn-block" style="font-size: 12px; padding: 6px;" onclick="openAssignBedModal(${b.id}, '${escapeHTML(b.ward)}', '${escapeHTML(b.bedNumber)}')">
                            Assign Patient
                        </button>
                    `}
                </div>
            </div>
        `;
    }).join('');
}

function openAssignBedModal(bedId, ward, bedNumber) {
    document.getElementById('assignBedId').value = bedId;
    document.getElementById('assignBedDetails').textContent = `Ward: ${ward} | Bed: ${bedNumber}`;

    const pSelect = document.getElementById('assignPatientSelect');
    pSelect.innerHTML = '<option value="">-- Choose Patient --</option>' +
        appState.patients.map(p => `<option value="${p.id}">${p.name} (ID: #${p.id})</option>`).join('');

    openModal('assignBedModal');
}

async function submitAssignBed(event) {
    event.preventDefault();
    const bedId = parseInt(document.getElementById('assignBedId').value);
    const patientId = parseInt(document.getElementById('assignPatientSelect').value);

    try {
        await apiCall('/beds', 'POST', {
            action: 'assign',
            bedId: bedId,
            patientId: patientId
        });
        showToast('Bed assigned to patient successfully!', 'success');
        closeModal('assignBedModal');
        await fetchAllData();
    } catch (err) {
        showToast('Could not assign bed: ' + err.message, 'error');
    }
}

function confirmReleaseBed(bedId, bedNumber) {
    openConfirmModal(
        'Release Bed',
        `Are you sure you want to discharge the patient and mark bed ${bedNumber} as Available?`,
        async () => {
            try {
                await apiCall('/beds', 'POST', {
                    action: 'release',
                    bedId: bedId
                });
                showToast(`Bed ${bedNumber} released and is now Available!`, 'success');
                await fetchAllData();
            } catch (err) {
                showToast('Failed to release bed: ' + err.message, 'error');
            }
        }
    );
}

// ==============================================================
// 12. MODAL & CONFIRMATION HELPERS
// ==============================================================
function openModal(modalId) {
    const modal = document.getElementById(modalId);
    if (modal) {
        modal.style.display = 'flex';
    }
}

function closeModal(modalId) {
    const modal = document.getElementById(modalId);
    if (modal) {
        modal.style.display = 'none';
    }
}

// Global modal background click to close
window.onclick = function(event) {
    if (event.target.classList.contains('modal-overlay')) {
        event.target.style.display = 'none';
    }
};

// Generic Confirmation Modal
function openConfirmModal(title, message, onConfirmCallback) {
    document.getElementById('confirmTitle').textContent = title;
    document.getElementById('confirmMessage').textContent = message;
    const btn = document.getElementById('confirmBtnAction');

    // Replace click listener
    btn.onclick = async () => {
        closeModal('confirmModal');
        if (typeof onConfirmCallback === 'function') {
            await onConfirmCallback();
        }
    };

    openModal('confirmModal');
}

// ==============================================================
// 13. TOAST NOTIFICATIONS
// ==============================================================
function showToast(message, type = 'success') {
    const container = document.getElementById('toastContainer');
    if (!container) return;

    const toast = document.createElement('div');
    toast.className = `toast toast-${type}`;

    let iconSvg = '';
    if (type === 'success') {
        iconSvg = '<svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" style="width: 18px; height: 18px; color: var(--success);"><polyline points="20 6 9 17 4 12"></polyline></svg>';
    } else if (type === 'error') {
        iconSvg = '<svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" style="width: 18px; height: 18px; color: var(--danger);"><circle cx="12" cy="12" r="10"></circle><line x1="15" y1="9" x2="9" y2="15"></line><line x1="9" y1="9" x2="15" y2="15"></line></svg>';
    } else {
        iconSvg = '<svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" style="width: 18px; height: 18px; color: var(--primary);"><circle cx="12" cy="12" r="10"></circle><line x1="12" y1="16" x2="12" y2="12"></line><line x1="12" y1="8" x2="12.01" y2="8"></line></svg>';
    }

    toast.innerHTML = `${iconSvg}<span>${escapeHTML(message)}</span>`;
    container.appendChild(toast);

    setTimeout(() => {
        toast.style.opacity = '0';
        toast.style.transform = 'translateX(50px)';
        toast.style.transition = 'all 0.3s ease';
        setTimeout(() => toast.remove(), 300);
    }, 3500);
}

// Helper to escape HTML characters to prevent XSS
function escapeHTML(str) {
    if (!str) return '';
    return str
        .replace(/&/g, '&amp;')
        .replace(/</g, '&lt;')
        .replace(/>/g, '&gt;')
        .replace(/"/g, '&quot;')
        .replace(/'/g, '&#039;');
}
