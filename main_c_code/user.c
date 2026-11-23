#include "user.h"
#include "data.h"
#include "utils.h"

/* Standard categories for residential and commercial waste */
static const char* WASTE_CATEGORIES[] = {
    "Organic / Biodegradable",
    "Recyclable - Plastic",
    "Recyclable - Paper & Cardboard",
    "Recyclable - Metal / Cans",
    "Recyclable - Glass",
    "Electronic Waste (E-Waste)",
    "Hazardous / Chemical Waste"
};
static const int NUM_WASTE_CATEGORIES = 7;

/* Standard municipal collection time windows */
static const char* TIME_SLOTS[] = {
    "Morning (8:00 AM - 12:00 PM)",
    "Afternoon (12:00 PM - 4:00 PM)",
    "Evening (4:00 PM - 7:00 PM)"
};
static const int NUM_TIME_SLOTS = 3;

/* Problem types commonly reported for public smart bins */
static const char* PROBLEM_TYPES[] = {
    "Bin Overflowing / Excess Garbage",
    "Damaged Lid or Bin Body",
    "Missing / Stolen Bin",
    "Strong Foul Odor / Health Hazard",
    "Chemical Leakage or Hazardous Spill",
    "Other Issue"
};
static const int NUM_PROBLEM_TYPES = 6;

/* ==================== CITIZEN PORTAL ENTRY ==================== */

void handleUserModule(void) {
    while (1) {
        clearScreen();
        printHeader("CITIZEN PORTAL - SMART WASTE MANAGEMENT");
        printf(" Welcome to the Waste Management & Recycling Portal!\n");
        printf(" Please choose an option:\n\n");
        printf("   [1] Citizen Login\n");
        printf("   [2] Citizen Registration (Create New Account)\n");
        printf("   [3] Return to Main Menu\n");
        printDivider('-', 70);

        int choice = getInt(" Enter your choice [1-3]: ", 1, 3);
        if (choice == 1) {
            User loggedInUser;
            if (handleUserLogin(&loggedInUser)) {
                showUserDashboard(&loggedInUser);
            }
        } else if (choice == 2) {
            handleUserRegistration();
        } else if (choice == 3) {
            break;
        }
    }
}

/* ==================== CITIZEN REGISTRATION ==================== */

bool handleUserRegistration(void) {
    clearScreen();
    printHeader("NEW CITIZEN REGISTRATION");
    printf(" Please provide your details to register as a citizen.\n\n");

    User newUser;
    memset(&newUser, 0, sizeof(newUser));

    generateNextUserId(newUser.id);
    printf(" Generated User ID: %s\n\n", newUser.id);

    getString(" Enter Full Name: ", newUser.name, sizeof(newUser.name), false);

    /* Check uniqueness of email and phone */
    User existingUsers[MAX_USERS];
    int userCount = loadUsers(existingUsers, MAX_USERS);

    while (1) {
        getString(" Enter Email Address: ", newUser.email, sizeof(newUser.email), false);
        if (!isValidEmailOrPhone(newUser.email)) {
            printf("[!] Invalid email format. Please include '@' and a domain.\n");
            continue;
        }
        if (findUserByEmailOrPhone(existingUsers, userCount, newUser.email)) {
            printf("[!] An account with this email already exists! Please use another.\n");
            continue;
        }
        break;
    }

    while (1) {
        getString(" Enter Phone Number: ", newUser.phone, sizeof(newUser.phone), false);
        if (!isValidEmailOrPhone(newUser.phone)) {
            printf("[!] Invalid phone number. Please enter a valid numerical phone number.\n");
            continue;
        }
        if (findUserByEmailOrPhone(existingUsers, userCount, newUser.phone)) {
            printf("[!] An account with this phone number already exists! Please use another.\n");
            continue;
        }
        break;
    }

    getString(" Create Password: ", newUser.password, sizeof(newUser.password), false);
    newUser.reward_points = 0;
    getCurrentTimestamp(newUser.created_at, sizeof(newUser.created_at));

    /* Registration review */
    printf("\n");
    printDivider('-', 50);
    printf(" Registration Summary:\n");
    printf("   User ID : %s\n", newUser.id);
    printf("   Name    : %s\n", newUser.name);
    printf("   Email   : %s\n", newUser.email);
    printf("   Phone   : %s\n", newUser.phone);
    printDivider('-', 50);

    char confirm[16];
    getString(" Confirm registration? (Y/N): ", confirm, sizeof(confirm), false);
    if (confirm[0] == 'Y' || confirm[0] == 'y') {
        if (addUser(&newUser)) {
            printf("\n[SUCCESS] Registration completed successfully!\n");
            printf("You can now login with your email/phone and password.\n");
        } else {
            printf("\n[ERROR] Failed to save user details to database.\n");
        }
    } else {
        printf("\n[INFO] Registration cancelled.\n");
    }

    pausePrompt();
    return true;
}

/* ==================== CITIZEN LOGIN & AUTHENTICATION ==================== */

bool handleUserLogin(User* loggedInUser) {
    clearScreen();
    printHeader("CITIZEN LOGIN");

    char identifier[64];
    char password[64];

    getString(" Enter Email or Phone: ", identifier, sizeof(identifier), false);
    getString(" Enter Password: ", password, sizeof(password), false);

    User users[MAX_USERS];
    int count = loadUsers(users, MAX_USERS);

    User* user = findUserByEmailOrPhone(users, count, identifier);
    if (!user || strcmp(user->password, password) != 0) {
        printf("\n[!] Error: Invalid credentials! Please check your email/phone and password.\n");
        pausePrompt();
        return false;
    }

    *loggedInUser = *user;
    printf("\n[SUCCESS] Login successful! Welcome back, %s.\n", loggedInUser->name);
    pausePrompt();
    return true;
}

/* ==================== CITIZEN DASHBOARD ==================== */

void showUserDashboard(User* user) {
    while (1) {
        /* Refresh user data to get updated points balance */
        User currentUsers[MAX_USERS];
        int uCount = loadUsers(currentUsers, MAX_USERS);
        User* freshUser = findUserById(currentUsers, uCount, user->id);
        if (freshUser) {
            *user = *freshUser;
        }

        /* Calculate current counts */
        static WasteRequest reqs[MAX_REQUESTS];
        int reqCount = loadRequests(reqs, MAX_REQUESTS);
        int totalUserReqs = 0;
        int activeUserReqs = 0;
        int completedUserReqs = 0;

        for (int i = 0; i < reqCount; i++) {
            if (strcmp(reqs[i].user_id, user->id) == 0) {
                totalUserReqs++;
                if (reqs[i].status == REQ_STATUS_FINAL_OUTCOME || reqs[i].status == REQ_STATUS_COLLECTED) {
                    completedUserReqs++;
                } else {
                    activeUserReqs++;
                }
            }
        }

        clearScreen();
        printHeader("CITIZEN WASTE MANAGEMENT DASHBOARD");
        printf(" Welcome, %s | ID: %s\n", user->name, user->id);
        printf(" Reward Points: %d pts | Active Requests: %d | Completed: %d\n",
               user->reward_points, activeUserReqs, completedUserReqs);
        printDivider('=', 70);
        printf(" Main Operations:\n");
        printf("   [1]  Submit New Waste Collection Request\n");
        printf("   [2]  Track Active Requests & View Status\n");
        printf("   [3]  View Request Details by ID\n");
        printf("   [4]  Report a Bin Problem (Full/Damaged/Leak)\n");
        printf("   [5]  View Collection History\n");
        printf("   [6]  View Reward Points & History\n");
        printf("   [7]  Eco-Store: Redeem Points for Items (Water Bottle, Dustbin, etc.)\n");
        printf("   [8]  View My Eco-Store Purchase History\n");
        printf("   [9]  View Final Waste Outcomes & Transparency Trail\n");
        printf("   [10] View My Profile Information\n");
        printf("   [11] Logout to Citizen Menu\n");
        printDivider('-', 70);

        int choice = getInt(" Select an action [1-11]: ", 1, 11);
        switch (choice) {
            case 1:
                submitWasteRequestUI(user);
                break;
            case 2:
                trackUserRequestsUI(user);
                break;
            case 3:
                viewRequestDetailsUI(user);
                break;
            case 4:
                reportBinProblemUI(user);
                break;
            case 5:
                viewCollectionHistoryUI(user);
                break;
            case 6:
                viewRewardSummaryUI(user);
                break;
            case 7:
                redeemPointsUI(user);
                break;
            case 8:
                viewPurchaseHistoryUI(user);
                break;
            case 9:
                viewWasteOutcomeTransparencyUI(user);
                break;
            case 10:
                viewUserProfileUI(user);
                break;
            case 11:
                return;
        }
    }
}

/* ==================== SUBMIT WASTE REQUEST ==================== */

void submitWasteRequestUI(User* user) {
    clearScreen();
    printHeader("SUBMIT WASTE COLLECTION REQUEST");
    printf(" Follow the steps below to schedule a verified waste collection.\n\n");

    WasteRequest req;
    memset(&req, 0, sizeof(req));

    generateNextRequestId(req.id);
    strncpy(req.user_id, user->id, sizeof(req.user_id) - 1);

    /* 1. Select Waste Type */
    printf(" Step 1: Select Waste Category:\n");
    for (int i = 0; i < NUM_WASTE_CATEGORIES; i++) {
        printf("   [%d] %s\n", i + 1, WASTE_CATEGORIES[i]);
    }
    int catChoice = getInt(" Select Category [1-7]: ", 1, NUM_WASTE_CATEGORIES);
    strncpy(req.waste_type, WASTE_CATEGORIES[catChoice - 1], sizeof(req.waste_type) - 1);

    /* 2. Estimated Weight */
    printf("\n Step 2: Enter Estimated Quantity / Weight (kg):\n");
    req.estimated_weight = getFloat(" Estimated Weight (0.1 - 500.0 kg): ", 0.1f, 500.0f);
    req.actual_weight = 0.0f;

    /* 3. Location & Bin Details */
    printf("\n Step 3: Location & Bin Details:\n");
    SmartBin bins[MAX_BINS];
    int binCount = loadBins(bins, MAX_BINS);
    printf(" Nearby Public Smart Bins available:\n");
    for (int i = 0; i < binCount && i < 5; i++) {
        printf("   * %s: %s (%s)\n", bins[i].bin_id, bins[i].location, bins[i].category);
    }
    printf("   * Or enter 'DOORSTEP' for pickup directly at your house/flat\n");

    getString(" Nearby Smart Bin ID or 'DOORSTEP': ", req.bin_id, sizeof(req.bin_id), false);
    getString(" Detailed Pickup Address / Landmark: ", req.location, sizeof(req.location), false);

    /* 4. Preferred Collection Schedule */
    printf("\n Step 4: Preferred Schedule:\n");
    getString(" Preferred Date (YYYY-MM-DD or DD/MM/YYYY): ", req.preferred_date, sizeof(req.preferred_date), false);

    printf(" Preferred Collection Time Window:\n");
    for (int i = 0; i < NUM_TIME_SLOTS; i++) {
        printf("   [%d] %s\n", i + 1, TIME_SLOTS[i]);
    }
    int slotChoice = getInt(" Select Time Slot [1-3]: ", 1, NUM_TIME_SLOTS);
    strncpy(req.preferred_slot, TIME_SLOTS[slotChoice - 1], sizeof(req.preferred_slot) - 1);

    getString(" Any Special Instructions/Notes (Optional, press ENTER to skip): ", req.special_notes, sizeof(req.special_notes), true);
    if (strlen(req.special_notes) == 0) {
        strcpy(req.special_notes, "None");
    }

    /* Initial state */
    req.status = REQ_STATUS_PENDING;
    strcpy(req.assigned_collector, "Unassigned");
    strcpy(req.assigned_truck, "Unassigned");
    strcpy(req.final_outcome, "Awaiting Admin Review & Collector Assignment");
    req.points_awarded = 0;
    getCurrentTimestamp(req.created_at, sizeof(req.created_at));
    strcpy(req.completed_at, "Pending");

    /* Confirmation */
    clearScreen();
    printHeader("REQUEST CONFIRMATION");
    printf(" Please review your waste collection request before submission:\n\n");
    printf("   Generated Request ID : %s\n", req.id);
    printf("   Citizen Name         : %s (ID: %s)\n", user->name, user->id);
    printf("   Waste Category       : %s\n", req.waste_type);
    printf("   Estimated Weight     : %.2f kg\n", req.estimated_weight);
    printf("   Pickup Location      : %s\n", req.location);
    printf("   Smart Bin Reference  : %s\n", req.bin_id);
    printf("   Preferred Date       : %s\n", req.preferred_date);
    printf("   Preferred Time Window: %s\n", req.preferred_slot);
    printf("   Special Notes        : %s\n", req.special_notes);
    printf("   Initial Status       : %s\n", getStatusName(req.status));
    printDivider('-', 70);

    char confirm[16];
    getString(" Confirm and submit this request? (Y/N): ", confirm, sizeof(confirm), false);
    if (confirm[0] == 'Y' || confirm[0] == 'y') {
        if (addRequest(&req)) {
            printf("\n[SUCCESS] Your request has been successfully submitted!\n");
            printf("Unique Request ID: %s (Save this to track your request)\n", req.id);
            printf("Stage: [1. Pending] -> Waiting for admin approval & collector assignment.\n");
        } else {
            printf("\n[ERROR] System was unable to save the request.\n");
        }
    } else {
        printf("\n[INFO] Request submission was cancelled.\n");
    }

    pausePrompt();
}

/* ==================== TRACK REQUESTS & STATUS ==================== */

void trackUserRequestsUI(const User* user) {
    clearScreen();
    printHeader("TRACK CURRENT REQUESTS & STATUS");

    static WasteRequest reqs[MAX_REQUESTS];
    int count = loadRequests(reqs, MAX_REQUESTS);

    int userReqCount = 0;
    for (int i = 0; i < count; i++) {
        if (strcmp(reqs[i].user_id, user->id) == 0) {
            userReqCount++;
        }
    }

    if (userReqCount == 0) {
        printf("\n You have not submitted any waste collection requests yet.\n");
        printf(" Submit a new request using Option [1] on your Dashboard.\n");
        pausePrompt();
        return;
    }

    printf(" Listing all requests submitted by %s:\n\n", user->name);
    printf("%-10s | %-24s | %-8s | %-12s | %-16s\n",
           "Req ID", "Waste Type", "Est. Wt", "Date", "Current Status");
    printDivider('-', 75);

    for (int i = 0; i < count; i++) {
        if (strcmp(reqs[i].user_id, user->id) == 0) {
            printf("%-10s | %-24s | %-5.1f kg | %-12s | %-16s\n",
                   reqs[i].id,
                   reqs[i].waste_type,
                   reqs[i].estimated_weight,
                   reqs[i].preferred_date,
                   getStatusColorTag(reqs[i].status));
            printProgressBar(reqs[i].status);
            printf("\n");
        }
    }

    printDivider('=', 75);
    printf(" 6 Stages Lifecycle Overview:\n");
    printf("  1. Pending: Request submitted and awaiting Admin validation.\n");
    printf("  2. Approved: Validated by Admin and scheduled for pickup.\n");
    printf("  3. Assigned: Assigned to designated Waste Collector and Truck.\n");
    printf("  4. In Progress: Collector is en-route or actively collecting.\n");
    printf("  5. Collected: Waste collected & actual physical weight verified.\n");
    printf("  6. Final Outcome Recorded: Processed (Recycled / Composted / Safely Disposed).\n");

    pausePrompt();
}

/* ==================== VIEW REQUEST DETAILS ==================== */

void viewRequestDetailsUI(const User* user) {
    clearScreen();
    printHeader("VIEW COMPLETE REQUEST DETAILS");

    char reqId[32];
    getString(" Enter Request ID to inspect (e.g., REQ1001): ", reqId, sizeof(reqId), false);

    static WasteRequest reqs[MAX_REQUESTS];
    int count = loadRequests(reqs, MAX_REQUESTS);
    WasteRequest* req = findRequestById(reqs, count, reqId);

    if (!req) {
        printf("\n[!] No waste request found with ID '%s'.\n", reqId);
        pausePrompt();
        return;
    }

    if (strcmp(req->user_id, user->id) != 0) {
        printf("\n[!] Access Denied: This request does not belong to your account.\n");
        pausePrompt();
        return;
    }

    printf("\n");
    printDivider('=', 70);
    printf(" DETAILED REQUEST RECORD: %s\n", req->id);
    printDivider('=', 70);
    printf("  Request ID          : %s\n", req->id);
    printf("  Submitted On        : %s\n", req->created_at);
    printf("  Citizen ID & Name   : %s (%s)\n", user->id, user->name);
    printf("  Waste Category      : %s\n", req->waste_type);
    printf("  Estimated Weight    : %.2f kg\n", req->estimated_weight);
    if (req->status >= REQ_STATUS_COLLECTED) {
        printf("  Actual Weight (Kg)  : %.2f kg (Verified by Collector)\n", req->actual_weight);
        printf("  Reward Points Earned: %d pts\n", req->points_awarded);
    } else {
        printf("  Actual Weight (Kg)  : Pending verification upon pickup\n");
    }
    printf("  Pickup Location     : %s\n", req->location);
    printf("  Smart Bin ID        : %s\n", req->bin_id);
    printf("  Preferred Schedule  : %s during %s\n", req->preferred_date, req->preferred_slot);
    printf("  Special Notes       : %s\n", req->special_notes);
    printf("  Current Status      : %s %s\n", getStatusName(req->status), getStatusColorTag(req->status));
    printf("  Assigned Collector  : %s\n", req->assigned_collector);
    printf("  Assigned Truck      : %s\n", req->assigned_truck);
    printf("  Final Outcome Trail : %s\n", req->final_outcome);
    printf("  Completion Timestamp: %s\n", req->completed_at);
    printDivider('-', 70);

    printProgressBar(req->status);
    pausePrompt();
}

/* ==================== REPORT BIN PROBLEM ==================== */

void reportBinProblemUI(const User* user) {
    clearScreen();
    printHeader("REPORT SMART BIN PROBLEM");
    printf(" Report a full, overflowing, damaged, or hazardous public bin.\n\n");

    SmartBin bins[MAX_BINS];
    int binCount = loadBins(bins, MAX_BINS);
    printf(" Reference of Public Smart Bins:\n");
    for (int i = 0; i < binCount; i++) {
        printf("   [%s] Location: %-32s | Type: %-22s | Status: %s\n",
               bins[i].bin_id, bins[i].location, bins[i].category, bins[i].status);
    }
    printDivider('-', 70);

    BinProblemReport rep;
    memset(&rep, 0, sizeof(rep));

    generateNextReportId(rep.id);
    strncpy(rep.user_id, user->id, sizeof(rep.user_id) - 1);

    getString(" Enter Bin ID (or 'CUSTOM' if unknown): ", rep.bin_id, sizeof(rep.bin_id), false);
    getString(" Specific Location / Landmark of the Bin: ", rep.location, sizeof(rep.location), false);

    printf("\n Select Problem Encountered:\n");
    for (int i = 0; i < NUM_PROBLEM_TYPES; i++) {
        printf("   [%d] %s\n", i + 1, PROBLEM_TYPES[i]);
    }
    int pChoice = getInt(" Select Problem Type [1-6]: ", 1, NUM_PROBLEM_TYPES);
    strncpy(rep.problem_type, PROBLEM_TYPES[pChoice - 1], sizeof(rep.problem_type) - 1);

    getString(" Detailed Description of Problem: ", rep.description, sizeof(rep.description), false);
    strcpy(rep.status, "Pending Review");
    getCurrentTimestamp(rep.created_at, sizeof(rep.created_at));

    printf("\n");
    printDivider('-', 60);
    printf(" Report Summary:\n");
    printf("   Report ID   : %s\n", rep.id);
    printf("   Bin ID      : %s\n", rep.bin_id);
    printf("   Location    : %s\n", rep.location);
    printf("   Problem Type: %s\n", rep.problem_type);
    printf("   Description : %s\n", rep.description);
    printDivider('-', 60);

    char confirm[16];
    getString(" Submit this problem report to Municipal Admin? (Y/N): ", confirm, sizeof(confirm), false);
    if (confirm[0] == 'Y' || confirm[0] == 'y') {
        if (addBinReport(&rep)) {
            printf("\n[SUCCESS] Problem report %s submitted successfully!\n", rep.id);
            printf("Municipal maintenance team and collectors have been notified.\n");
        } else {
            printf("\n[ERROR] Failed to save report.\n");
        }
    } else {
        printf("\n[INFO] Problem report was cancelled.\n");
    }

    pausePrompt();
}

/* ==================== VIEW COLLECTION HISTORY ==================== */

void viewCollectionHistoryUI(const User* user) {
    clearScreen();
    printHeader("COLLECTION HISTORY");
    printf(" Viewing completed collections for citizen: %s\n\n", user->name);

    static WasteRequest reqs[MAX_REQUESTS];
    int count = loadRequests(reqs, MAX_REQUESTS);

    float totalWeight = 0.0f;
    int totalPoints = 0;
    int completedCount = 0;

    printf("%-10s | %-16s | %-24s | %-10s | %-10s\n",
           "Req ID", "Completed Date", "Waste Type", "Actual Wt", "Points");
    printDivider('-', 75);

    for (int i = 0; i < count; i++) {
        if (strcmp(reqs[i].user_id, user->id) == 0 &&
            (reqs[i].status == REQ_STATUS_COLLECTED || reqs[i].status == REQ_STATUS_FINAL_OUTCOME)) {
            printf("%-10s | %-16s | %-24s | %-6.2f kg  | +%-6d pts\n",
                   reqs[i].id,
                   reqs[i].completed_at,
                   reqs[i].waste_type,
                   reqs[i].actual_weight,
                   reqs[i].points_awarded);
            totalWeight += reqs[i].actual_weight;
            totalPoints += reqs[i].points_awarded;
            completedCount++;
        }
    }

    if (completedCount == 0) {
        printf(" No completed collections on record yet.\n");
    } else {
        printDivider('-', 75);
        printf(" TOTALS: %d Collections Completed | %.2f kg Waste Diverted | %d Total Points Earned\n",
               completedCount, totalWeight, totalPoints);
    }

    pausePrompt();
}

/* ==================== VIEW REWARD POINTS & HISTORY ==================== */

void viewRewardSummaryUI(const User* user) {
    clearScreen();
    printHeader("REWARD POINTS & HISTORY");

    printf(" Citizen Account: %s (%s)\n", user->name, user->id);
    printf(" Current Available Balance: [%d Reward Points]\n\n", user->reward_points);
    printDivider('-', 70);

    printf(" Verified Reward Points Ledger:\n\n");
    printf("%-10s | %-10s | %-16s | %-24s | %-8s | %-8s\n",
           "Reward ID", "Req ID", "Date", "Waste Category", "Weight", "Earned");
    printDivider('-', 80);

    RewardEntry rewards[MAX_REWARDS];
    int count = loadRewards(rewards, MAX_REWARDS);
    int entriesFound = 0;

    for (int i = 0; i < count; i++) {
        if (strcmp(rewards[i].user_id, user->id) == 0) {
            printf("%-10s | %-10s | %-16s | %-24s | %-5.2f kg | +%d pts\n",
                   rewards[i].id,
                   rewards[i].request_id,
                   rewards[i].date,
                   rewards[i].waste_type,
                   rewards[i].weight,
                   rewards[i].points);
            entriesFound++;
        }
    }

    if (entriesFound == 0) {
        printf(" No reward transactions recorded yet.\n");
        printf(" Points are awarded automatically when collectors verify your waste weight!\n");
    }

    printDivider('=', 80);
    printf(" Point Calculation Rule:\n");
    printf("  * Plastic & Metal Recyclables : 10 points per kg\n");
    printf("  * Organic & Compostable       : 5 points per kg\n");
    printf("  * Paper & Cardboard           : 8 points per kg\n");
    printf("  * Electronic Waste            : 15 points per kg\n");
    printf("  * Chemical / Hazardous        : 20 points per kg\n");
    printf("\n [ECO-STORE] You currently have %d Reward Points available!\n", user->reward_points);
    printf(" Use Option [7] in the main menu to redeem points for water bottles, small dustbins,\n");
    printf(" liquid handwash, tissue boxes, compostable bags, and more!\n");

    pausePrompt();
}

/* ==================== VIEW FINAL WASTE OUTCOMES ==================== */

void viewWasteOutcomeTransparencyUI(const User* user) {
    clearScreen();
    printHeader("FINAL WASTE OUTCOME & TRANSPARENCY TRAIL");
    printf(" Track what happened to your collected waste after handover.\n\n");

    static WasteRequest reqs[MAX_REQUESTS];
    int count = loadRequests(reqs, MAX_REQUESTS);

    int found = 0;
    for (int i = 0; i < count; i++) {
        if (strcmp(reqs[i].user_id, user->id) == 0 && reqs[i].status == REQ_STATUS_FINAL_OUTCOME) {
            printf(" [Record %s] - Completed: %s\n", reqs[i].id, reqs[i].completed_at);
            printf("   * Waste Type       : %s\n", reqs[i].waste_type);
            printf("   * Diverted Weight  : %.2f kg\n", reqs[i].actual_weight);
            printf("   * Collection Agent : %s via Truck %s\n", reqs[i].assigned_collector, reqs[i].assigned_truck);
            printf("   * Final Destination: %s\n", reqs[i].final_outcome);
            printf("   * Citizen Reward   : +%d points awarded\n", reqs[i].points_awarded);
            printDivider('-', 70);
            found++;
        }
    }

    if (found == 0) {
        printf(" No final processed outcomes recorded yet for your submitted requests.\n");
        printf(" Once our recycling facilities process your collected waste, full transparency records will appear here.\n");
    }

    pausePrompt();
}

/* ==================== VIEW CITIZEN PROFILE ==================== */

void viewUserProfileUI(const User* user) {
    clearScreen();
    printHeader("CITIZEN PROFILE DETAILS");
    printf("  Account ID       : %s\n", user->id);
    printf("  Full Name        : %s\n", user->name);
    printf("  Registered Email : %s\n", user->email);
    printf("  Contact Phone    : %s\n", user->phone);
    printf("  Reward Points    : %d pts\n", user->reward_points);
    printf("  Member Since     : %s\n", user->created_at);
    printDivider('-', 70);
    pausePrompt();
}

/* ==================== ECO-STORE REDEMPTION ==================== */

void redeemPointsUI(User* user) {
    while (1) {
        /* Refresh user data */
        User currentUsers[MAX_USERS];
        int uCount = loadUsers(currentUsers, MAX_USERS);
        User* freshUser = findUserById(currentUsers, uCount, user->id);
        if (freshUser) {
            *user = *freshUser;
        }

        clearScreen();
        printHeader("ECO-REWARDS REDEMPTION STORE");
        printf(" Welcome, %s!\n", user->name);
        printf(" Your Current Balance: [%d Reward Points]\n\n", user->reward_points);
        printf(" Use your earned recycling points to purchase high-quality eco-friendly items!\n");
        printDivider('=', 85);

        static StoreItem items[MAX_STORE_ITEMS];
        int itemCount = loadStoreItems(items, MAX_STORE_ITEMS);

        if (itemCount == 0) {
            printf(" Store catalog is currently empty. Please check back later!\n");
            pausePrompt();
            return;
        }

        printf("%-8s | %-28s | %-16s | %-12s | %-10s\n",
               "Item ID", "Product Name", "Category", "Points Cost", "In Stock");
        printDivider('-', 85);

        for (int i = 0; i < itemCount; i++) {
            char stockText[32];
            if (items[i].stock_qty > 0) {
                sprintf(stockText, "%d pcs", items[i].stock_qty);
            } else {
                strcpy(stockText, "OUT OF STOCK");
            }
            printf("%-8s | %-28.28s | %-16.16s | %3d pts      | %-10s\n",
                   items[i].id,
                   items[i].name,
                   items[i].category,
                   items[i].points_cost,
                   stockText);
        }
        printDivider('=', 85);

        char inputId[32];
        getString("\n Enter Item ID to purchase (e.g. ITEM101) or '0' to return: ", inputId, sizeof(inputId), true);
        trimString(inputId);

        if (strcmp(inputId, "0") == 0 || strlen(inputId) == 0) {
            return;
        }

        /* Case-insensitive matching for convenience */
        StoreItem* selected = NULL;
        for (int i = 0; i < itemCount; i++) {
            if (strcasecmp(items[i].id, inputId) == 0) {
                selected = &items[i];
                break;
            }
        }

        if (!selected) {
            printf("\n[!] Error: Item with ID '%s' not found in catalog!\n", inputId);
            pausePrompt();
            continue;
        }

        if (selected->stock_qty <= 0) {
            printf("\n[!] Sorry, '%s' is currently OUT OF STOCK!\n", selected->name);
            printf(" Please select another item or check back later.\n");
            pausePrompt();
            continue;
        }

        printf("\n Selected Product Details:\n");
        printf("   * Name       : %s\n", selected->name);
        printf("   * Category   : %s\n", selected->category);
        printf("   * Description: %s\n", selected->description);
        printf("   * Price      : %d points per unit\n", selected->points_cost);
        printf("   * Available  : %d unit(s) in stock\n", selected->stock_qty);
        printDivider('-', 65);

        int maxAffordable = user->reward_points / selected->points_cost;
        if (maxAffordable < 1) {
            printf("\n[!] Insufficient Points! You have %d pts, but 1 unit costs %d pts.\n",
                   user->reward_points, selected->points_cost);
            printf(" Need %d more points. Keep recycling to earn more rewards!\n",
                   selected->points_cost - user->reward_points);
            pausePrompt();
            continue;
        }

        int maxQty = selected->stock_qty < maxAffordable ? selected->stock_qty : maxAffordable;
        printf(" Based on your points (%d pts), you can purchase up to %d unit(s).\n",
               user->reward_points, maxQty);

        int qty = getInt(" Enter quantity to redeem [1 to max, or 0 to cancel]: ", 0, selected->stock_qty);
        if (qty <= 0) {
            printf(" Purchase cancelled.\n");
            pausePrompt();
            continue;
        }

        int totalPoints = selected->points_cost * qty;
        if (totalPoints > user->reward_points) {
            printf("\n[!] Error: Insufficient points! %d unit(s) require %d pts, but you have %d pts.\n",
                   qty, totalPoints, user->reward_points);
            pausePrompt();
            continue;
        }

        char address[128];
        getString(" Enter delivery address / area (e.g., House 12, Road 4): ", address, sizeof(address), true);
        trimString(address);
        if (strlen(address) == 0) {
            strcpy(address, "Customer Pickup / Default Area");
        }

        clearScreen();
        printHeader("CONFIRM ECO-STORE PURCHASE");
        printf(" Please review your order details before confirming:\n\n");
        printf("   Item to Purchase  : %s (%s)\n", selected->name, selected->id);
        printf("   Category          : %s\n", selected->category);
        printf("   Quantity          : %d unit(s)\n", qty);
        printf("   Points per Unit   : %d pts\n", selected->points_cost);
        printf("   Total Cost        : %d pts\n", totalPoints);
        printf("   Current Balance   : %d pts\n", user->reward_points);
        printf("   Remaining Balance : %d pts\n", user->reward_points - totalPoints);
        printf("   Delivery Address  : %s\n", address);
        printf("   Recipient Name    : %s\n", user->name);
        printf("   Contact Phone     : %s\n", user->phone);
        printDivider('=', 70);

        char confirm[16];
        getString(" Confirm this redemption using your reward points? (y/n): ", confirm, sizeof(confirm), false);
        if (confirm[0] != 'y' && confirm[0] != 'Y') {
            printf("\n Purchase cancelled. No points were deducted.\n");
            pausePrompt();
            continue;
        }

        /* 1. Deduct points from user */
        user->reward_points -= totalPoints;
        updateUser(user);

        /* 2. Deduct item inventory stock */
        selected->stock_qty -= qty;
        updateStoreItem(selected);

        /* 3. Record purchase in purchases ledger */
        ItemPurchase purchase;
        memset(&purchase, 0, sizeof(purchase));
        generateNextPurchaseId(purchase.purchase_id);
        strncpy(purchase.user_id, user->id, sizeof(purchase.user_id) - 1);
        strncpy(purchase.user_name, user->name, sizeof(purchase.user_name) - 1);
        strncpy(purchase.item_id, selected->id, sizeof(purchase.item_id) - 1);
        strncpy(purchase.item_name, selected->name, sizeof(purchase.item_name) - 1);
        purchase.quantity = qty;
        purchase.points_spent = totalPoints;
        strncpy(purchase.delivery_address, address, sizeof(purchase.delivery_address) - 1);
        strcpy(purchase.status, "Confirmed - In Dispatch");
        getCurrentTimestamp(purchase.purchase_date, sizeof(purchase.purchase_date));

        addPurchase(&purchase);

        /* 4. Display Receipt */
        clearScreen();
        printHeader("PURCHASE & REDEMPTION RECEIPT");
        printf(" [SUCCESS] Congratulations! Your eco-reward redemption was successful!\n\n");
        printf("   Order ID         : %s\n", purchase.purchase_id);
        printf("   Product          : %s (x%d)\n", purchase.item_name, purchase.quantity);
        printf("   Points Deducted  : -%d pts\n", purchase.points_spent);
        printf("   New Balance      : %d pts\n", user->reward_points);
        printf("   Delivery Location: %s\n", purchase.delivery_address);
        printf("   Order Date       : %s\n", purchase.purchase_date);
        printf("   Status           : %s\n", purchase.status);
        printDivider('-', 70);
        printf(" Thank you for recycling with us and keeping our city clean and green!\n");
        pausePrompt();
    }
}

/* ==================== VIEW PURCHASE HISTORY ==================== */

void viewPurchaseHistoryUI(const User* user) {
    clearScreen();
    printHeader("MY ECO-STORE REDEMPTION & PURCHASE HISTORY");
    printf(" Citizen: %s (ID: %s) | Available Points: %d pts\n\n",
           user->name, user->id, user->reward_points);

    static ItemPurchase purchases[MAX_PURCHASES];
    int count = loadPurchases(purchases, MAX_PURCHASES);
    int userPurchases = 0;
    int totalItems = 0;
    int totalPointsSpent = 0;

    printf("%-10s | %-16s | %-26s | %-4s | %-10s | %-16s\n",
           "Order ID", "Date", "Item Purchased", "Qty", "Points", "Status");
    printDivider('-', 90);

    for (int i = 0; i < count; i++) {
        if (strcmp(purchases[i].user_id, user->id) == 0) {
            printf("%-10s | %-16.16s | %-26.26s | %3d  | %4d pts   | %-16.16s\n",
                   purchases[i].purchase_id,
                   purchases[i].purchase_date,
                   purchases[i].item_name,
                   purchases[i].quantity,
                   purchases[i].points_spent,
                   purchases[i].status);
            userPurchases++;
            totalItems += purchases[i].quantity;
            totalPointsSpent += purchases[i].points_spent;
        }
    }

    if (userPurchases == 0) {
        printf(" You have not redeemed any items from the Eco-Store yet.\n");
        printf(" Earn reward points by submitting waste for collection and spend them in the store!\n");
    } else {
        printDivider('-', 90);
        printf(" TOTALS: %d Orders | %d Items Received | %d Total Points Spent\n",
               userPurchases, totalItems, totalPointsSpent);
    }

    pausePrompt();
}
