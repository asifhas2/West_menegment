# Community Smart Waste Management System (C-WMS)
A Modular, Console-Based Waste Management & Transparency Platform written in standard **C**.

---

## 📌 Architecture & Features

The system connects three primary municipal actors through persistent file storage:

### 1. Citizen / User Portal
- **Account Registration & Login**: Validates credentials and ensures unique citizen email and phone numbers.
- **Central Citizen Dashboard**: Displays active balance, real-time request counts, and operations.
- **Submit Waste Collection Request**: 7 waste categories (Plastic, Organic, Paper, Metal, Glass, E-Waste, Chemical), estimated weight, pickup location/Smart Bin, and preferred time window.
- **Confirmation Step**: Complete summary review before final submission.
- **6-Stage Lifecycle Tracking**: Live progress bar:
  `[1. Pending] ➔ [2. Approved] ➔ [3. Assigned] ➔ [4. In Progress] ➔ [5. Collected] ➔ [6. Final Outcome Recorded]`
- **Report Public Bin Problem**: Report overflowing, damaged, or hazardous public bins directly to dispatchers.
- **Verified Collection History**: Review completed pick-ups with actual measured scale weights.
- **Reward Points & History**: Real-time points balance and transaction ledger.
- **Waste Outcome Transparency Trail**: Follow collected waste to its final ecological destination (recycled, composted, or safely treated).

### 2. Waste Collector Portal
- **Collector Login**: Authentication for municipal field crew.
- **Field Dashboard**: View active shift duties, assigned vehicle, and total haul collected.
- **View Assigned Tasks & Waypoints**: Daily pickup schedule with street addresses and notes.
- **Start Collection**: Marks task as *In Progress* to alert citizens that crew is on-site or en-route.
- **Weigh & Complete Collection**: Records physical scale weight, updates vehicle load, updates request to *Collected*, and automatically calculates and deposits reward points into the citizen's account.
- **Field Incident Reporting**: Log route impediments (road blockage, contaminated waste, absent resident).
- **Performance Summary**: Total collections completed and vehicle capacity utilization.

### 3. Municipal Administrator Portal
- **Real-Time Operations Dashboard**: High-level metrics on citizens, collectors, fleet, and pending batches.
- **Request Management**: Review, search, approve, or reject incoming collection requests.
- **Resource Assignment**: Assign available field collectors and fleet vehicles to approved requests.
- **Route Manifest Generation**: Create and dispatch daily collection routes.
- **Waste Processing Outcomes**: Record certified recycling outcomes or authorized chemical waste treatment and manifests.
- **Commercial Recyclable Sales**: Log bulk material sales to recycling and industrial partners.
- **End-to-End Transparency Trail**: Full audit trail from citizen origin to final product/disposal.
- **Entity Management**: CRUD operations for citizens, public smart bins, collectors, and trucks.
- **Incident Management**: Review and resolve problem reports from both citizens and collectors.
- **Statistical Analytics & Reports**: Material volume breakdown, rewards ledger, collector rankings, and fleet load reports.

---

## 🚀 How to Build & Run

### Quick Build (Windows)
```cmd
build.bat
```
Or directly with GCC:
```cmd
gcc -Wall -Wextra -Wl,--stack,16777216 -Iheader_file main_c_code/main.c main_c_code/user.c main_c_code/collector.c main_c_code/admin.c main_c_code/data.c main_c_code/utils.c -o waste_management.exe
```

### Run
```cmd
waste_management.exe
```

---

## 🔑 Account Authentication & Registration for All Actors

All three portals now support **full user registration** and **real credential authentication** backed by persistent data storage:

1. **Citizen Portal**:
   - `[1] Citizen Login` (Log in with registered email or phone and password)
   - `[2] Citizen Registration` (Create new account saved directly to `data/users.txt`)

2. **Waste Collector Portal**:
   - `[1] Collector Login` (Log in with Collector ID or email and password from `data/collectors.txt`)
   - `[2] Collector Registration` (Join collection crew, choose assigned truck, generates `COLxxx` ID)

3. **Municipal Admin Portal**:
   - `[1] Administrator Login` (Log in with admin username/email and password from `data/admins.txt`)
   - `[2] Register New Administrator` (Authorized staff onboarding using Municipal Master Key: `admin123`)

### Pre-configured Accounts (Ready to Test)

| Role | Username / ID / Email | Password | Details |
|---|---|---|---|
| **Citizen** | `user@test.com` or `01711223344` | `password123` | Rahim Ahmed |
| **Collector** | `COL101` or `karim@collector.com` | `collector123` | Karim Ullah (Truck TRK-01) |
| **Collector** | `COL102` or `hasan@collector.com` | `collector123` | Hasan Mahmud (Truck TRK-03) |
| **Admin** | `admin` | `admin123` | System Administrator (Super Admin) |
| **Admin** | `manager` | `admin123` | Operations Director |

*(Master Authorization Key for onboarding new Admins: `admin123`)*

---

## 📂 Project Directory Structure

```
West_menegment_v3/
├── build.bat                  # Automated compilation script
├── README.md                  # System overview and manual
├── header_file/               # Header files (.h)
│   ├── admin.h                # Admin portal interface
│   ├── collector.h            # Collector portal interface
│   ├── common.h               # Core definitions, limits, and file paths
│   ├── data.h                 # File persistence declarations
│   ├── models.h               # Data structures (User, WasteRequest, SmartBin, Admin, etc.)
│   ├── user.h                 # Citizen portal interface
│   └── utils.h                # Input validation, screen handling, UI formatters
├── main_c_code/               # Source files (.c)
│   ├── admin.c                # Admin management, approval, fleet, reports
│   ├── collector.c            # Field operations, weigh-in, reward deposits
│   ├── data.c                 # File I/O operations and database initialization
│   ├── main.c                 # Main menu and portal routing
│   ├── user.c                 # Citizen portal, request submission, tracking
│   └── utils.c                # Safe fgets input, screen handling, progress bars
└── data/                      # Persistent flat files
    ├── admins.txt             # Registered municipal administrator accounts
    ├── bin_reports.txt        # Public bin reports
    ├── bins.txt               # Smart bins database
    ├── collector_reports.txt  # Field incident reports
    ├── collectors.txt         # Collector crew records
    ├── requests.txt           # Waste collection requests and status
    ├── rewards.txt            # Reward points transaction ledger
    ├── sales.txt              # Commercial recyclable sales
    ├── trucks.txt             # Municipal fleet database
    └── users.txt              # Registered citizen accounts
```
