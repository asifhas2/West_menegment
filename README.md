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
- **Eco-Store (Rewards Redemption)**: Spend earned recycling points to purchase high-quality eco-friendly items (Stainless Steel Water Bottle, Small Household Dustbin, Antibacterial Handwash, Recycled Paper Tissues, Biodegradable Trash Bags, Cotton Totes, Rubber Cleaning Gloves, and Organic Dish Soap).
- **Eco-Store Purchase History**: View past orders, delivery addresses, items received, and dispatch status.
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
- **Real-Time Operations Dashboard**: Displays logged-in Admin Points, Total Sales Revenue, Warehouse status (`Collected`, `Recycled`, `Finalized`), and system metrics.
- **Request Management**: Review, search, approve, or reject incoming collection requests.
- **Resource Assignment**: Assign available field collectors and fleet vehicles to approved requests.
- **Route Manifest Generation**: Create and dispatch daily collection routes.
- **Admin Warehouse Pipeline (Collected ➔ Recycle ➔ Sell ➔ Admin Points)**:
  1. **Automatic Collector Handoff**: When a collector finishes a pickup, the verified waste batch (`BATxxxx`) is automatically logged into the Admin Warehouse (`data/warehouse.txt`).
  2. **Step 1 – Recycle / Process Waste**: Admin processes collected batches into sellable recycled materials (or certifies safe hazardous chemical disposal).
  3. **Step 2 – Sell Recycled Waste & Claim Admin Points**: Admin selects recycled batches from warehouse stock and sells them to commercial buyers. **Net Revenue and Admin Points** are calculated automatically using the rule: `Net Revenue = Admin Sale Amount - User Earned Reward Coins`. This net profit is directly credited to the Administrator's Wallet and Total Revenue.
  4. **Warehouse Inventory & Points Ledger**: Full audit of all batches across stages along with Admin Points & Net Revenue totals.
- **End-to-End Transparency Trail**: Full audit trail from citizen origin to final recycling & commercial sale.
- **Eco-Store Orders & Dispatch Management**: Review citizen reward orders, track delivery addresses, update order status (`Processing in Warehouse`, `Dispatched for Delivery`, `Delivered to Citizen`), or cancel and refund points.
- **Entity Management**: CRUD operations for citizens, public smart bins, collectors, and trucks.
- **Incident Management**: Review and resolve problem reports from both citizens and collectors.
- **Statistical Analytics & Reports**: Material volume breakdown, citizen rewards ledger, collector rankings, fleet load reports, Admin Points & Commercial Sales Net Revenue report, and Eco-Store Redemption report.
- **AI Environmental & Waste Management Assistant (EcoBot)**: Access municipal AI capabilities directly from the Admin console.

### 4. AI Environmental & Waste Assistant (EcoBot - Local Qwen 1.7B)
- **Offline & Private AI Inference**: Powered by `llama.cpp` and `models/Qwen3-1.7B-Q4_K_M.gguf` stored locally in the `AI/` directory.
- **Custom System Prompt**: Initialized with municipal domain knowledge (`AI/system_prompt.txt`), covering sorting procedures for all 7 waste categories, point calculation rules, store items, and chemical waste hazard protocols.
- **Interactive Live Chat Mode**: Open-ended back-and-forth dialogue with the AI; supports clean exit with `/exit` returning directly to the C console.
- **Single Question & Answer Mode**: Quickly ask an ad-hoc question from the console without starting a full session (uses prompt buffer `AI/user_prompt.txt` to eliminate command-line character escape glitches).
- **Quick-Access Municipal Waste Guide**: Instant offline reference answers for common waste segregation queries.
- **Diagnostics & Status**: Verifies local binary and model weights paths and confirms runtime health.

---

## 🚀 How to Build & Run

### Quick Build (Windows)
```cmd
build.bat
```
Or directly with GCC:
```cmd
gcc -Wall -Wextra -Wl,--stack,16777216 -Iheader_file main_c_code/main.c main_c_code/user.c main_c_code/collector.c main_c_code/admin.c main_c_code/ai_assistant.c main_c_code/data.c main_c_code/utils.c -o waste_management.exe
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

### Active Accounts (Cleaned & Ready to Test)

| Role | Username / ID / Email | Password | Details |
|---|---|---|---|
| **Citizen** | `ak1247942@gmail.com` or `01810401376` | `12345678` | Md Asif Hasan |
| **Citizen** | `Mirpur10` or `01810401377` | `1234` | sune sinha |
| **Collector** | `COL104` or `rafiq@collector.com` | `pass1234` | Rafiqul Islam (Truck TRK-02) |
| **Collector** | `COL105` or `sune@12gmail.com` | `1234` | sune (Truck TRK-01) |
| **Collector** | `COL106` or `sinhasonia97@gmail.com` | `1234` | sonia (Truck TRK-02) |
| **Admin** | `ADM102` or `manager` | `admin123` | Operations Director |
| **Admin** | `ADM103` or `farhana@city.gov` | `adminpass123` | Farhana Yasmin |
| **Admin** | `ADM104` or `shoybe@gmail.com` | `1234` | shoybe |
| **Admin** | `ADM105` or `shoyeb6004@gmail.com` | `1234` | Md Shoybe hasan |

*(Master Authorization Key for onboarding new Admins: `admin123`)*

---

## 📂 Project Directory Structure

```
West_menegment_v3/
├── build.bat                  # Automated compilation script
├── README.md                  # System overview and manual
├── header_file/               # Header files (.h)
│   ├── admin.h                # Admin portal interface
│   ├── ai_assistant.h         # Local AI assistant interface (Qwen 1.7B / llama.cpp)
│   ├── collector.h            # Collector portal interface
│   ├── common.h               # Core definitions, limits, and file paths
│   ├── data.h                 # File persistence declarations
│   ├── models.h               # Data structures (User, WasteRequest, SmartBin, Admin, etc.)
│   ├── user.h                 # Citizen portal interface
│   └── utils.h                # Input validation, screen handling, UI formatters
├── main_c_code/               # Source files (.c)
│   ├── admin.c                # Admin management, approval, fleet, reports
│   ├── ai_assistant.c         # AI assistant integration (chat, single question, diagnostics)
│   ├── collector.c            # Field operations, weigh-in, reward deposits
│   ├── data.c                 # File I/O operations and database initialization
│   ├── main.c                 # Main menu and portal routing
│   ├── user.c                 # Citizen portal, request submission, tracking
│   └── utils.c                # Safe fgets input, screen handling, progress bars
├── AI/                        # Local AI Model & llama.cpp Environment
│   ├── system_prompt.txt      # Customized domain system prompt for waste management
│   ├── user_prompt.txt        # Safe temporary buffer for user prompt queries
│   └── llama.cpp/             # Compiled llama.cpp runtime
│       ├── build/bin/         # llama-cli.exe executable
│       └── models/            # Qwen3-1.7B-Q4_K_M.gguf quantized model
└── data/                      # Persistent flat files
    ├── admins.txt             # Registered municipal administrator accounts
    ├── bin_reports.txt        # Public bin reports
    ├── bins.txt               # Smart bins database
    ├── collector_reports.txt  # Field incident reports
    ├── collectors.txt         # Collector crew records
    ├── requests.txt           # Waste collection requests and status
    ├── rewards.txt            # Reward points transaction ledger
    ├── sales.txt              # Commercial recyclable sales
    ├── store_items.txt        # Eco-Store catalog and inventory stock
    ├── purchases.txt          # Citizen points redemption orders and dispatch status
    ├── trucks.txt             # Municipal fleet database
    ├── users.txt              # Registered citizen accounts
    └── warehouse.txt          # Admin warehouse inventory (Collected -> Recycled -> Sold)
```
