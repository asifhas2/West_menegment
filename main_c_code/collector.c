#include "collector.h"
#include "data.h"
#include "utils.h"

/* Common field problem categories encountered during collection routes */
static const char* FIELD_PROBLEM_TYPES[] = {
    "Citizen Absent / Door Locked",
    "Narrow Road / Vehicle Access Blocked",
    "Hazardous or Contaminated Waste Mixture",
    "Actual Weight Far Exceeds Estimate",
    "Waste Missing or Already Removed",
    "Other Logistical Impediment"
};
static const int NUM_FIELD_PROBLEMS = 6;

/* Helper to compute reward points based on verified material type and weight */
static int calculatePoints(const char* waste_type, float weight) {
    if (strstr(waste_type, "Plastic") || strstr(waste_type, "Metal")) {
        return (int)(weight * 10.0f);
    } else if (strstr(waste_type, "Organic")) {
        return (int)(weight * 5.0f);
    } else if (strstr(waste_type, "E-Waste")) {
        return (int)(weight * 15.0f);
    } else if (strstr(waste_type, "Paper")) {
        return (int)(weight * 8.0f);
    } else if (strstr(waste_type, "Hazardous") || strstr(waste_type, "Chemical")) {
        return (int)(weight * 20.0f);
    }
    return (int)(weight * 5.0f);
}

/* ==================== COLLECTOR ENTRY & AUTHENTICATION ==================== */

void handleCollectorModule(void) {
    while (1) {
        clearScreen();
        printHeader("WASTE COLLECTOR PORTAL - FIELD OPERATIONS");
        printf(" Welcome, Waste Operations Crew!\n");
        printf(" Please choose an action:\n\n");
        printf("   [1] Collector Login\n");
        printf("   [2] Collector Registration (Join Collection Crew)\n");
        printf("   [3] Return to Main Menu\n");
        printDivider('-', 70);

        int choice = getInt(" Enter your choice [1-3]: ", 1, 3);
        if (choice == 1) {
            Collector loggedInCollector;
            if (handleCollectorLogin(&loggedInCollector)) {
                showCollectorDashboard(&loggedInCollector);
            }
        } else if (choice == 2) {
            handleCollectorRegistration();
        } else {
            break;
        }
    }
}

bool handleCollectorRegistration(void) {
    clearScreen();
    printHeader("FIELD COLLECTOR REGISTRATION");
    printf(" Register a new collector profile for municipal field operations.\n\n");

    Collector newCol;
    memset(&newCol, 0, sizeof(newCol));

    generateNextCollectorId(newCol.id);
    printf(" Assigned Collector ID: %s\n\n", newCol.id);

    getString(" Enter Full Name: ", newCol.name, sizeof(newCol.name), false);

    /* Validate email uniqueness */
    Collector collectors[MAX_COLLECTORS];
    int count = loadCollectors(collectors, MAX_COLLECTORS);

    while (1) {
        getString(" Enter Email Address: ", newCol.email, sizeof(newCol.email), false);
        if (!isValidEmailOrPhone(newCol.email)) {
            printf("[!] Invalid email format. Please include '@' and a domain.\n");
            continue;
        }
        if (findCollectorByIdOrEmail(collectors, count, newCol.email)) {
            printf("[!] A collector with this email already exists! Please use another.\n");
            continue;
        }
        break;
    }

    while (1) {
        getString(" Enter Contact Phone: ", newCol.phone, sizeof(newCol.phone), false);
        if (!isValidEmailOrPhone(newCol.phone)) {
            printf("[!] Invalid phone number. Please enter digits.\n");
            continue;
        }
        bool phoneExists = false;
        for (int i = 0; i < count; i++) {
            if (strcmp(collectors[i].phone, newCol.phone) == 0) {
                phoneExists = true;
                break;
            }
        }
        if (phoneExists) {
            printf("[!] A collector with this phone number already exists! Please use another.\n");
            continue;
        }
        break;
    }

    getString(" Create Password: ", newCol.password, sizeof(newCol.password), false);

    /* Select Default Assigned Truck */
    Truck trucks[MAX_TRUCKS];
    int truckCount = loadTrucks(trucks, MAX_TRUCKS);
    if (truckCount > 0) {
        printf("\n Available Fleet Trucks:\n");
        for (int i = 0; i < truckCount; i++) {
            printf("   [%d] %s (%s) - Capacity: %.0f kg\n",
                   i + 1, trucks[i].id, trucks[i].plate_number, trucks[i].capacity_kg);
        }
        int trkChoice = getInt(" Select Assigned Truck [1 to count]: ", 1, truckCount);
        strncpy(newCol.assigned_truck, trucks[trkChoice - 1].id, sizeof(newCol.assigned_truck) - 1);
    } else {
        strcpy(newCol.assigned_truck, "TRK-01");
    }

    strcpy(newCol.status, "Available");
    newCol.total_collected_kg = 0.0f;
    newCol.tasks_completed = 0;

    printf("\n");
    printDivider('-', 50);
    printf(" Collector Profile Summary:\n");
    printf("   Collector ID   : %s\n", newCol.id);
    printf("   Officer Name   : %s\n", newCol.name);
    printf("   Email Address  : %s\n", newCol.email);
    printf("   Contact Phone  : %s\n", newCol.phone);
    printf("   Assigned Truck : %s\n", newCol.assigned_truck);
    printDivider('-', 50);

    char confirm[16];
    getString(" Confirm collector registration? (Y/N): ", confirm, sizeof(confirm), false);
    if (confirm[0] == 'Y' || confirm[0] == 'y') {
        if (addCollector(&newCol)) {
            printf("\n[SUCCESS] Registration completed successfully!\n");
            printf("Your Collector ID is: %s\n", newCol.id);
            printf("You can now login using your Collector ID or Email and password.\n");
        } else {
            printf("\n[ERROR] Failed to save collector details.\n");
        }
    } else {
        printf("\n[INFO] Registration cancelled.\n");
    }

    pausePrompt();
    return true;
}

bool handleCollectorLogin(Collector* loggedInCollector) {
    clearScreen();
    printHeader("FIELD COLLECTOR LOGIN");

    char identifier[64];
    char password[64];

    getString(" Enter Collector ID or Email: ", identifier, sizeof(identifier), false);
    getString(" Enter Password: ", password, sizeof(password), false);

    Collector collectors[MAX_COLLECTORS];
    int count = loadCollectors(collectors, MAX_COLLECTORS);

    Collector* col = findCollectorByIdOrEmail(collectors, count, identifier);
    if (!col || strcmp(col->password, password) != 0) {
        printf("\n[!] Error: Invalid credentials! Please check your Collector ID/Email and password.\n");
        pausePrompt();
        return false;
    }

    *loggedInCollector = *col;
    printf("\n[SUCCESS] Login successful! Welcome back, Officer %s.\n", loggedInCollector->name);
    pausePrompt();
    return true;
}

/* ==================== COLLECTOR DASHBOARD ==================== */

void showCollectorDashboard(Collector* collector) {
    while (1) {
        /* Sync fresh collector data from disk */
        Collector currentList[MAX_COLLECTORS];
        int colCount = loadCollectors(currentList, MAX_COLLECTORS);
        Collector* fresh = findCollectorById(currentList, colCount, collector->id);
        if (fresh) {
            *collector = *fresh;
        }

        /* Count active tasks for this collector */
        static WasteRequest reqs[MAX_REQUESTS];
        int reqCount = loadRequests(reqs, MAX_REQUESTS);
        int assignedCount = 0;
        int inProgressCount = 0;

        for (int i = 0; i < reqCount; i++) {
            if (strstr(reqs[i].assigned_collector, collector->id) != NULL) {
                if (reqs[i].status == REQ_STATUS_ASSIGNED) {
                    assignedCount++;
                } else if (reqs[i].status == REQ_STATUS_IN_PROGRESS) {
                    inProgressCount++;
                }
            }
        }

        clearScreen();
        printHeader("COLLECTOR FIELD DASHBOARD");
        printf(" Officer: %s (ID: %s) | Truck: %s\n", collector->name, collector->id, collector->assigned_truck);
        printf(" Status: %s | Tasks Completed: %d | Total Haul: %.2f kg\n",
               collector->status, collector->tasks_completed, collector->total_collected_kg);
        printf(" Active Route Tasks: %d Assigned | %d In Progress\n", assignedCount, inProgressCount);
        printDivider('=', 70);
        printf(" Actions Available:\n");
        printf("   [1] View Assigned Tasks\n");
        printf("   [2] View Task Details by Request ID\n");
        printf("   [3] View Collection Route & Stops\n");
        printf("   [4] Start Collection (Mark In Progress)\n");
        printf("   [5] Record Verified Weight & Complete Collection\n");
        printf("   [6] Report a Collection Problem (Road/Hazard/Resident)\n");
        printf("   [7] View My Collection Performance Summary\n");
        printf("   [8] Logout to Collector Menu\n");
        printDivider('-', 70);

        int choice = getInt(" Select an action [1-8]: ", 1, 8);
        switch (choice) {
            case 1:
                viewAssignedTasksUI(collector);
                break;
            case 2:
                viewTaskDetailsUI(collector);
                break;
            case 3:
                viewCollectionRouteUI(collector);
                break;
            case 4:
                startCollectionUI(collector);
                break;
            case 5:
                completeCollectionUI(collector);
                break;
            case 6:
                reportCollectionProblemUI(collector);
                break;
            case 7:
                viewCollectorPerformanceUI(collector);
                break;
            case 8:
                return;
        }
    }
}

/* ==================== VIEW ASSIGNED TASKS ==================== */

void viewAssignedTasksUI(const Collector* collector) {
    clearScreen();
    printHeader("ASSIGNED COLLECTION TASKS");
    printf(" Listing tasks assigned to %s (%s):\n\n", collector->name, collector->id);

    static WasteRequest reqs[MAX_REQUESTS];
    int count = loadRequests(reqs, MAX_REQUESTS);
    int assignedFound = 0;

    printf("%-10s | %-24s | %-8s | %-22s | %-14s\n",
           "Req ID", "Waste Type", "Est. Wt", "Pickup Area", "Status");
    printDivider('-', 80);

    for (int i = 0; i < count; i++) {
        if (strstr(reqs[i].assigned_collector, collector->id) != NULL) {
            printf("%-10s | %-24s | %-5.1f kg | %-22.22s | %-14s\n",
                   reqs[i].id,
                   reqs[i].waste_type,
                   reqs[i].estimated_weight,
                   reqs[i].location,
                   getStatusColorTag(reqs[i].status));
            assignedFound++;
        }
    }

    if (assignedFound == 0) {
        printf(" No active tasks assigned to your schedule at this time.\n");
    }

    printDivider('=', 80);
    pausePrompt();
}

/* ==================== VIEW TASK DETAILS ==================== */

void viewTaskDetailsUI(const Collector* collector) {
    clearScreen();
    printHeader("VIEW COLLECTION TASK DETAILS");

    char reqId[32];
    getString(" Enter Request ID to inspect: ", reqId, sizeof(reqId), false);

    static WasteRequest reqs[MAX_REQUESTS];
    int count = loadRequests(reqs, MAX_REQUESTS);
    WasteRequest* req = findRequestById(reqs, count, reqId);

    if (!req) {
        printf("\n[!] Task ID '%s' was not found.\n", reqId);
        pausePrompt();
        return;
    }

    if (strstr(req->assigned_collector, collector->id) == NULL) {
        printf("\n[!] Note: This task is currently assigned to '%s', not your account.\n", req->assigned_collector);
    }

    printf("\n");
    printDivider('=', 70);
    printf(" TASK DETAILS: %s\n", req->id);
    printDivider('=', 70);
    printf("  Request ID          : %s\n", req->id);
    printf("  Citizen ID          : %s\n", req->user_id);
    printf("  Waste Category      : %s\n", req->waste_type);
    printf("  Estimated Quantity  : %.2f kg\n", req->estimated_weight);
    if (req->status >= REQ_STATUS_COLLECTED) {
        printf("  Actual Weight       : %.2f kg (Verified)\n", req->actual_weight);
    }
    printf("  Pickup Location     : %s\n", req->location);
    printf("  Smart Bin Reference : %s\n", req->bin_id);
    printf("  Preferred Window    : %s during %s\n", req->preferred_date, req->preferred_slot);
    printf("  Citizen Notes       : %s\n", req->special_notes);
    printf("  Current Status      : %s %s\n", getStatusName(req->status), getStatusColorTag(req->status));
    printf("  Assigned Collector  : %s\n", req->assigned_collector);
    printf("  Assigned Vehicle    : %s\n", req->assigned_truck);
    printDivider('-', 70);
    printProgressBar(req->status);

    pausePrompt();
}

/* ==================== VIEW COLLECTION ROUTE ==================== */

void viewCollectionRouteUI(const Collector* collector) {
    clearScreen();
    printHeader("OPTIMIZED COLLECTION ROUTE & WAYPOINTS");
    printf(" Vehicle: %s | Officer: %s\n\n", collector->assigned_truck, collector->name);

    static WasteRequest reqs[MAX_REQUESTS];
    int count = loadRequests(reqs, MAX_REQUESTS);
    int stopNumber = 1;

    for (int i = 0; i < count; i++) {
        if (strstr(reqs[i].assigned_collector, collector->id) != NULL &&
            (reqs[i].status == REQ_STATUS_ASSIGNED || reqs[i].status == REQ_STATUS_IN_PROGRESS)) {
            printf(" [STOP #%d] Task: %s\n", stopNumber++, reqs[i].id);
            printf("   * Location : %s (Bin: %s)\n", reqs[i].location, reqs[i].bin_id);
            printf("   * Material : %s (Est: %.1f kg)\n", reqs[i].waste_type, reqs[i].estimated_weight);
            printf("   * Time Slot: %s\n", reqs[i].preferred_slot);
            printf("   * Status   : %s\n", getStatusName(reqs[i].status));
            printDivider('-', 65);
        }
    }

    if (stopNumber == 1) {
        printf(" No pending route stops currently scheduled for you.\n");
    } else {
        printf(" Drive safely and follow municipal safety regulations.\n");
    }

    pausePrompt();
}

/* ==================== START COLLECTION ==================== */

void startCollectionUI(Collector* collector) {
    clearScreen();
    printHeader("START COLLECTION - MARK IN PROGRESS");
    printf(" Select a task to mark as active (en-route or on-site).\n\n");

    static WasteRequest reqs[MAX_REQUESTS];
    int count = loadRequests(reqs, MAX_REQUESTS);
    int eligible = 0;

    for (int i = 0; i < count; i++) {
        if (strstr(reqs[i].assigned_collector, collector->id) != NULL &&
            reqs[i].status == REQ_STATUS_ASSIGNED) {
            printf("  [%s] %s | Est: %.1f kg | %s\n",
                   reqs[i].id, reqs[i].waste_type, reqs[i].estimated_weight, reqs[i].location);
            eligible++;
        }
    }

    if (eligible == 0) {
        printf(" No tasks currently in 'Assigned' state ready to start.\n");
        pausePrompt();
        return;
    }

    printf("\n");
    char reqId[32];
    getString(" Enter Request ID to begin collection: ", reqId, sizeof(reqId), false);

    WasteRequest* target = findRequestById(reqs, count, reqId);
    if (!target) {
        printf("[!] Request not found.\n");
        pausePrompt();
        return;
    }

    if (strstr(target->assigned_collector, collector->id) == NULL) {
        printf("[!] This task is assigned to a different collector.\n");
        pausePrompt();
        return;
    }

    if (target->status != REQ_STATUS_ASSIGNED) {
        printf("[!] Task is not in 'Assigned' stage (Current: %s).\n", getStatusName(target->status));
        pausePrompt();
        return;
    }

    target->status = REQ_STATUS_IN_PROGRESS;
    strcpy(target->final_outcome, "Collection In Progress - Officer On Route");

    if (updateRequest(target)) {
        printf("\n[SUCCESS] Request %s is now IN PROGRESS!\n", target->id);
        printf("The citizen can now see that you are en-route to their location.\n");
    } else {
        printf("\n[ERROR] Failed to update request status.\n");
    }

    pausePrompt();
}

/* ==================== COMPLETE COLLECTION & WEIGH ==================== */

void completeCollectionUI(Collector* collector) {
    clearScreen();
    printHeader("COMPLETE COLLECTION & VERIFY WEIGHT");
    printf(" Finalize collection, record actual scale weight, and award points.\n\n");

    static WasteRequest reqs[MAX_REQUESTS];
    int count = loadRequests(reqs, MAX_REQUESTS);
    int activeFound = 0;

    for (int i = 0; i < count; i++) {
        if (strstr(reqs[i].assigned_collector, collector->id) != NULL &&
            (reqs[i].status == REQ_STATUS_ASSIGNED || reqs[i].status == REQ_STATUS_IN_PROGRESS)) {
            printf("  [%s] %s | Est: %.1f kg | Location: %s (%s)\n",
                   reqs[i].id, reqs[i].waste_type, reqs[i].estimated_weight,
                   reqs[i].location, getStatusName(reqs[i].status));
            activeFound++;
        }
    }

    if (activeFound == 0) {
        printf(" No active collections currently pending completion.\n");
        pausePrompt();
        return;
    }

    printf("\n");
    char reqId[32];
    getString(" Enter Request ID to finalize: ", reqId, sizeof(reqId), false);

    WasteRequest* req = findRequestById(reqs, count, reqId);
    if (!req) {
        printf("[!] Request ID not found.\n");
        pausePrompt();
        return;
    }

    if (strstr(req->assigned_collector, collector->id) == NULL) {
        printf("[!] Access Denied: This task is assigned to another officer.\n");
        pausePrompt();
        return;
    }

    printf("\n Task Summary:\n");
    printf("   Material Type : %s\n", req->waste_type);
    printf("   Est. Weight   : %.2f kg\n", req->estimated_weight);
    printf("   Citizen ID    : %s\n", req->user_id);
    printDivider('-', 50);

    /* Record actual physical weight */
    float actualWeight = getFloat(" Enter Actual Measured Weight from Scale (0.1 - 500.0 kg): ", 0.1f, 500.0f);

    /* Calculate rewards */
    int pointsEarned = calculatePoints(req->waste_type, actualWeight);

    printf("\n Verification Summary:\n");
    printf("   Actual Weight  : %.2f kg\n", actualWeight);
    printf("   Points to Award: %d points\n", pointsEarned);
    printDivider('-', 50);

    char confirm[16];
    getString(" Confirm completion and award reward points? (Y/N): ", confirm, sizeof(confirm), false);
    if (confirm[0] != 'Y' && confirm[0] != 'y') {
        printf("\n[INFO] Collection completion aborted.\n");
        pausePrompt();
        return;
    }

    /* 1. Update Request */
    req->actual_weight = actualWeight;
    req->points_awarded = pointsEarned;
    req->status = REQ_STATUS_COLLECTED;
    strcpy(req->final_outcome, "Collected - Transferred to Municipal Sorting Facility");
    getCurrentTimestamp(req->completed_at, sizeof(req->completed_at));
    updateRequest(req);

    /* 2. Credit Citizen Account & Add Reward Ledger Entry */
    User users[MAX_USERS];
    int userCount = loadUsers(users, MAX_USERS);
    User* citizen = findUserById(users, userCount, req->user_id);
    if (citizen) {
        citizen->reward_points += pointsEarned;
        updateUser(citizen);

        RewardEntry rew;
        generateNextRewardId(rew.id);
        strncpy(rew.user_id, citizen->id, sizeof(rew.user_id) - 1);
        strncpy(rew.request_id, req->id, sizeof(rew.request_id) - 1);
        strncpy(rew.waste_type, req->waste_type, sizeof(rew.waste_type) - 1);
        rew.weight = actualWeight;
        rew.points = pointsEarned;
        getCurrentTimestamp(rew.date, sizeof(rew.date));
        addReward(&rew);
    }

    /* 3. Update Collector Performance Stats */
    collector->total_collected_kg += actualWeight;
    collector->tasks_completed += 1;
    updateCollector(collector);

    /* 4. Update Truck Payload */
    Truck trucks[MAX_TRUCKS];
    int truckCount = loadTrucks(trucks, MAX_TRUCKS);
    Truck* truck = findTruckById(trucks, truckCount, collector->assigned_truck);
    if (truck) {
        truck->current_load_kg += actualWeight;
        updateTruck(truck);
    }

    printf("\n[SUCCESS] Collection for %s completed successfully!\n", req->id);
    printf("Citizen %s has been credited with +%d reward points.\n", req->user_id, pointsEarned);
    printf("Updated truck load: %.2f kg\n", truck ? truck->current_load_kg : actualWeight);

    pausePrompt();
}

/* ==================== REPORT COLLECTION PROBLEM ==================== */

void reportCollectionProblemUI(const Collector* collector) {
    clearScreen();
    printHeader("REPORT FIELD COLLECTION PROBLEM");
    printf(" Submit an issue encountered on the route for administrative review.\n\n");

    CollectorProblemReport report;
    memset(&report, 0, sizeof(report));

    generateNextCollectorReportId(report.id);
    strncpy(report.collector_id, collector->id, sizeof(report.collector_id) - 1);

    getString(" Enter Relevant Request ID (or 'GENERAL'): ", report.request_id, sizeof(report.request_id), false);

    printf("\n Select Problem Category:\n");
    for (int i = 0; i < NUM_FIELD_PROBLEMS; i++) {
        printf("   [%d] %s\n", i + 1, FIELD_PROBLEM_TYPES[i]);
    }
    int pChoice = getInt(" Select Problem Type [1-6]: ", 1, NUM_FIELD_PROBLEMS);
    strncpy(report.problem_type, FIELD_PROBLEM_TYPES[pChoice - 1], sizeof(report.problem_type) - 1);

    getString(" Detailed Incident Description: ", report.description, sizeof(report.description), false);
    strcpy(report.status, "Open");
    getCurrentTimestamp(report.created_at, sizeof(report.created_at));

    if (addCollectorReport(&report)) {
        printf("\n[SUCCESS] Incident report %s logged successfully.\n", report.id);
        printf("Municipal dispatchers have been alerted.\n");
    } else {
        printf("\n[ERROR] Unable to save incident report.\n");
    }

    pausePrompt();
}

/* ==================== VIEW COLLECTOR PERFORMANCE ==================== */

void viewCollectorPerformanceUI(const Collector* collector) {
    clearScreen();
    printHeader("COLLECTOR FIELD PERFORMANCE SUMMARY");
    printf("  Officer Name        : %s\n", collector->name);
    printf("  Badge / ID          : %s\n", collector->id);
    printf("  Phone               : %s\n", collector->phone);
    printf("  Assigned Vehicle    : %s\n", collector->assigned_truck);
    printf("  Duty Status         : %s\n", collector->status);
    printf("  Collections Done    : %d completed tasks\n", collector->tasks_completed);
    printf("  Total Haul Delivered: %.2f kg\n", collector->total_collected_kg);
    printDivider('-', 60);

    /* Fetch assigned vehicle capacity */
    Truck trucks[MAX_TRUCKS];
    int truckCount = loadTrucks(trucks, MAX_TRUCKS);
    Truck* truck = findTruckById(trucks, truckCount, collector->assigned_truck);
    if (truck) {
        printf("  Current Vehicle Load: %.2f / %.2f kg (%.1f%% capacity)\n",
               truck->current_load_kg, truck->capacity_kg,
               (truck->current_load_kg / truck->capacity_kg) * 100.0f);
    }
    printDivider('=', 60);

    pausePrompt();
}
