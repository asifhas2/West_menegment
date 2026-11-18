#include "admin.h"
#include "data.h"
#include "utils.h"

/* Municipal staff authorization passkey for registering new admin profiles */
#define MUNICIPAL_MASTER_KEY "admin123"

/* ==================== ADMIN ENTRY & AUTHENTICATION ==================== */

void handleAdminModule(void) {
    while (1) {
        clearScreen();
        printHeader("MUNICIPAL ADMIN PORTAL - OPERATIONS MANAGEMENT");
        printf(" Central Municipal Waste Control & Administration\n");
        printf(" Please choose an action:\n\n");
        printf("   [1] Administrator Login\n");
        printf("   [2] Register New Administrator (Staff Sign-Up)\n");
        printf("   [3] Return to Main Menu\n");
        printDivider('-', 70);

        int choice = getInt(" Enter your choice [1-3]: ", 1, 3);
        if (choice == 1) {
            Admin loggedInAdmin;
            if (handleAdminLogin(&loggedInAdmin)) {
                showAdminDashboard(&loggedInAdmin);
            }
        } else if (choice == 2) {
            handleAdminRegistration();
        } else {
            break;
        }
    }
}

bool handleAdminRegistration(void) {
    clearScreen();
    printHeader("MUNICIPAL ADMINISTRATOR ONBOARDING");
    printf(" Authorized municipal personnel registration.\n\n");

    /* Require master authorization key */
    char masterKey[64];
    char key []="ADMIN1234";
    getString(" Enter Municipal Master Authorization Key: ", masterKey, sizeof(masterKey), false);
    if (strcmp(masterKey, MUNICIPAL_MASTER_KEY) != 0 && strcmp(masterKey, key) != 0) {
        printf("\n[!] Authorization Failed: Invalid master passkey. Contact the City IT Administrator.\n");
        pausePrompt();
        return false;
    }

    Admin newAdmin;
    memset(&newAdmin, 0, sizeof(newAdmin));

    generateNextAdminId(newAdmin.id);
    printf("\n Assigned Admin ID: %s\n\n", newAdmin.id);

    getString(" Enter Full Name of Officer: ", newAdmin.name, sizeof(newAdmin.name), false);

    /* Username uniqueness */
    static Admin admins[MAX_ADMINS];
    int count = loadAdmins(admins, MAX_ADMINS);

    while (1) {
        getString(" Enter Admin Username or Official Email: ", newAdmin.username, sizeof(newAdmin.username), false);
        if (findAdminByUsernameOrEmail(admins, count, newAdmin.username)) {
            printf("[!] An administrator with this username already exists! Please pick another.\n");
            continue;
        }
        break;
    }

    getString(" Enter Contact Phone: ", newAdmin.phone, sizeof(newAdmin.phone), false);
    getString(" Create Admin Password: ", newAdmin.password, sizeof(newAdmin.password), false);

    printf("\n Select Administrative Department / Role:\n");
    printf("   [1] Operations Supervisor\n");
    printf("   [2] Field Dispatcher & Route Coordinator\n");
    printf("   [3] Sanitation Director\n");
    printf("   [4] Environmental Auditor\n");
    int rChoice = getInt(" Select Role [1-4]: ", 1, 4);

    if (rChoice == 1) strcpy(newAdmin.role, "Operations Supervisor");
    else if (rChoice == 2) strcpy(newAdmin.role, "Field Dispatcher");
    else if (rChoice == 3) strcpy(newAdmin.role, "Sanitation Director");
    else strcpy(newAdmin.role, "Environmental Auditor");

    getCurrentTimestamp(newAdmin.created_at, sizeof(newAdmin.created_at));

    printf("\n");
    printDivider('-', 55);
    printf(" Administrator Account Summary:\n");
    printf("   Admin ID       : %s\n", newAdmin.id);
    printf("   Officer Name   : %s\n", newAdmin.name);
    printf("   Username/Email : %s\n", newAdmin.username);
    printf("   Phone Number   : %s\n", newAdmin.phone);
    printf("   Assigned Role  : %s\n", newAdmin.role);
    printDivider('-', 55);

    char confirm[16];
    getString(" Confirm admin registration? (Y/N): ", confirm, sizeof(confirm), false);
    if (confirm[0] == 'Y' || confirm[0] == 'y') {
        if (addAdmin(&newAdmin)) {
            printf("\n[SUCCESS] Administrator %s registered successfully!\n", newAdmin.name);
            printf("You can now login using username '%s' and your password.\n", newAdmin.username);
        } else {
            printf("\n[ERROR] Failed to save administrator profile.\n");
        }
    } else {
        printf("\n[INFO] Registration cancelled.\n");
    }

    pausePrompt();
    return true;
}

bool handleAdminLogin(Admin* loggedInAdmin) {
    clearScreen();
    printHeader("MUNICIPAL ADMINISTRATOR LOGIN");

    char username[64];
    char password[64];

    getString(" Enter Admin Username or Email: ", username, sizeof(username), false);
    getString(" Enter Admin Password: ", password, sizeof(password), false);

    static Admin admins[MAX_ADMINS];
    int count = loadAdmins(admins, MAX_ADMINS);

    Admin* admin = findAdminByUsernameOrEmail(admins, count, username);
    if (!admin || strcmp(admin->password, password) != 0) {
        printf("\n[!] Access Denied: Incorrect administrator username or password.\n");
        pausePrompt();
        return false;
    }

    *loggedInAdmin = *admin;
    printf("\n[SUCCESS] Administrator authenticated. Welcome back, %s (%s).\n",
           loggedInAdmin->name, loggedInAdmin->role);
    pausePrompt();
    return true;
}

/* Helper to calculate Admin Portal points earned after recycling and selling a batch */
static int calculateAdminSalePoints(const char* waste_type, float weight_kg, float total_amount) {
    int basePerKg = 8;
    if (strstr(waste_type, "Plastic") || strstr(waste_type, "Metal") || strstr(waste_type, "E-Waste")) {
        basePerKg = 10;
    } else if (strstr(waste_type, "Organic")) {
        basePerKg = 5;
    }
    int weightPoints = (int)(weight_kg * (float)basePerKg);
    int revenueBonus = (int)(total_amount / 10.0f);
    return weightPoints + revenueBonus;
}

/* ==================== ADMIN DASHBOARD ==================== */

void showAdminDashboard(Admin* admin) {
    while (1) {
        /* Refresh admin profile from disk so points and revenue stay current */
        static Admin adminList[MAX_ADMINS];
        int adminCount = loadAdmins(adminList, MAX_ADMINS);
        Admin* freshAdmin = findAdminById(adminList, adminCount, admin->id);
        if (freshAdmin) {
            *admin = *freshAdmin;
        }

        /* Sync any newly collected requests into the Admin Warehouse */
        syncCollectedRequestsToWarehouse();

        /* Gather quick dashboard metrics */
        static User users[MAX_USERS];
        int userCount = loadUsers(users, MAX_USERS);

        static WasteRequest reqs[MAX_REQUESTS];
        int reqCount = loadRequests(reqs, MAX_REQUESTS);
        int pendingReqs = 0, approvedReqs = 0;

        for (int i = 0; i < reqCount; i++) {
            if (reqs[i].status == REQ_STATUS_PENDING) pendingReqs++;
            else if (reqs[i].status == REQ_STATUS_APPROVED) approvedReqs++;
        }

        static WarehouseBatch batches[MAX_WAREHOUSE];
        int batchCount = loadWarehouseBatches(batches, MAX_WAREHOUSE);
        int toRecycleCount = 0, readyToSellCount = 0, completedBatchCount = 0;
        for (int i = 0; i < batchCount; i++) {
            if (strcmp(batches[i].stage, "Collected") == 0) toRecycleCount++;
            else if (strcmp(batches[i].stage, "Recycled") == 0) readyToSellCount++;
            else completedBatchCount++;
        }

        Collector collectors[MAX_COLLECTORS];
        int colCount = loadCollectors(collectors, MAX_COLLECTORS);

        Truck trucks[MAX_TRUCKS];
        int truckCount = loadTrucks(trucks, MAX_TRUCKS);

        static BinProblemReport bReports[MAX_BIN_REPORTS];
        int bRepCount = loadBinReports(bReports, MAX_BIN_REPORTS);
        int openBinReports = 0;
        for (int i = 0; i < bRepCount; i++) {
            if (strcmp(bReports[i].status, "Resolved") != 0) openBinReports++;
        }

        clearScreen();
        printHeader("MUNICIPAL OPERATIONS DASHBOARD");
        printf(" Logged In   : %s (ID: %s) | Role: %s\n", admin->name, admin->id, admin->role);
        printf(" Admin Wallet: %d Admin Points | Total Sales Revenue: $%.2f\n",
               admin->admin_points, admin->total_revenue);
        printf(" Warehouse   : %d Collected (To Recycle) | %d Recycled (Ready to Sell) | %d Finalized\n",
               toRecycleCount, readyToSellCount, completedBatchCount);
        printf(" Overview    : %d Citizens | %d Collectors | %d Trucks | %d Open Reports\n",
               userCount, colCount, truckCount, openBinReports);
        printf(" Requests    : %d Pending Review | %d Approved/Unassigned\n",
               pendingReqs, approvedReqs);
        printDivider('=', 74);
        printf(" Core Management Operations:\n");
        printf("   [1]  View & Search All Waste Requests\n");
        printf("   [2]  Approve or Reject Pending Requests\n");
        printf("   [3]  Assign Collector & Truck to Approved Requests\n");
        printf("   [4]  Generate Collection Route & Dispatch\n");
        printf("   [5]  Recycle / Process Collected Waste (Step 1: Warehouse)\n");
        printf("   [6]  Sell Recycled Waste & Claim Admin Points (Step 2: Sales)\n");
        printf("   [7]  View Admin Warehouse Inventory & Points Ledger\n");
        printf("   [8]  End-to-End Waste Transparency Trail\n");
        printf("   [9]  Manage Citizens Accounts\n");
        printf("   [10] Manage Public Smart Bins\n");
        printf("   [11] Manage Field Collectors\n");
        printf("   [12] Manage Vehicle Fleet (Trucks)\n");
        printf("   [13] Manage Problem Reports (Citizen & Collector)\n");
        printf("   [14] Statistical Reports & Performance Analytics\n");
        printf("   [15] Admin Logout\n");
        printDivider('-', 74);

        int choice = getInt(" Select an action [1-15]: ", 1, 15);
        switch (choice) {
            case 1:  manageRequestsUI(); break;
            case 2:  approveOrRejectRequestUI(); break;
            case 3:  assignCollectorAndTruckUI(); break;
            case 4:  routeDispatchUI(); break;
            case 5:  manageProcessingRecordsUI(admin); break;
            case 6:  recordRecyclableSaleUI(admin); break;
            case 7:  viewAdminWarehouseUI(admin); break;
            case 8:  viewTransparencyTrailUI(); break;
            case 9:  manageUsersUI(); break;
            case 10: manageBinsUI(); break;
            case 11: manageCollectorsUI(); break;
            case 12: manageTrucksUI(); break;
            case 13: manageProblemReportsUI(); break;
            case 14: generateSystemReportsUI(); break;
            case 15: return;
        }
    }
}

/* ==================== 1. MANAGE REQUESTS ==================== */

void manageRequestsUI(void) {
    clearScreen();
    printHeader("ALL SUBMITTED WASTE COLLECTION REQUESTS");

    static WasteRequest reqs[MAX_REQUESTS];
    int count = loadRequests(reqs, MAX_REQUESTS);

    if (count == 0) {
        printf(" No waste requests in the system.\n");
        pausePrompt();
        return;
    }

    printf("%-10s | %-10s | %-22s | %-8s | %-16s | %-14s\n",
           "Req ID", "Citizen", "Waste Type", "Est. Wt", "Assigned To", "Status");
    printDivider('-', 88);

    for (int i = 0; i < count; i++) {
        printf("%-10s | %-10s | %-22.22s | %-5.1f kg | %-16.16s | %-14s\n",
               reqs[i].id,
               reqs[i].user_id,
               reqs[i].waste_type,
               reqs[i].estimated_weight,
               reqs[i].assigned_collector,
               getStatusColorTag(reqs[i].status));
    }
    printDivider('=', 88);

    printf("\nOptions: [1] View Specific Request Details  [2] Return\n");
    int op = getInt(" Select Option [1-2]: ", 1, 2);
    if (op == 1) {
        char reqId[32];
        getString(" Enter Request ID: ", reqId, sizeof(reqId), false);
        WasteRequest* req = findRequestById(reqs, count, reqId);
        if (req) {
            clearScreen();
            printHeader("DETAILED WASTE REQUEST PROFILE");
            printf("  Request ID         : %s\n", req->id);
            printf("  Citizen ID         : %s\n", req->user_id);
            printf("  Submitted At       : %s\n", req->created_at);
            printf("  Waste Category     : %s\n", req->waste_type);
            printf("  Estimated Weight   : %.2f kg\n", req->estimated_weight);
            printf("  Actual Weight      : %.2f kg\n", req->actual_weight);
            printf("  Pickup Location    : %s\n", req->location);
            printf("  Smart Bin ID       : %s\n", req->bin_id);
            printf("  Preferred Schedule : %s during %s\n", req->preferred_date, req->preferred_slot);
            printf("  Citizen Notes      : %s\n", req->special_notes);
            printf("  Current Status     : %s %s\n", getStatusName(req->status), getStatusColorTag(req->status));
            printf("  Assigned Collector : %s\n", req->assigned_collector);
            printf("  Assigned Truck     : %s\n", req->assigned_truck);
            printf("  Processing Outcome : %s\n", req->final_outcome);
            printf("  Points Awarded     : %d pts\n", req->points_awarded);
            printf("  Completed At       : %s\n", req->completed_at);
            printDivider('-', 70);
            printProgressBar(req->status);
        } else {
            printf("[!] Request not found.\n");
        }
        pausePrompt();
    }
}

/* ==================== 2. APPROVE OR REJECT REQUEST ==================== */

void approveOrRejectRequestUI(void) {
    clearScreen();
    printHeader("REVIEW & APPROVE / REJECT PENDING REQUESTS");

    static WasteRequest reqs[MAX_REQUESTS];
    int count = loadRequests(reqs, MAX_REQUESTS);
    int pendingCount = 0;

    printf("%-10s | %-10s | %-24s | %-8s | %-20s\n",
           "Req ID", "Citizen", "Waste Type", "Est Wt", "Pickup Date");
    printDivider('-', 75);

    for (int i = 0; i < count; i++) {
        if (reqs[i].status == REQ_STATUS_PENDING) {
            printf("%-10s | %-10s | %-24s | %-5.1f kg | %-20s\n",
                   reqs[i].id, reqs[i].user_id, reqs[i].waste_type,
                   reqs[i].estimated_weight, reqs[i].preferred_date);
            pendingCount++;
        }
    }

    if (pendingCount == 0) {
        printf(" No pending requests currently awaiting review.\n");
        pausePrompt();
        return;
    }

    printDivider('=', 75);
    char reqId[32];
    getString(" Enter Request ID to Review (or '0' to return): ", reqId, sizeof(reqId), false);
    if (strcmp(reqId, "0") == 0) return;

    WasteRequest* req = findRequestById(reqs, count, reqId);
    if (!req || req->status != REQ_STATUS_PENDING) {
        printf("[!] Invalid Request ID or request is not pending.\n");
        pausePrompt();
        return;
    }

    printf("\n Action for %s (%s, %.1f kg):\n", req->id, req->waste_type, req->estimated_weight);
    printf("   [1] Approve Request (Move to Scheduled/Ready for Assignment)\n");
    printf("   [2] Reject Request (Provide reason for cancellation)\n");
    printf("   [3] Cancel\n");

    int decision = getInt(" Select decision [1-3]: ", 1, 3);
    if (decision == 1) {
        req->status = REQ_STATUS_APPROVED;
        strcpy(req->final_outcome, "Approved by Admin - Awaiting Collector & Truck Assignment");
        updateRequest(req);
        printf("\n[SUCCESS] Request %s approved! It is now ready for collector assignment.\n", req->id);
    } else if (decision == 2) {
        char reason[128];
        getString(" Enter Rejection Reason: ", reason, sizeof(reason), false);
        snprintf(req->final_outcome, sizeof(req->final_outcome), "Rejected: %s", reason);
        strcpy(req->completed_at, "Rejected");
        updateRequest(req);
        printf("\n[SUCCESS] Request %s has been rejected with reason noted.\n", req->id);
    }

    pausePrompt();
}

/* ==================== 3. ASSIGN COLLECTOR & TRUCK ==================== */

void assignCollectorAndTruckUI(void) {
    clearScreen();
    printHeader("ASSIGN COLLECTOR & TRUCK TO APPROVED REQUESTS");

    static WasteRequest reqs[MAX_REQUESTS];
    int reqCount = loadRequests(reqs, MAX_REQUESTS);
    int approvedCount = 0;

    printf(" Requests Approved and Awaiting Assignment:\n");
    printf("%-10s | %-10s | %-24s | %-8s | %-20s\n",
           "Req ID", "Citizen", "Waste Type", "Est Wt", "Location");
    printDivider('-', 75);

    for (int i = 0; i < reqCount; i++) {
        if (reqs[i].status == REQ_STATUS_APPROVED) {
            printf("%-10s | %-10s | %-24s | %-5.1f kg | %-20.20s\n",
                   reqs[i].id, reqs[i].user_id, reqs[i].waste_type,
                   reqs[i].estimated_weight, reqs[i].location);
            approvedCount++;
        }
    }

    if (approvedCount == 0) {
        printf(" No approved requests currently pending assignment.\n");
        pausePrompt();
        return;
    }

    printDivider('=', 75);
    char reqId[32];
    getString(" Enter Request ID to assign: ", reqId, sizeof(reqId), false);

    WasteRequest* req = findRequestById(reqs, reqCount, reqId);
    if (!req || req->status != REQ_STATUS_APPROVED) {
        printf("[!] Request not eligible for assignment.\n");
        pausePrompt();
        return;
    }

    /* Select Collector */
    Collector collectors[MAX_COLLECTORS];
    int colCount = loadCollectors(collectors, MAX_COLLECTORS);
    printf("\n Available Collectors:\n");
    for (int i = 0; i < colCount; i++) {
        printf("   [%d] %s (%s) - Default Truck: %s | Status: %s\n",
               i + 1, collectors[i].name, collectors[i].id,
               collectors[i].assigned_truck, collectors[i].status);
    }
    int colChoice = getInt(" Select Collector [1 to count]: ", 1, colCount);
    Collector* chosenCol = &collectors[colChoice - 1];

    /* Select Truck */
    Truck trucks[MAX_TRUCKS];
    int truckCount = loadTrucks(trucks, MAX_TRUCKS);
    printf("\n Available Fleet Trucks:\n");
    for (int i = 0; i < truckCount; i++) {
        printf("   [%d] %s (%s) | Cap: %.0f kg | Load: %.1f kg | %s\n",
               i + 1, trucks[i].id, trucks[i].plate_number,
               trucks[i].capacity_kg, trucks[i].current_load_kg, trucks[i].status);
    }
    int trkChoice = getInt(" Select Truck [1 to count]: ", 1, truckCount);
    Truck* chosenTruck = &trucks[trkChoice - 1];

    /* Update request with assignment */
    snprintf(req->assigned_collector, sizeof(req->assigned_collector), "%s (%s)", chosenCol->id, chosenCol->name);
    strncpy(req->assigned_truck, chosenTruck->id, sizeof(req->assigned_truck) - 1);
    req->status = REQ_STATUS_ASSIGNED;
    strcpy(req->final_outcome, "Assigned to Collector - Scheduled on Daily Route");

    if (updateRequest(req)) {
        printf("\n[SUCCESS] Request %s assigned to Collector %s and Truck %s!\n",
               req->id, chosenCol->name, chosenTruck->id);
        printf("The collection task has been dispatched to the collector's schedule.\n");
    } else {
        printf("\n[ERROR] Failed to save assignment.\n");
    }

    pausePrompt();
}

/* ==================== 4. ROUTE DISPATCH ==================== */



void manageUsersUI(void) {
    while (1) {
        clearScreen();
        printHeader("MANAGE CITIZEN ACCOUNTS");

        User users[MAX_USERS];
        int count = loadUsers(users, MAX_USERS);

        printf("%-10s | %-20s | %-24s | %-14s | %-8s\n",
               "User ID", "Full Name", "Email Address", "Phone", "Points");
        printDivider('-', 85);

        for (int i = 0; i < count; i++) {
            printf("%-10s | %-20.20s | %-24.24s | %-14s | %d pts\n",
                   users[i].id, users[i].name, users[i].email, users[i].phone, users[i].reward_points);
        }
        printDivider('=', 85);
        printf(" Options: [1] Add User  [2] Update User  [3] Remove User  [4] Return\n");

        int choice = getInt(" Select Option [1-4]: ", 1, 4);
        if (choice == 1) {
            User newUser;
            memset(&newUser, 0, sizeof(newUser));
            generateNextUserId(newUser.id);
            printf("\n Creating Citizen Account %s:\n", newUser.id);
            getString(" Enter Name: ", newUser.name, sizeof(newUser.name), false);
            getString(" Enter Email: ", newUser.email, sizeof(newUser.email), false);
            getString(" Enter Phone: ", newUser.phone, sizeof(newUser.phone), false);
            getString(" Enter Password: ", newUser.password, sizeof(newUser.password), false);
            newUser.reward_points = 0;
            getCurrentTimestamp(newUser.created_at, sizeof(newUser.created_at));
            addUser(&newUser);
            printf("\n[SUCCESS] Citizen %s added.\n", newUser.name);
            pausePrompt();
        } else if (choice == 2) {
            char uId[32];
            getString("\n Enter User ID to update: ", uId, sizeof(uId), false);
            User* u = findUserById(users, count, uId);
            if (u) {
                getString(" New Name (press ENTER to keep current): ", u->name, sizeof(u->name), true);
                getString(" New Phone (press ENTER to keep current): ", u->phone, sizeof(u->phone), true);
                updateUser(u);
                printf("\n[SUCCESS] User %s updated.\n", u->id);
            } else {
                printf("[!] User not found.\n");
            }
            pausePrompt();
        } else if (choice == 3) {
            char uId[32];
            getString("\n Enter User ID to remove: ", uId, sizeof(uId), false);
            if (deleteUser(uId)) {
                printf("\n[SUCCESS] User %s removed from system.\n", uId);
            } else {
                printf("[!] User ID not found.\n");
            }
            pausePrompt();
        } else {
            break;
        }
    }
}

void routeDispatchUI(void) {
    clearScreen();
    printHeader("COLLECTION ROUTE GENERATION & DISPATCH");

    Collector collectors[MAX_COLLECTORS];
    int colCount = loadCollectors(collectors, MAX_COLLECTORS);

    printf(" Select an active collector to generate/review their route:\n");
    for (int i = 0; i < colCount; i++) {
        printf("   [%d] %s (ID: %s) - Vehicle: %s\n",
               i + 1, collectors[i].name, collectors[i].id, collectors[i].assigned_truck);
    }
    int choice = getInt(" Select Collector [1 to count]: ", 1, colCount);
    Collector* col = &collectors[choice - 1];

    clearScreen();
    printHeader("ROUTE DISPATCH MANIFEST");
    printf(" Assigned Officer: %s (%s) | Truck: %s\n\n", col->name, col->id, col->assigned_truck);

    static WasteRequest reqs[MAX_REQUESTS];
    int count = loadRequests(reqs, MAX_REQUESTS);
    int stopNum = 1;
    float totalEstimatedWeight = 0.0f;

    for (int i = 0; i < count; i++) {
        if (strstr(reqs[i].assigned_collector, col->id) != NULL &&
            (reqs[i].status == REQ_STATUS_ASSIGNED || reqs[i].status == REQ_STATUS_IN_PROGRESS)) {
            printf(" [STOP %d] Task: %s\n", stopNum++, reqs[i].id);
            printf("   * Location      : %s\n", reqs[i].location);
            printf("   * Smart Bin Ref : %s\n", reqs[i].bin_id);
            printf("   * Waste Type    : %s\n", reqs[i].waste_type);
            printf("   * Est Weight    : %.2f kg\n", reqs[i].estimated_weight);
            printf("   * Preferred Slot: %s\n", reqs[i].preferred_slot);
            printDivider('-', 65);
            totalEstimatedWeight += reqs[i].estimated_weight;
        }
    }

    if (stopNum == 1) {
        printf(" No active route stops scheduled for this collector.\n");
    } else {
        printf(" Total Stops: %d | Total Estimated Payload: %.2f kg\n", stopNum - 1, totalEstimatedWeight);
        printf("[DISPATCH] Route manifests are automatically synced to the collector's mobile portal.\n");
    }

    pausePrompt();
}

/* ==================== 5. STEP 1: RECYCLE / PROCESS COLLECTED WASTE ==================== */

void manageProcessingRecordsUI(Admin* admin) {
    clearScreen();
    printHeader("STEP 1: RECYCLE / PROCESS COLLECTED WASTE");
    printf(" Select collected waste delivered by field collectors to recycle or safely dispose.\n\n");

    syncCollectedRequestsToWarehouse();

    static WarehouseBatch batches[MAX_WAREHOUSE];
    int batchCount = loadWarehouseBatches(batches, MAX_WAREHOUSE);
    int pendingRecycle = 0;

    printf("%-10s | %-10s | %-10s | %-24s | %-10s | %-16s\n",
           "Batch ID", "Req ID", "Collector", "Waste Category", "Weight", "Delivered On");
    printDivider('-', 90);

    for (int i = 0; i < batchCount; i++) {
        if (strcmp(batches[i].stage, "Collected") == 0) {
            printf("%-10s | %-10s | %-10s | %-24.24s | %-6.2f kg  | %-16s\n",
                   batches[i].batch_id, batches[i].request_id, batches[i].collector_id,
                   batches[i].waste_type, batches[i].weight_kg, batches[i].updated_at);
            pendingRecycle++;
        }
    }

    if (pendingRecycle == 0) {
        printf(" No collected waste batches currently waiting for recycling.\n");
        printf(" Once a collector completes a collection, it automatically appears here.\n");
        pausePrompt();
        return;
    }

    printDivider('=', 90);
    char inputId[32];
    getString(" Enter Batch ID (e.g. BAT1001) or Request ID (e.g. REQ1012) to process: ", inputId, sizeof(inputId), false);

    WarehouseBatch* batch = findWarehouseBatchByIdOrReq(batches, batchCount, inputId);
    if (!batch || strcmp(batch->stage, "Collected") != 0) {
        printf("[!] Invalid ID or this batch has already been processed.\n");
        pausePrompt();
        return;
    }

    static WasteRequest reqs[MAX_REQUESTS];
    int reqCount = loadRequests(reqs, MAX_REQUESTS);
    WasteRequest* req = findRequestById(reqs, reqCount, batch->request_id);

    printf("\n Selected Warehouse Batch: %s (Request: %s)\n", batch->batch_id, batch->request_id);
    printf("   Material Type : %s\n", batch->waste_type);
    printf("   Actual Weight : %.2f kg (Delivered by %s)\n", batch->weight_kg, batch->collector_id);

    /* Special compliance handling for Chemical / Hazardous Waste */
    if (strstr(batch->waste_type, "Hazardous") || strstr(batch->waste_type, "Chemical")) {
        printf("\n [!] CHEMICAL / HAZARDOUS WASTE DETECTED [!]\n");
        printf(" Chemical waste cannot be recycled or sold like ordinary materials.\n");
        printf(" Please log certified handover to an authorized treatment facility.\n\n");

        char facility[64];
        char manifestId[32];
        char method[64];

        getString(" Authorized Chemical Disposal Facility: ", facility, sizeof(facility), false);
        getString(" Safe Handover Manifest / Permit Number: ", manifestId, sizeof(manifestId), false);
        getString(" Treatment Method (e.g. High-Temp Incineration / Neutralization): ", method, sizeof(method), false);

        snprintf(batch->recycle_details, sizeof(batch->recycle_details),
                 "Handed over to %s (Permit: %s) for safe %s", facility, manifestId, method);
        strcpy(batch->stage, "Disposed");
        strncpy(batch->processed_by, admin->id, sizeof(batch->processed_by) - 1);
        getCurrentTimestamp(batch->updated_at, sizeof(batch->updated_at));

        /* Award compliance points to Admin for safe hazardous waste disposal */
        int safetyPoints = (int)(batch->weight_kg * 15.0f);
        if (safetyPoints < 10) safetyPoints = 10;
        batch->admin_points_earned = safetyPoints;

        updateWarehouseBatch(batch);

        if (req) {
            req->status = REQ_STATUS_FINAL_OUTCOME;
            strncpy(req->final_outcome, batch->recycle_details, sizeof(req->final_outcome) - 1);
            getCurrentTimestamp(req->completed_at, sizeof(req->completed_at));
            updateRequest(req);
        }

        admin->admin_points += safetyPoints;
        updateAdmin(admin);

        printf("\n[SUCCESS] Hazardous batch %s safely disposed and certified!\n", batch->batch_id);
        printf("[ADMIN POINTS ADDED] +%d Safety Compliance Points added to your Admin Portal!\n", safetyPoints);
        printf("Current Admin Balance: %d pts\n", admin->admin_points);
    } else {
        /* Recyclable, Organic, or E-Waste -> Step 1: Recycle into sellable material */
        printf("\n Select Recycling / Processing Method:\n");
        printf("   [1] Recycled into raw polymer pellets / flake (Plastic)\n");
        printf("   [2] Pulped into recycled cardboard & paper stock (Paper)\n");
        printf("   [3] Smelted & purified into metal ingots (Metal)\n");
        printf("   [4] Crushed & cleaned into glass cullet (Glass)\n");
        printf("   [5] Composted into organic bio-fertilizer (Organic)\n");
        printf("   [6] Dismantled into recovered electronic components (E-Waste)\n");
        printf("   [7] Enter Custom Recycling Description\n");

        int pChoice = getInt(" Select Option [1-7]: ", 1, 7);
        if (pChoice == 1) strcpy(batch->recycle_details, "Recycled into raw polymer pellets at Municipal Recycling Plant");
        else if (pChoice == 2) strcpy(batch->recycle_details, "Pulped into recycled cardboard & paper sheets");
        else if (pChoice == 3) strcpy(batch->recycle_details, "Smelted and purified into reusable metal ingots");
        else if (pChoice == 4) strcpy(batch->recycle_details, "Crushed and sanitized into reusable glass cullet");
        else if (pChoice == 5) strcpy(batch->recycle_details, "Composted into organic bio-fertilizer stock");
        else if (pChoice == 6) strcpy(batch->recycle_details, "Dismantled into recovered e-waste components & copper");
        else {
            getString(" Enter Recycling Description: ", batch->recycle_details, sizeof(batch->recycle_details), false);
        }

        strcpy(batch->stage, "Recycled");
        strncpy(batch->processed_by, admin->id, sizeof(batch->processed_by) - 1);
        getCurrentTimestamp(batch->updated_at, sizeof(batch->updated_at));
        updateWarehouseBatch(batch);

        if (req) {
            snprintf(req->final_outcome, sizeof(req->final_outcome),
                     "Recycled: %s (Ready for Commercial Sale)", batch->recycle_details);
            updateRequest(req);
        }

        printf("\n[SUCCESS] Batch %s (%.2f kg %s) has been RECYCLED!\n",
               batch->batch_id, batch->weight_kg, batch->waste_type);
        printf("Processing Note: %s\n", batch->recycle_details);
        printf("Next Step: Go to Option [6] to sell this recycled batch and earn Admin Points.\n");
    }

    pausePrompt();
}

/* ==================== 6. STEP 2: SELL RECYCLED WASTE & ADD ADMIN POINTS ==================== */

void recordRecyclableSaleUI(Admin* admin) {
    clearScreen();
    printHeader("STEP 2: SELL RECYCLED WASTE & EARN ADMIN POINTS");
    printf(" Sell recycled waste batches from your warehouse to commercial buyers.\n");
    printf(" Once sold, points and revenue are automatically added to your Admin Portal!\n\n");

    static WarehouseBatch batches[MAX_WAREHOUSE];
    int batchCount = loadWarehouseBatches(batches, MAX_WAREHOUSE);
    int readyCount = 0;

    printf("%-10s | %-10s | %-24s | %-10s | %-28s\n",
           "Batch ID", "Req ID", "Material Type", "Weight", "Recycled Form");
    printDivider('-', 90);

    for (int i = 0; i < batchCount; i++) {
        if (strcmp(batches[i].stage, "Recycled") == 0) {
            printf("%-10s | %-10s | %-24.24s | %-6.2f kg  | %-28.28s\n",
                   batches[i].batch_id, batches[i].request_id, batches[i].waste_type,
                   batches[i].weight_kg, batches[i].recycle_details);
            readyCount++;
        }
    }

    if (readyCount == 0) {
        printf(" No recycled batches are currently ready for sale.\n");
        printf(" Please complete Step 1 (Option [5] Recycle Collected Waste) first!\n");
        pausePrompt();
        return;
    }

    printDivider('=', 90);
    char inputId[32];
    getString(" Enter Batch ID (e.g. BAT1001) or Request ID to sell: ", inputId, sizeof(inputId), false);

    WarehouseBatch* batch = findWarehouseBatchByIdOrReq(batches, batchCount, inputId);
    if (!batch || strcmp(batch->stage, "Recycled") != 0) {
        printf("[!] Invalid ID or this batch is not in 'Recycled' stage ready for sale.\n");
        pausePrompt();
        return;
    }

    RecyclableSale sale;
    memset(&sale, 0, sizeof(sale));

    generateNextSaleId(sale.id);
    strncpy(sale.material_type, batch->waste_type, sizeof(sale.material_type) - 1);
    sale.weight_kg = batch->weight_kg;
    strncpy(sale.batch_id, batch->batch_id, sizeof(sale.batch_id) - 1);
    strncpy(sale.admin_id, admin->id, sizeof(sale.admin_id) - 1);

    printf("\n Selling Recycled Batch %s (%s - %.2f kg)\n",
           batch->batch_id, batch->waste_type, batch->weight_kg);
    printf(" Recycling Details: %s\n\n", batch->recycle_details);

    getString(" Commercial Buyer / Recycling Company: ", sale.buyer_name, sizeof(sale.buyer_name), false);
    sale.rate_per_kg = getFloat(" Sale Rate per kg (Currency): ", 0.5f, 1000.0f);
    sale.total_amount = sale.weight_kg * sale.rate_per_kg;
    getCurrentTimestamp(sale.sale_date, sizeof(sale.sale_date));

    int earnedPoints = calculateAdminSalePoints(sale.material_type, sale.weight_kg, sale.total_amount);

    printf("\n Commercial Sale & Admin Points Summary:\n");
    printDivider('-', 65);
    printf("   Sale ID            : %s\n", sale.id);
    printf("   Linked Batch / Req : %s / %s\n", batch->batch_id, batch->request_id);
    printf("   Material Sold      : %s\n", sale.material_type);
    printf("   Verified Weight    : %.2f kg\n", sale.weight_kg);
    printf("   Buyer Company      : %s\n", sale.buyer_name);
    printf("   Rate / kg          : $%.2f\n", sale.rate_per_kg);
    printf("   Total Sale Revenue : $%.2f\n", sale.total_amount);
    printf("   Admin Points Earned: +%d pts (Weight + Revenue Bonus)\n", earnedPoints);
    printDivider('-', 65);

    char confirm[16];
    getString(" Confirm sale and credit points to Admin Portal? (Y/N): ", confirm, sizeof(confirm), false);
    if (confirm[0] == 'Y' || confirm[0] == 'y') {
        /* 1. Save Sale Record */
        addSale(&sale);

        /* 2. Update Warehouse Batch to Sold */
        strcpy(batch->stage, "Sold");
        strncpy(batch->sale_id, sale.id, sizeof(batch->sale_id) - 1);
        batch->sale_amount = sale.total_amount;
        batch->admin_points_earned = earnedPoints;
        strncpy(batch->processed_by, admin->id, sizeof(batch->processed_by) - 1);
        getCurrentTimestamp(batch->updated_at, sizeof(batch->updated_at));
        updateWarehouseBatch(batch);

        /* 3. Finalize Citizen's Request Transparency Trail */
        static WasteRequest reqs[MAX_REQUESTS];
        int reqCount = loadRequests(reqs, MAX_REQUESTS);
        WasteRequest* req = findRequestById(reqs, reqCount, batch->request_id);
        if (req) {
            req->status = REQ_STATUS_FINAL_OUTCOME;
            snprintf(req->final_outcome, sizeof(req->final_outcome),
                     "%s | Sold to %s ($%.2f, %s)",
                     batch->recycle_details, sale.buyer_name, sale.total_amount, sale.id);
            getCurrentTimestamp(req->completed_at, sizeof(req->completed_at));
            updateRequest(req);
        }

        /* 4. Credit Admin Points & Revenue to Admin Portal Account */
        admin->admin_points += earnedPoints;
        admin->total_revenue += sale.total_amount;
        updateAdmin(admin);

        printf("\n[SUCCESS] Sale transaction %s completed!\n", sale.id);
        printf("[ADMIN POINTS ADDED] +%d Admin Points & +$%.2f Revenue added to %s's Portal!\n",
               earnedPoints, sale.total_amount, admin->name);
        printf("Updated Admin Balance: %d pts | Total Revenue: $%.2f\n",
               admin->admin_points, admin->total_revenue);
    } else {
        printf("\n[INFO] Commercial sale cancelled.\n");
    }

    pausePrompt();
}

/* ==================== 7. VIEW ADMIN WAREHOUSE INVENTORY & POINTS ==================== */

void viewAdminWarehouseUI(const Admin* admin) {
    clearScreen();
    printHeader("ADMIN WAREHOUSE INVENTORY & POINTS LEDGER");
    printf(" Admin Officer : %s (%s) | Role: %s\n", admin->name, admin->id, admin->role);
    printf(" Admin Balance : [%d Admin Points] | Total Revenue: [$%.2f]\n",
           admin->admin_points, admin->total_revenue);
    printDivider('=', 95);

    syncCollectedRequestsToWarehouse();

    static WarehouseBatch batches[MAX_WAREHOUSE];
    int count = loadWarehouseBatches(batches, MAX_WAREHOUSE);

    if (count == 0) {
        printf(" Warehouse is currently empty. Batches appear automatically when collectors complete tasks.\n");
    } else {
        printf("%-9s | %-9s | %-8s | %-20s | %-8s | %-10s | %-9s | %-8s\n",
               "Batch ID", "Req ID", "Coll ID", "Waste Type", "Weight", "Stage", "Revenue", "Adm Pts");
        printDivider('-', 95);

        float totalKg = 0.0f;
        float totalRev = 0.0f;
        int totalAdmPts = 0;

        for (int i = 0; i < count; i++) {
            printf("%-9s | %-9s | %-8s | %-20.20s | %-5.1f kg | %-10s | $%-8.2f | +%-6d\n",
                   batches[i].batch_id,
                   batches[i].request_id,
                   batches[i].collector_id,
                   batches[i].waste_type,
                   batches[i].weight_kg,
                   batches[i].stage,
                   batches[i].sale_amount,
                   batches[i].admin_points_earned);
            totalKg += batches[i].weight_kg;
            totalRev += batches[i].sale_amount;
            totalAdmPts += batches[i].admin_points_earned;
        }
        printDivider('-', 95);
        printf(" WAREHOUSE TOTALS: %d Batches | %.2f kg Handled | $%.2f Sold | +%d Admin Points Earned\n",
               count, totalKg, totalRev, totalAdmPts);
    }

    printf("\n Pipeline Stages:\n");
    printf("   * Collected -> Received from Collector (Go to Option [5] to Recycle)\n");
    printf("   * Recycled  -> Processed & Ready (Go to Option [6] to Sell & Earn Points)\n");
    printf("   * Sold      -> Sold to Buyer & Points Credited to Admin Portal\n");
    printf("   * Disposed  -> Hazardous/Chemical Waste Safely Handed Over\n");
    printDivider('=', 95);

    pausePrompt();
}

/* ==================== 7. TRANSPARENCY TRAIL ==================== */

void viewTransparencyTrailUI(void) {
    clearScreen();
    printHeader("END-TO-END TRANSPARENCY TRAIL AUDIT");
    printf(" Trace waste from citizen source -> collection -> processing -> final destination.\n\n");

    char reqId[32];
    getString(" Enter Request ID to audit (or press ENTER to view all final records): ", reqId, sizeof(reqId), true);

    static WasteRequest reqs[MAX_REQUESTS];
    int count = loadRequests(reqs, MAX_REQUESTS);
    int shown = 0;

    for (int i = 0; i < count; i++) {
        if (strlen(reqId) > 0 && strcmp(reqs[i].id, reqId) != 0) {
            continue;
        }

        if (reqs[i].status == REQ_STATUS_FINAL_OUTCOME || strlen(reqId) > 0) {
            printf(" [BATCH AUDIT %s]\n", reqs[i].id);
            printf("   1. SOURCE GENERATION :\n");
            printf("      * Citizen ID : %s | Pickup Spot: %s\n", reqs[i].user_id, reqs[i].location);
            printf("      * Waste Type : %s | Est Wt: %.2f kg\n", reqs[i].waste_type, reqs[i].estimated_weight);
            printf("   2. COLLECTION STAGE  :\n");
            printf("      * Collector  : %s | Truck: %s\n", reqs[i].assigned_collector, reqs[i].assigned_truck);
            printf("      * Actual Wt  : %.2f kg | Points: %d pts\n", reqs[i].actual_weight, reqs[i].points_awarded);
            printf("   3. FINAL OUTCOME     :\n");
            printf("      * Destination: %s\n", reqs[i].final_outcome);
            printf("      * Finalized  : %s\n", reqs[i].completed_at);
            printDivider('-', 75);
            shown++;
        }
    }

    if (shown == 0) {
        printf(" No matching transparency records found.\n");
    }

    pausePrompt();
}


/* ==================== 9. MANAGE BINS ==================== */

void manageBinsUI(void) {
    while (1) {
        clearScreen();
        printHeader("MANAGE PUBLIC SMART BINS");

        SmartBin bins[MAX_BINS];
        int count = loadBins(bins, MAX_BINS);

        printf("%-10s | %-32s | %-24s | %-12s\n",
               "Bin ID", "Location Landmark", "Waste Category", "Status");
        printDivider('-', 85);

        for (int i = 0; i < count; i++) {
            printf("%-10s | %-32.32s | %-24.24s | %-12s\n",
                   bins[i].bin_id, bins[i].location, bins[i].category, bins[i].status);
        }
        printDivider('=', 85);
        printf(" Options: [1] Add Smart Bin  [2] Update Bin Status  [3] Delete Bin  [4] Return\n");

        int choice = getInt(" Select Option [1-4]: ", 1, 4);
        if (choice == 1) {
            SmartBin newBin;
            memset(&newBin, 0, sizeof(newBin));
            getString("\n Enter Bin ID (e.g. BIN106): ", newBin.bin_id, sizeof(newBin.bin_id), false);
            getString(" Enter Location Landmark: ", newBin.location, sizeof(newBin.location), false);
            getString(" Enter Waste Category: ", newBin.category, sizeof(newBin.category), false);
            strcpy(newBin.status, "Normal");
            addBin(&newBin);
            printf("\n[SUCCESS] Bin %s installed.\n", newBin.bin_id);
            pausePrompt();
        } else if (choice == 2) {
            char bId[32];
            getString("\n Enter Bin ID to update: ", bId, sizeof(bId), false);
            bool found = false;
            for (int i = 0; i < count; i++) {
                if (strcmp(bins[i].bin_id, bId) == 0) {
                    printf(" Current Status: %s\n", bins[i].status);
                    printf(" Select New Status: [1] Normal  [2] Full  [3] Maintenance\n");
                    int sChoice = getInt(" Select [1-3]: ", 1, 3);
                    if (sChoice == 1) strcpy(bins[i].status, "Normal");
                    else if (sChoice == 2) strcpy(bins[i].status, "Full");
                    else strcpy(bins[i].status, "Maintenance");
                    updateBin(&bins[i]);
                    printf("\n[SUCCESS] Bin status updated to %s.\n", bins[i].status);
                    found = true;
                    break;
                }
            }
            if (!found) printf("[!] Bin ID not found.\n");
            pausePrompt();
        } else if (choice == 3) {
            char bId[32];
            getString("\n Enter Bin ID to remove: ", bId, sizeof(bId), false);
            if (deleteBin(bId)) {
                printf("\n[SUCCESS] Bin %s removed.\n", bId);
            } else {
                printf("[!] Bin ID not found.\n");
            }
            pausePrompt();
        } else {
            break;
        }
    }
}

/* ==================== 10. MANAGE COLLECTORS ==================== */

void manageCollectorsUI(void) {
    while (1) {
        clearScreen();
        printHeader("MANAGE FIELD WASTE COLLECTORS");

        Collector collectors[MAX_COLLECTORS];
        int count = loadCollectors(collectors, MAX_COLLECTORS);

        printf("%-10s | %-18s | %-12s | %-10s | %-12s | %-10s\n",
               "ID", "Name", "Phone", "Truck", "Status", "Haul (kg)");
        printDivider('-', 80);

        for (int i = 0; i < count; i++) {
            printf("%-10s | %-18.18s | %-12s | %-10s | %-12s | %-6.1f kg\n",
                   collectors[i].id, collectors[i].name, collectors[i].phone,
                   collectors[i].assigned_truck, collectors[i].status, collectors[i].total_collected_kg);
        }
        printDivider('=', 80);
        printf(" Options: [1] Add New Collector  [2] Update Collector Status  [3] Remove Collector  [4] Return\n");

        int choice = getInt(" Select Option [1-4]: ", 1, 4);
        if (choice == 1) {
            Collector newCol;
            memset(&newCol, 0, sizeof(newCol));
            generateNextCollectorId(newCol.id);
            printf("\n Adding New Collector (%s):\n", newCol.id);
            getString(" Enter Officer Name: ", newCol.name, sizeof(newCol.name), false);
            getString(" Enter Email: ", newCol.email, sizeof(newCol.email), false);
            getString(" Enter Phone: ", newCol.phone, sizeof(newCol.phone), false);
            getString(" Enter Password: ", newCol.password, sizeof(newCol.password), false);
            getString(" Assigned Truck ID (e.g. TRK-01): ", newCol.assigned_truck, sizeof(newCol.assigned_truck), false);
            strcpy(newCol.status, "Available");
            newCol.total_collected_kg = 0.0f;
            newCol.tasks_completed = 0;
            addCollector(&newCol);
            printf("\n[SUCCESS] Collector %s registered.\n", newCol.name);
            pausePrompt();
        } else if (choice == 2) {
            char colId[32];
            getString("\n Enter Collector ID to update: ", colId, sizeof(colId), false);
            Collector* col = findCollectorById(collectors, count, colId);
            if (col) {
                printf(" Current Status: %s | Current Truck: %s\n", col->status, col->assigned_truck);
                printf(" Choose New Duty Status: [1] Available  [2] Busy  [3] On Leave\n");
                int s = getInt(" Select [1-3]: ", 1, 3);
                if (s == 1) strcpy(col->status, "Available");
                else if (s == 2) strcpy(col->status, "Busy");
                else strcpy(col->status, "On Leave");
                getString(" Update Assigned Truck (press ENTER to keep current): ", col->assigned_truck, sizeof(col->assigned_truck), true);
                updateCollector(col);
                printf("\n[SUCCESS] Collector %s updated.\n", col->id);
            } else {
                printf("[!] Collector not found.\n");
            }
            pausePrompt();
        } else if (choice == 3) {
            char colId[32];
            getString("\n Enter Collector ID to remove: ", colId, sizeof(colId), false);
            if (deleteCollector(colId)) {
                printf("\n[SUCCESS] Collector %s removed.\n", colId);
            } else {
                printf("[!] Collector not found.\n");
            }
            pausePrompt();
        } else {
            break;
        }
    }
}

/* ==================== 11. MANAGE TRUCKS ==================== */

void manageTrucksUI(void) {
    while (1) {
        clearScreen();
        printHeader("MANAGE VEHICLE FLEET (TRUCKS)");

        Truck trucks[MAX_TRUCKS];
        int count = loadTrucks(trucks, MAX_TRUCKS);

        printf("%-10s | %-16s | %-18s | %-10s | %-10s | %-12s\n",
               "Truck ID", "Plate No.", "Assigned Driver", "Max Cap", "Load (kg)", "Status");
        printDivider('-', 85);

        for (int i = 0; i < count; i++) {
            printf("%-10s | %-16s | %-18.18s | %-6.0f kg | %-6.1f kg | %-12s\n",
                   trucks[i].id, trucks[i].plate_number, trucks[i].driver_name,
                   trucks[i].capacity_kg, trucks[i].current_load_kg, trucks[i].status);
        }
        printDivider('=', 85);
        printf(" Options: [1] Add New Truck  [2] Reset/Empty Truck Load  [3] Remove Truck  [4] Return\n");

        int choice = getInt(" Select Option [1-4]: ", 1, 4);
        if (choice == 1) {
            Truck newTruck;
            memset(&newTruck, 0, sizeof(newTruck));
            generateNextTruckId(newTruck.id);
            printf("\n Adding Fleet Vehicle (%s):\n", newTruck.id);
            getString(" Enter Plate Number: ", newTruck.plate_number, sizeof(newTruck.plate_number), false);
            getString(" Enter Driver Name: ", newTruck.driver_name, sizeof(newTruck.driver_name), false);
            newTruck.capacity_kg = getFloat(" Enter Haul Capacity (kg): ", 100.0f, 100000.0f);
            newTruck.current_load_kg = 0.0f;
            strcpy(newTruck.status, "Available");
            addTruck(&newTruck);
            printf("\n[SUCCESS] Truck %s added to fleet.\n", newTruck.id);
            pausePrompt();
        } else if (choice == 2) {
            char tId[32];
            getString("\n Enter Truck ID to reset load: ", tId, sizeof(tId), false);
            Truck* t = findTruckById(trucks, count, tId);
            if (t) {
                t->current_load_kg = 0.0f;
                strcpy(t->status, "Available");
                updateTruck(t);
                printf("\n[SUCCESS] Truck %s load reset to 0.0 kg (Ready for next shift).\n", t->id);
            } else {
                printf("[!] Truck not found.\n");
            }
            pausePrompt();
        } else if (choice == 3) {
            char tId[32];
            getString("\n Enter Truck ID to decommission: ", tId, sizeof(tId), false);
            if (deleteTruck(tId)) {
                printf("\n[SUCCESS] Truck %s removed from fleet.\n", tId);
            } else {
                printf("[!] Truck not found.\n");
            }
            pausePrompt();
        } else {
            break;
        }
    }
}

/* ==================== 12. MANAGE PROBLEM REPORTS ==================== */

void manageProblemReportsUI(void) {
    while (1) {
        clearScreen();
        printHeader("MANAGE INCIDENT & PROBLEM REPORTS");
        printf(" Review and update handling status of reported issues.\n\n");
        printf("   [1] Citizen Public Bin Reports\n");
        printf("   [2] Field Collector Incident Reports\n");
        printf("   [3] Return\n");
        printDivider('-', 60);

        int choice = getInt(" Select Option [1-3]: ", 1, 3);
        if (choice == 1) {
            clearScreen();
            printHeader("CITIZEN PUBLIC BIN REPORTS");
            BinProblemReport reports[MAX_BIN_REPORTS];
            int count = loadBinReports(reports, MAX_BIN_REPORTS);

            printf("%-10s | %-10s | %-10s | %-24s | %-14s\n",
                   "Rep ID", "User ID", "Bin ID", "Problem Type", "Status");
            printDivider('-', 75);

            for (int i = 0; i < count; i++) {
                printf("%-10s | %-10s | %-10s | %-24.24s | %-14s\n",
                       reports[i].id, reports[i].user_id, reports[i].bin_id,
                       reports[i].problem_type, reports[i].status);
            }

            printf("\nOptions: [1] Update Report Status  [2] Back\n");
            int sub = getInt(" Select [1-2]: ", 1, 2);
            if (sub == 1) {
                char repId[32];
                getString(" Enter Report ID to update: ", repId, sizeof(repId), false);
                bool found = false;
                for (int i = 0; i < count; i++) {
                    if (strcmp(reports[i].id, repId) == 0) {
                        printf(" Description: %s\n", reports[i].description);
                        printf(" New Status: [1] Under Review  [2] Resolved  [3] Dismissed\n");
                        int st = getInt(" Select [1-3]: ", 1, 3);
                        if (st == 1) strcpy(reports[i].status, "Under Review");
                        else if (st == 2) strcpy(reports[i].status, "Resolved");
                        else strcpy(reports[i].status, "Dismissed");
                        updateBinReport(&reports[i]);
                        printf("\n[SUCCESS] Report %s status updated to '%s'.\n", reports[i].id, reports[i].status);
                        found = true;
                        break;
                    }
                }
                if (!found) printf("[!] Report ID not found.\n");
                pausePrompt();
            }
        } else if (choice == 2) {
            clearScreen();
            printHeader("FIELD COLLECTOR INCIDENT REPORTS");
            CollectorProblemReport cReports[MAX_COLLECTOR_REPORTS];
            int count = loadCollectorReports(cReports, MAX_COLLECTOR_REPORTS);

            printf("%-10s | %-12s | %-10s | %-24s | %-12s\n",
                   "Report ID", "Collector", "Task ID", "Issue Category", "Status");
            printDivider('-', 75);

            for (int i = 0; i < count; i++) {
                printf("%-10s | %-12s | %-10s | %-24.24s | %-12s\n",
                       cReports[i].id, cReports[i].collector_id, cReports[i].request_id,
                       cReports[i].problem_type, cReports[i].status);
            }

            printf("\nOptions: [1] Update Incident Status  [2] Back\n");
            int sub = getInt(" Select [1-2]: ", 1, 2);
            if (sub == 1) {
                char cRepId[32];
                getString(" Enter Incident Report ID to update: ", cRepId, sizeof(cRepId), false);
                bool found = false;
                for (int i = 0; i < count; i++) {
                    if (strcmp(cReports[i].id, cRepId) == 0) {
                        printf(" Incident Details: %s\n", cReports[i].description);
                        printf(" Set Status: [1] Under Review  [2] Resolved\n");
                        int st = getInt(" Select [1-2]: ", 1, 2);
                        if (st == 1) strcpy(cReports[i].status, "Under Review");
                        else strcpy(cReports[i].status, "Resolved");
                        updateCollectorReport(&cReports[i]);
                        printf("\n[SUCCESS] Incident %s status updated to '%s'.\n", cReports[i].id, cReports[i].status);
                        found = true;
                        break;
                    }
                }
                if (!found) printf("[!] Report not found.\n");
                pausePrompt();
            }
        } else {
            break;
        }
    }
}

/* ==================== 14. STATISTICAL REPORTS & ANALYTICS ==================== */

void generateSystemReportsUI(void) {
    while (1) {
        clearScreen();
        printHeader("STATISTICAL REPORTS & PERFORMANCE ANALYTICS");
        printf(" Select report category to generate:\n\n");
        printf("   [1] Waste Collection Volume Statistics by Category\n");
        printf("   [2] Citizen Reward Points Ledger & Distribution Report\n");
        printf("   [3] Field Collector Performance Report\n");
        printf("   [4] Vehicle Fleet Utilization & Load Report\n");
        printf("   [5] Admin Points & Commercial Sales Revenue Report\n");
        printf("   [6] Return to Dashboard\n");
        printDivider('-', 65);

        int choice = getInt(" Select Report [1-6]: ", 1, 6);
        if (choice == 1) {
            clearScreen();
            printHeader("WASTE VOLUME STATISTICS BY CATEGORY");
            static WasteRequest reqs[MAX_REQUESTS];
            int count = loadRequests(reqs, MAX_REQUESTS);

            float totalWeight = 0.0f;
            float plasticWt = 0.0f, organicWt = 0.0f, paperWt = 0.0f;
            float metalWt = 0.0f, glassWt = 0.0f, ewasteWt = 0.0f, chemWt = 0.0f;
            int totalCollections = 0;

            for (int i = 0; i < count; i++) {
                if (reqs[i].status >= REQ_STATUS_COLLECTED) {
                    float w = reqs[i].actual_weight;
                    totalWeight += w;
                    totalCollections++;
                    if (strstr(reqs[i].waste_type, "Plastic")) plasticWt += w;
                    else if (strstr(reqs[i].waste_type, "Organic")) organicWt += w;
                    else if (strstr(reqs[i].waste_type, "Paper")) paperWt += w;
                    else if (strstr(reqs[i].waste_type, "Metal")) metalWt += w;
                    else if (strstr(reqs[i].waste_type, "Glass")) glassWt += w;
                    else if (strstr(reqs[i].waste_type, "E-Waste")) ewasteWt += w;
                    else if (strstr(reqs[i].waste_type, "Hazardous") || strstr(reqs[i].waste_type, "Chemical")) chemWt += w;
                }
            }

            printf(" Total Completed Collections : %d batches\n", totalCollections);
            printf(" Total Waste Collected       : %.2f kg (%.2f metric tons)\n", totalWeight, totalWeight / 1000.0f);
            printDivider('-', 60);
            printf(" Breakdown by Material Category:\n");
            printf("   * Recyclable Plastic      : %8.2f kg (%5.1f%%)\n", plasticWt, totalWeight > 0 ? (plasticWt / totalWeight) * 100.0f : 0);
            printf("   * Organic / Compostable   : %8.2f kg (%5.1f%%)\n", organicWt, totalWeight > 0 ? (organicWt / totalWeight) * 100.0f : 0);
            printf("   * Paper & Cardboard       : %8.2f kg (%5.1f%%)\n", paperWt, totalWeight > 0 ? (paperWt / totalWeight) * 100.0f : 0);
            printf("   * Metal / Cans            : %8.2f kg (%5.1f%%)\n", metalWt, totalWeight > 0 ? (metalWt / totalWeight) * 100.0f : 0);
            printf("   * Glass Recyclables       : %8.2f kg (%5.1f%%)\n", glassWt, totalWeight > 0 ? (glassWt / totalWeight) * 100.0f : 0);
            printf("   * Electronic Waste        : %8.2f kg (%5.1f%%)\n", ewasteWt, totalWeight > 0 ? (ewasteWt / totalWeight) * 100.0f : 0);
            printf("   * Chemical / Hazardous    : %8.2f kg (%5.1f%%)\n", chemWt, totalWeight > 0 ? (chemWt / totalWeight) * 100.0f : 0);
            printDivider('=', 60);
            pausePrompt();
        } else if (choice == 2) {
            clearScreen();
            printHeader("REWARD POINTS SUMMARY & DISTRIBUTION");
            User users[MAX_USERS];
            int uCount = loadUsers(users, MAX_USERS);
            int totalPoints = 0;

            for (int i = 0; i < uCount; i++) {
                totalPoints += users[i].reward_points;
            }

            RewardEntry rewards[MAX_REWARDS];
            int rCount = loadRewards(rewards, MAX_REWARDS);

            printf(" Total Registered Citizens: %d\n", uCount);
            printf(" Total Reward Points Held : %d pts (Available Liability)\n", totalPoints);
            printf(" Total Reward Grants      : %d transactions verified\n", rCount);
            printDivider('-', 60);
            printf(" Top Contributing Citizens:\n");

            for (int i = 0; i < uCount && i < 5; i++) {
                printf("   #%d: %-20s (ID: %s) -> %d pts\n",
                       i + 1, users[i].name, users[i].id, users[i].reward_points);
            }
            printDivider('=', 60);
            pausePrompt();
        } else if (choice == 3) {
            clearScreen();
            printHeader("COLLECTOR FIELD PERFORMANCE REPORT");
            Collector collectors[MAX_COLLECTORS];
            int count = loadCollectors(collectors, MAX_COLLECTORS);

            printf("%-10s | %-20s | %-12s | %-14s | %-12s\n",
                   "Officer ID", "Officer Name", "Vehicle", "Tasks Done", "Total Haul");
            printDivider('-', 75);

            for (int i = 0; i < count; i++) {
                printf("%-10s | %-20.20s | %-12s | %-14d | %-6.2f kg\n",
                       collectors[i].id, collectors[i].name, collectors[i].assigned_truck,
                       collectors[i].tasks_completed, collectors[i].total_collected_kg);
            }
            printDivider('=', 75);
            pausePrompt();
        } else if (choice == 4) {
            clearScreen();
            printHeader("FLEET PERFORMANCE & CAPACITY UTILIZATION");
            Truck trucks[MAX_TRUCKS];
            int count = loadTrucks(trucks, MAX_TRUCKS);

            printf("%-10s | %-16s | %-10s | %-10s | %-12s | %-8s\n",
                   "Truck ID", "Plate No.", "Capacity", "Current Load", "Status", "Utilization");
            printDivider('-', 80);

            for (int i = 0; i < count; i++) {
                float util = (trucks[i].capacity_kg > 0) ? (trucks[i].current_load_kg / trucks[i].capacity_kg) * 100.0f : 0;
                printf("%-10s | %-16s | %-6.0f kg | %-6.1f kg  | %-12s | %5.1f%%\n",
                       trucks[i].id, trucks[i].plate_number, trucks[i].capacity_kg,
                       trucks[i].current_load_kg, trucks[i].status, util);
            }
            printDivider('=', 80);
            pausePrompt();
        } else if (choice == 5) {
            clearScreen();
            printHeader("ADMIN PORTAL POINTS & COMMERCIAL SALES REVENUE REPORT");
            static Admin admins[MAX_ADMINS];
            int aCount = loadAdmins(admins, MAX_ADMINS);

            printf(" Administrator Recycling & Sales Performance:\n");
            printf("%-10s | %-22s | %-20s | %-10s | %-12s\n",
                   "Admin ID", "Officer Name", "Role", "Admin Pts", "Sales Rev");
            printDivider('-', 84);

            int totalAdminPts = 0;
            float totalAdminRev = 0.0f;
            for (int i = 0; i < aCount; i++) {
                printf("%-10s | %-22.22s | %-20.20s | %-10d | $%-10.2f\n",
                       admins[i].id, admins[i].name, admins[i].role,
                       admins[i].admin_points, admins[i].total_revenue);
                totalAdminPts += admins[i].admin_points;
                totalAdminRev += admins[i].total_revenue;
            }
            printDivider('-', 84);
            printf(" TOTAL ADMIN CREDITS: %d pts | ADMIN-TRACKED REVENUE: $%.2f\n\n",
                   totalAdminPts, totalAdminRev);

            static RecyclableSale sales[MAX_SALES];
            int sCount = loadSales(sales, MAX_SALES);
            printf(" Commercial Recyclable Sales Ledger (%d transactions):\n", sCount);
            printf("%-10s | %-10s | %-22s | %-9s | %-18s | %-10s\n",
                   "Sale ID", "Batch ID", "Material", "Weight", "Buyer", "Total ($)");
            printDivider('-', 90);
            float grandSales = 0.0f;
            for (int i = 0; i < sCount; i++) {
                printf("%-10s | %-10s | %-22.22s | %-6.1f kg | %-18.18s | $%-9.2f\n",
                       sales[i].id, sales[i].batch_id, sales[i].material_type,
                       sales[i].weight_kg, sales[i].buyer_name, sales[i].total_amount);
                grandSales += sales[i].total_amount;
            }
            printDivider('=', 90);
            printf(" GRAND COMMERCIAL SALES REVENUE: $%.2f\n", grandSales);
            pausePrompt();
        } else {
            break;
        }
    }
}
