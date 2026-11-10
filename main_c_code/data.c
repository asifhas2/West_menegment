#include "data.h"
#include "utils.h"

/* Helper to safely split delimiter-separated lines while preserving empty fields */
static int splitLine(char* line, char* fields[], int max_fields, char delimiter) {
    int count = 0;
    char* start = line;
    char* current = line;

    while (*current && count < max_fields) {
        if (*current == delimiter) {
            *current = '\0';
            fields[count++] = start;
            start = current + 1;
        }
        current++;
    }
    if (count < max_fields) {
        fields[count++] = start;
    }
    return count;
}

static bool fileExists(const char* filename) {
    FILE* fp = fopen(filename, "r");
    if (fp) {
        fclose(fp);
        return true;
    }
    return false;
}

/* Ensure all project data files exist and populate sensible mock seeds on first run */
bool initDataFiles(void) {
    /* 1. Smart Bins */
    if (!fileExists(FILE_BINS)) {
        FILE* fp = fopen(FILE_BINS, "w");
        if (fp) {
            fprintf(fp, "BIN101|Block A - Green Park Zone|Organic / Biodegradable|Normal\n");
            fprintf(fp, "BIN102|Block B - Commercial Market|Recyclable - Plastic|Normal\n");
            fprintf(fp, "BIN103|Block C - School & Library Area|Recyclable - Paper & Metal|Full\n");
            fprintf(fp, "BIN104|Block D - Healthcare Complex|Hazardous / Chemical Waste|Normal\n");
            fprintf(fp, "BIN105|Block E - Residential Avenue 4|Electronic Waste (E-Waste)|Normal\n");
            fclose(fp);
        }
    }

    /* 2. Registered Citizens */
    if (!fileExists(FILE_USERS)) {
        FILE* fp = fopen(FILE_USERS, "w");
        if (fp) {
            fprintf(fp, "USR1001|Rahim Ahmed|user@test.com|01711223344|password123|115|2026-10-01 10:00\n");
            fprintf(fp, "USR1002|Fatima Begum|fatima@test.com|01799887766|password123|60|2026-10-15 14:20\n");
            fclose(fp);
        }
    }

    /* 3. Waste Collection Requests across different stages */
    if (!fileExists(FILE_REQUESTS)) {
        FILE* fp = fopen(FILE_REQUESTS, "w");
        if (fp) {
            /* Completed with final outcome recorded */
            fprintf(fp, "REQ1001|USR1001|Recyclable - Plastic|5.00|5.50|House 12, Road 4, Sector 7|BIN102|2026-10-05|Morning (8:00 AM - 12:00 PM)|Bottles and food containers clean|5|COL101 (Karim)|TRK-01|Recycled into polymer pellets at EcoPlast Plant|55|2026-10-04 18:20|2026-10-05 14:30\n");
            fprintf(fp, "REQ1002|USR1001|Organic / Biodegradable|10.00|12.00|House 12, Road 4, Sector 7|BIN101|2026-10-12|Afternoon (12:00 PM - 4:00 PM)|Kitchen and garden organic waste|5|COL102 (Hasan)|TRK-03|Composted into bio-fertilizer at GreenBio Facility|60|2026-10-11 09:15|2026-10-12 16:10\n");

            /* Active request en route */
            fprintf(fp, "REQ1003|USR1001|Electronic Waste (E-Waste)|3.00|0.00|House 12, Road 4, Sector 7|BIN105|2026-11-08|Morning (8:00 AM - 12:00 PM)|Old circuit boards and cables|3|COL101 (Karim)|TRK-02|Pending Collection|0|2026-11-06 14:00|In Progress\n");

            /* Pending requests awaiting admin approval */
            fprintf(fp, "REQ1004|USR1001|Hazardous / Chemical Waste|2.00|0.00|House 12, Road 4, Sector 7|BIN104|2026-11-09|Evening (4:00 PM - 7:00 PM)|Expired household solvents|0|Unassigned|Unassigned|Awaiting Admin Review|0|2026-11-07 10:30|Pending\n");
            fprintf(fp, "REQ1005|USR1002|Recyclable - Paper & Cardboard|15.00|0.00|Flat 4B, Green Villa, Dhanmondi|BIN103|2026-11-12|Morning (8:00 AM - 12:00 PM)|Bundles of old newspapers|1|Unassigned|Unassigned|Approved - Scheduled for Assignment|0|2026-11-08 11:00|Pending\n");
            fclose(fp);
        }
    }

    /* 4. Rewards ledger */
    if (!fileExists(FILE_REWARDS)) {
        FILE* fp = fopen(FILE_REWARDS, "w");
        if (fp) {
            fprintf(fp, "REW1001|USR1001|REQ1001|Recyclable - Plastic|5.50|55|2026-10-05 14:30\n");
            fprintf(fp, "REW1002|USR1001|REQ1002|Organic / Biodegradable|12.00|60|2026-10-12 16:10\n");
            fclose(fp);
        }
    }

    /* 5. Citizen bin problem reports */
    if (!fileExists(FILE_BIN_REPORTS)) {
        FILE* fp = fopen(FILE_BIN_REPORTS, "w");
        if (fp) {
            fprintf(fp, "REP1001|USR1001|BIN103|Block C - School & Library Area|Bin Overflowing / Excess Garbage|Lid cannot close and cardboard is spilling on footpath|Under Review|2026-10-25 11:20\n");
            fclose(fp);
        }
    }

    /* 6. Field Waste Collectors */
    if (!fileExists(FILE_COLLECTORS)) {
        FILE* fp = fopen(FILE_COLLECTORS, "w");
        if (fp) {
            fprintf(fp, "COL101|Karim Ullah|karim@collector.com|01811223344|collector123|TRK-01|Available|85.50|12\n");
            fprintf(fp, "COL102|Hasan Mahmud|hasan@collector.com|01822334455|collector123|TRK-03|Available|64.00|8\n");
            fprintf(fp, "COL103|Tariq Islam|tariq@collector.com|01833445566|collector123|TRK-02|Available|42.00|5\n");
            fclose(fp);
        }
    }

    /* 7. Collection Trucks / Fleet */
    if (!fileExists(FILE_TRUCKS)) {
        FILE* fp = fopen(FILE_TRUCKS, "w");
        if (fp) {
            fprintf(fp, "TRK-01|DHK-METRO-1122|Rahman Mia|1500.00|250.00|Available\n");
            fprintf(fp, "TRK-02|DHK-METRO-3344|Solaiman Ali|2000.00|320.00|Available\n");
            fprintf(fp, "TRK-03|DHK-METRO-5566|Jamal Hossain|1800.00|0.00|Available\n");
            fprintf(fp, "TRK-04|DHK-METRO-7788|Anwar Hossain|2500.00|0.00|Maintenance\n");
            fclose(fp);
        }
    }

    /* 8. Collector Problem Reports */
    if (!fileExists(FILE_COLLECTOR_REPORTS)) {
        FILE* fp = fopen(FILE_COLLECTOR_REPORTS, "w");
        if (fp) {
            fprintf(fp, "CREP1001|COL101|REQ1003|Location Inaccessible|Road digging in Sector 7 delayed vehicle entry|Under Review|2026-11-06 15:30\n");
            fclose(fp);
        }
    }

    /* 9. Commercial Recyclable Sales */
    if (!fileExists(FILE_SALES)) {
        FILE* fp = fopen(FILE_SALES, "w");
        if (fp) {
            fprintf(fp, "SALE1001|Recyclable - Plastic|500.00|EcoPlast Industries Ltd.|35.00|17500.00|2026-10-30 14:00\n");
            fprintf(fp, "SALE1002|Recyclable - Paper & Cardboard|800.00|GreenPaper Mills|18.00|14400.00|2026-11-02 11:30\n");
            fprintf(fp, "SALE1003|Recyclable - Metal / Cans|300.00|Bengal Metal Recyclers|75.00|22500.00|2026-11-05 16:45\n");
            fclose(fp);
        }
    }

    /* 10. Municipal Administrators */
    if (!fileExists(FILE_ADMINS)) {
        FILE* fp = fopen(FILE_ADMINS, "w");
        if (fp) {
            fprintf(fp, "ADM101|admin|System Administrator|01700000000|admin123|Super Admin|2026-10-01 10:00\n");
            fprintf(fp, "ADM102|manager|Operations Director|01711002200|admin123|Operations Director|2026-10-05 09:30\n");
            fclose(fp);
        }
    }

    return true;
}

/* ==================== USER DATA OPERATIONS ==================== */

int loadUsers(User users[], int max_count) {
    FILE* fp = fopen(FILE_USERS, "r");
    if (!fp) return 0;

    char line[512];
    int count = 0;
    while (fgets(line, sizeof(line), fp) && count < max_count) {
        trimString(line);
        if (strlen(line) == 0) continue;

        char* fields[10];
        int num_fields = splitLine(line, fields, 10, '|');
        if (num_fields >= 7) {
            strncpy(users[count].id, fields[0], sizeof(users[count].id) - 1);
            strncpy(users[count].name, fields[1], sizeof(users[count].name) - 1);
            strncpy(users[count].email, fields[2], sizeof(users[count].email) - 1);
            strncpy(users[count].phone, fields[3], sizeof(users[count].phone) - 1);
            strncpy(users[count].password, fields[4], sizeof(users[count].password) - 1);
            users[count].reward_points = atoi(fields[5]);
            strncpy(users[count].created_at, fields[6], sizeof(users[count].created_at) - 1);
            count++;
        }
    }
    fclose(fp);
    return count;
}

bool saveUsers(const User users[], int count) {
    FILE* fp = fopen(FILE_USERS, "w");
    if (!fp) return false;

    for (int i = 0; i < count; i++) {
        fprintf(fp, "%s|%s|%s|%s|%s|%d|%s\n",
                users[i].id,
                users[i].name,
                users[i].email,
                users[i].phone,
                users[i].password,
                users[i].reward_points,
                users[i].created_at);
    }
    fclose(fp);
    return true;
}

bool addUser(const User* user) {
    FILE* fp = fopen(FILE_USERS, "a");
    if (!fp) return false;

    fprintf(fp, "%s|%s|%s|%s|%s|%d|%s\n",
            user->id,
            user->name,
            user->email,
            user->phone,
            user->password,
            user->reward_points,
            user->created_at);
    fclose(fp);
    return true;
}

bool updateUser(const User* user) {
    User users[MAX_USERS];
    int count = loadUsers(users, MAX_USERS);
    bool found = false;
    for (int i = 0; i < count; i++) {
        if (strcmp(users[i].id, user->id) == 0) {
            users[i] = *user;
            found = true;
            break;
        }
    }
    if (found) {
        return saveUsers(users, count);
    }
    return false;
}

bool deleteUser(const char* user_id) {
    User users[MAX_USERS];
    int count = loadUsers(users, MAX_USERS);
    int new_count = 0;
    User updated[MAX_USERS];

    for (int i = 0; i < count; i++) {
        if (strcmp(users[i].id, user_id) != 0) {
            updated[new_count++] = users[i];
        }
    }
    if (new_count < count) {
        return saveUsers(updated, new_count);
    }
    return false;
}

User* findUserByEmailOrPhone(User users[], int count, const char* identifier) {
    for (int i = 0; i < count; i++) {
        if (strcmp(users[i].email, identifier) == 0 || strcmp(users[i].phone, identifier) == 0) {
            return &users[i];
        }
    }
    return NULL;
}

User* findUserById(User users[], int count, const char* id) {
    for (int i = 0; i < count; i++) {
        if (strcmp(users[i].id, id) == 0) {
            return &users[i];
        }
    }
    return NULL;
}

void generateNextUserId(char* out_id) {
    User users[MAX_USERS];
    int count = loadUsers(users, MAX_USERS);
    int max_num = 1000;
    for (int i = 0; i < count; i++) {
        if (strncmp(users[i].id, "USR", 3) == 0) {
            int num = atoi(users[i].id + 3);
            if (num > max_num) max_num = num;
        }
    }
    sprintf(out_id, "USR%d", max_num + 1);
}

/* ==================== WASTE REQUEST OPERATIONS ==================== */

int loadRequests(WasteRequest requests[], int max_count) {
    FILE* fp = fopen(FILE_REQUESTS, "r");
    if (!fp) return 0;

    char line[1024];
    int count = 0;
    while (fgets(line, sizeof(line), fp) && count < max_count) {
        trimString(line);
        if (strlen(line) == 0) continue;

        char* f[20];
        int num_fields = splitLine(line, f, 20, '|');
        if (num_fields >= 17) {
            strncpy(requests[count].id, f[0], sizeof(requests[count].id) - 1);
            strncpy(requests[count].user_id, f[1], sizeof(requests[count].user_id) - 1);
            strncpy(requests[count].waste_type, f[2], sizeof(requests[count].waste_type) - 1);
            requests[count].estimated_weight = strtof(f[3], NULL);
            requests[count].actual_weight = strtof(f[4], NULL);
            strncpy(requests[count].location, f[5], sizeof(requests[count].location) - 1);
            strncpy(requests[count].bin_id, f[6], sizeof(requests[count].bin_id) - 1);
            strncpy(requests[count].preferred_date, f[7], sizeof(requests[count].preferred_date) - 1);
            strncpy(requests[count].preferred_slot, f[8], sizeof(requests[count].preferred_slot) - 1);
            strncpy(requests[count].special_notes, f[9], sizeof(requests[count].special_notes) - 1);
            requests[count].status = (RequestStatus)atoi(f[10]);
            strncpy(requests[count].assigned_collector, f[11], sizeof(requests[count].assigned_collector) - 1);
            strncpy(requests[count].assigned_truck, f[12], sizeof(requests[count].assigned_truck) - 1);
            strncpy(requests[count].final_outcome, f[13], sizeof(requests[count].final_outcome) - 1);
            requests[count].points_awarded = atoi(f[14]);
            strncpy(requests[count].created_at, f[15], sizeof(requests[count].created_at) - 1);
            strncpy(requests[count].completed_at, f[16], sizeof(requests[count].completed_at) - 1);
            count++;
        }
    }
    fclose(fp);
    return count;
}

bool saveRequests(const WasteRequest requests[], int count) {
    FILE* fp = fopen(FILE_REQUESTS, "w");
    if (!fp) return false;

    for (int i = 0; i < count; i++) {
        fprintf(fp, "%s|%s|%s|%.2f|%.2f|%s|%s|%s|%s|%s|%d|%s|%s|%s|%d|%s|%s\n",
                requests[i].id,
                requests[i].user_id,
                requests[i].waste_type,
                requests[i].estimated_weight,
                requests[i].actual_weight,
                requests[i].location,
                requests[i].bin_id,
                requests[i].preferred_date,
                requests[i].preferred_slot,
                requests[i].special_notes,
                (int)requests[i].status,
                requests[i].assigned_collector,
                requests[i].assigned_truck,
                requests[i].final_outcome,
                requests[i].points_awarded,
                requests[i].created_at,
                requests[i].completed_at);
    }
    fclose(fp);
    return true;
}

bool addRequest(const WasteRequest* req) {
    FILE* fp = fopen(FILE_REQUESTS, "a");
    if (!fp) return false;

    fprintf(fp, "%s|%s|%s|%.2f|%.2f|%s|%s|%s|%s|%s|%d|%s|%s|%s|%d|%s|%s\n",
            req->id,
            req->user_id,
            req->waste_type,
            req->estimated_weight,
            req->actual_weight,
            req->location,
            req->bin_id,
            req->preferred_date,
            req->preferred_slot,
            req->special_notes,
            (int)req->status,
            req->assigned_collector,
            req->assigned_truck,
            req->final_outcome,
            req->points_awarded,
            req->created_at,
            req->completed_at);
    fclose(fp);
    return true;
}

bool updateRequest(const WasteRequest* req) {
    WasteRequest requests[MAX_REQUESTS];
    int count = loadRequests(requests, MAX_REQUESTS);
    bool found = false;
    for (int i = 0; i < count; i++) {
        if (strcmp(requests[i].id, req->id) == 0) {
            requests[i] = *req;
            found = true;
            break;
        }
    }
    if (found) {
        return saveRequests(requests, count);
    }
    return false;
}

WasteRequest* findRequestById(WasteRequest requests[], int count, const char* id) {
    for (int i = 0; i < count; i++) {
        if (strcmp(requests[i].id, id) == 0) {
            return &requests[i];
        }
    }
    return NULL;
}

void generateNextRequestId(char* out_id) {
    WasteRequest reqs[MAX_REQUESTS];
    int count = loadRequests(reqs, MAX_REQUESTS);
    int max_num = 1000;
    for (int i = 0; i < count; i++) {
        if (strncmp(reqs[i].id, "REQ", 3) == 0) {
            int num = atoi(reqs[i].id + 3);
            if (num > max_num) max_num = num;
        }
    }
    sprintf(out_id, "REQ%d", max_num + 1);
}

/* ==================== BIN PROBLEM REPORT OPERATIONS ==================== */

int loadBinReports(BinProblemReport reports[], int max_count) {
    FILE* fp = fopen(FILE_BIN_REPORTS, "r");
    if (!fp) return 0;

    char line[1024];
    int count = 0;
    while (fgets(line, sizeof(line), fp) && count < max_count) {
        trimString(line);
        if (strlen(line) == 0) continue;

        char* f[10];
        int num_fields = splitLine(line, f, 10, '|');
        if (num_fields >= 8) {
            strncpy(reports[count].id, f[0], sizeof(reports[count].id) - 1);
            strncpy(reports[count].user_id, f[1], sizeof(reports[count].user_id) - 1);
            strncpy(reports[count].bin_id, f[2], sizeof(reports[count].bin_id) - 1);
            strncpy(reports[count].location, f[3], sizeof(reports[count].location) - 1);
            strncpy(reports[count].problem_type, f[4], sizeof(reports[count].problem_type) - 1);
            strncpy(reports[count].description, f[5], sizeof(reports[count].description) - 1);
            strncpy(reports[count].status, f[6], sizeof(reports[count].status) - 1);
            strncpy(reports[count].created_at, f[7], sizeof(reports[count].created_at) - 1);
            count++;
        }
    }
    fclose(fp);
    return count;
}

static bool saveBinReports(const BinProblemReport reports[], int count) {
    FILE* fp = fopen(FILE_BIN_REPORTS, "w");
    if (!fp) return false;

    for (int i = 0; i < count; i++) {
        fprintf(fp, "%s|%s|%s|%s|%s|%s|%s|%s\n",
                reports[i].id,
                reports[i].user_id,
                reports[i].bin_id,
                reports[i].location,
                reports[i].problem_type,
                reports[i].description,
                reports[i].status,
                reports[i].created_at);
    }
    fclose(fp);
    return true;
}

bool addBinReport(const BinProblemReport* report) {
    FILE* fp = fopen(FILE_BIN_REPORTS, "a");
    if (!fp) return false;

    fprintf(fp, "%s|%s|%s|%s|%s|%s|%s|%s\n",
            report->id,
            report->user_id,
            report->bin_id,
            report->location,
            report->problem_type,
            report->description,
            report->status,
            report->created_at);
    fclose(fp);
    return true;
}

bool updateBinReport(const BinProblemReport* report) {
    BinProblemReport reports[MAX_BIN_REPORTS];
    int count = loadBinReports(reports, MAX_BIN_REPORTS);
    bool found = false;
    for (int i = 0; i < count; i++) {
        if (strcmp(reports[i].id, report->id) == 0) {
            reports[i] = *report;
            found = true;
            break;
        }
    }
    if (found) {
        return saveBinReports(reports, count);
    }
    return false;
}

void generateNextReportId(char* out_id) {
    BinProblemReport reports[MAX_BIN_REPORTS];
    int count = loadBinReports(reports, MAX_BIN_REPORTS);
    int max_num = 1000;
    for (int i = 0; i < count; i++) {
        if (strncmp(reports[i].id, "REP", 3) == 0) {
            int num = atoi(reports[i].id + 3);
            if (num > max_num) max_num = num;
        }
    }
    sprintf(out_id, "REP%d", max_num + 1);
}

/* ==================== COLLECTOR PROBLEM REPORT OPERATIONS ==================== */

int loadCollectorReports(CollectorProblemReport reports[], int max_count) {
    FILE* fp = fopen(FILE_COLLECTOR_REPORTS, "r");
    if (!fp) return 0;

    char line[1024];
    int count = 0;
    while (fgets(line, sizeof(line), fp) && count < max_count) {
        trimString(line);
        if (strlen(line) == 0) continue;

        char* f[10];
        int num_fields = splitLine(line, f, 10, '|');
        if (num_fields >= 7) {
            strncpy(reports[count].id, f[0], sizeof(reports[count].id) - 1);
            strncpy(reports[count].collector_id, f[1], sizeof(reports[count].collector_id) - 1);
            strncpy(reports[count].request_id, f[2], sizeof(reports[count].request_id) - 1);
            strncpy(reports[count].problem_type, f[3], sizeof(reports[count].problem_type) - 1);
            strncpy(reports[count].description, f[4], sizeof(reports[count].description) - 1);
            strncpy(reports[count].status, f[5], sizeof(reports[count].status) - 1);
            strncpy(reports[count].created_at, f[6], sizeof(reports[count].created_at) - 1);
            count++;
        }
    }
    fclose(fp);
    return count;
}

static bool saveCollectorReports(const CollectorProblemReport reports[], int count) {
    FILE* fp = fopen(FILE_COLLECTOR_REPORTS, "w");
    if (!fp) return false;

    for (int i = 0; i < count; i++) {
        fprintf(fp, "%s|%s|%s|%s|%s|%s|%s\n",
                reports[i].id,
                reports[i].collector_id,
                reports[i].request_id,
                reports[i].problem_type,
                reports[i].description,
                reports[i].status,
                reports[i].created_at);
    }
    fclose(fp);
    return true;
}

bool addCollectorReport(const CollectorProblemReport* report) {
    FILE* fp = fopen(FILE_COLLECTOR_REPORTS, "a");
    if (!fp) return false;

    fprintf(fp, "%s|%s|%s|%s|%s|%s|%s\n",
            report->id,
            report->collector_id,
            report->request_id,
            report->problem_type,
            report->description,
            report->status,
            report->created_at);
    fclose(fp);
    return true;
}

bool updateCollectorReport(const CollectorProblemReport* report) {
    CollectorProblemReport reports[MAX_COLLECTOR_REPORTS];
    int count = loadCollectorReports(reports, MAX_COLLECTOR_REPORTS);
    bool found = false;
    for (int i = 0; i < count; i++) {
        if (strcmp(reports[i].id, report->id) == 0) {
            reports[i] = *report;
            found = true;
            break;
        }
    }
    if (found) {
        return saveCollectorReports(reports, count);
    }
    return false;
}

void generateNextCollectorReportId(char* out_id) {
    CollectorProblemReport reports[MAX_COLLECTOR_REPORTS];
    int count = loadCollectorReports(reports, MAX_COLLECTOR_REPORTS);
    int max_num = 1000;
    for (int i = 0; i < count; i++) {
        if (strncmp(reports[i].id, "CREP", 4) == 0) {
            int num = atoi(reports[i].id + 4);
            if (num > max_num) max_num = num;
        }
    }
    sprintf(out_id, "CREP%d", max_num + 1);
}

/* ==================== REWARD OPERATIONS ==================== */

int loadRewards(RewardEntry rewards[], int max_count) {
    FILE* fp = fopen(FILE_REWARDS, "r");
    if (!fp) return 0;

    char line[512];
    int count = 0;
    while (fgets(line, sizeof(line), fp) && count < max_count) {
        trimString(line);
        if (strlen(line) == 0) continue;

        char* f[10];
        int num_fields = splitLine(line, f, 10, '|');
        if (num_fields >= 7) {
            strncpy(rewards[count].id, f[0], sizeof(rewards[count].id) - 1);
            strncpy(rewards[count].user_id, f[1], sizeof(rewards[count].user_id) - 1);
            strncpy(rewards[count].request_id, f[2], sizeof(rewards[count].request_id) - 1);
            strncpy(rewards[count].waste_type, f[3], sizeof(rewards[count].waste_type) - 1);
            rewards[count].weight = strtof(f[4], NULL);
            rewards[count].points = atoi(f[5]);
            strncpy(rewards[count].date, f[6], sizeof(rewards[count].date) - 1);
            count++;
        }
    }
    fclose(fp);
    return count;
}

bool addReward(const RewardEntry* reward) {
    FILE* fp = fopen(FILE_REWARDS, "a");
    if (!fp) return false;

    fprintf(fp, "%s|%s|%s|%s|%.2f|%d|%s\n",
            reward->id,
            reward->user_id,
            reward->request_id,
            reward->waste_type,
            reward->weight,
            reward->points,
            reward->date);
    fclose(fp);
    return true;
}

void generateNextRewardId(char* out_id) {
    RewardEntry rewards[MAX_REWARDS];
    int count = loadRewards(rewards, MAX_REWARDS);
    int max_num = 1000;
    for (int i = 0; i < count; i++) {
        if (strncmp(rewards[i].id, "REW", 3) == 0) {
            int num = atoi(rewards[i].id + 3);
            if (num > max_num) max_num = num;
        }
    }
    sprintf(out_id, "REW%d", max_num + 1);
}

/* ==================== SMART BINS OPERATIONS ==================== */

int loadBins(SmartBin bins[], int max_count) {
    FILE* fp = fopen(FILE_BINS, "r");
    if (!fp) return 0;

    char line[512];
    int count = 0;
    while (fgets(line, sizeof(line), fp) && count < max_count) {
        trimString(line);
        if (strlen(line) == 0) continue;

        char* f[6];
        int num_fields = splitLine(line, f, 6, '|');
        if (num_fields >= 4) {
            strncpy(bins[count].bin_id, f[0], sizeof(bins[count].bin_id) - 1);
            strncpy(bins[count].location, f[1], sizeof(bins[count].location) - 1);
            strncpy(bins[count].category, f[2], sizeof(bins[count].category) - 1);
            strncpy(bins[count].status, f[3], sizeof(bins[count].status) - 1);
            count++;
        }
    }
    fclose(fp);
    return count;
}

bool saveBins(const SmartBin bins[], int count) {
    FILE* fp = fopen(FILE_BINS, "w");
    if (!fp) return false;

    for (int i = 0; i < count; i++) {
        fprintf(fp, "%s|%s|%s|%s\n",
                bins[i].bin_id,
                bins[i].location,
                bins[i].category,
                bins[i].status);
    }
    fclose(fp);
    return true;
}

bool addBin(const SmartBin* bin) {
    FILE* fp = fopen(FILE_BINS, "a");
    if (!fp) return false;

    fprintf(fp, "%s|%s|%s|%s\n",
            bin->bin_id,
            bin->location,
            bin->category,
            bin->status);
    fclose(fp);
    return true;
}

bool updateBin(const SmartBin* bin) {
    SmartBin bins[MAX_BINS];
    int count = loadBins(bins, MAX_BINS);
    bool found = false;
    for (int i = 0; i < count; i++) {
        if (strcmp(bins[i].bin_id, bin->bin_id) == 0) {
            bins[i] = *bin;
            found = true;
            break;
        }
    }
    if (found) {
        return saveBins(bins, count);
    }
    return false;
}

bool deleteBin(const char* bin_id) {
    SmartBin bins[MAX_BINS];
    int count = loadBins(bins, MAX_BINS);
    int new_count = 0;
    SmartBin updated[MAX_BINS];

    for (int i = 0; i < count; i++) {
        if (strcmp(bins[i].bin_id, bin_id) != 0) {
            updated[new_count++] = bins[i];
        }
    }
    if (new_count < count) {
        return saveBins(updated, new_count);
    }
    return false;
}

/* ==================== COLLECTOR OPERATIONS ==================== */

int loadCollectors(Collector collectors[], int max_count) {
    FILE* fp = fopen(FILE_COLLECTORS, "r");
    if (!fp) return 0;

    char line[512];
    int count = 0;
    while (fgets(line, sizeof(line), fp) && count < max_count) {
        trimString(line);
        if (strlen(line) == 0) continue;

        char* f[12];
        int num_fields = splitLine(line, f, 12, '|');
        if (num_fields >= 8) {
            strncpy(collectors[count].id, f[0], sizeof(collectors[count].id) - 1);
            strncpy(collectors[count].name, f[1], sizeof(collectors[count].name) - 1);
            strncpy(collectors[count].email, f[2], sizeof(collectors[count].email) - 1);
            strncpy(collectors[count].phone, f[3], sizeof(collectors[count].phone) - 1);
            strncpy(collectors[count].password, f[4], sizeof(collectors[count].password) - 1);
            strncpy(collectors[count].assigned_truck, f[5], sizeof(collectors[count].assigned_truck) - 1);
            strncpy(collectors[count].status, f[6], sizeof(collectors[count].status) - 1);
            collectors[count].total_collected_kg = strtof(f[7], NULL);
            if (num_fields >= 9) {
                collectors[count].tasks_completed = atoi(f[8]);
            } else {
                collectors[count].tasks_completed = 0;
            }
            count++;
        }
    }
    fclose(fp);
    return count;
}

bool saveCollectors(const Collector collectors[], int count) {
    FILE* fp = fopen(FILE_COLLECTORS, "w");
    if (!fp) return false;

    for (int i = 0; i < count; i++) {
        fprintf(fp, "%s|%s|%s|%s|%s|%s|%s|%.2f|%d\n",
                collectors[i].id,
                collectors[i].name,
                collectors[i].email,
                collectors[i].phone,
                collectors[i].password,
                collectors[i].assigned_truck,
                collectors[i].status,
                collectors[i].total_collected_kg,
                collectors[i].tasks_completed);
    }
    fclose(fp);
    return true;
}

bool addCollector(const Collector* col) {
    FILE* fp = fopen(FILE_COLLECTORS, "a");
    if (!fp) return false;

    fprintf(fp, "%s|%s|%s|%s|%s|%s|%s|%.2f|%d\n",
            col->id,
            col->name,
            col->email,
            col->phone,
            col->password,
            col->assigned_truck,
            col->status,
            col->total_collected_kg,
            col->tasks_completed);
    fclose(fp);
    return true;
}

bool updateCollector(const Collector* col) {
    Collector collectors[MAX_COLLECTORS];
    int count = loadCollectors(collectors, MAX_COLLECTORS);
    bool found = false;
    for (int i = 0; i < count; i++) {
        if (strcmp(collectors[i].id, col->id) == 0) {
            collectors[i] = *col;
            found = true;
            break;
        }
    }
    if (found) {
        return saveCollectors(collectors, count);
    }
    return false;
}

bool deleteCollector(const char* collector_id) {
    Collector collectors[MAX_COLLECTORS];
    int count = loadCollectors(collectors, MAX_COLLECTORS);
    int new_count = 0;
    Collector updated[MAX_COLLECTORS];

    for (int i = 0; i < count; i++) {
        if (strcmp(collectors[i].id, collector_id) != 0) {
            updated[new_count++] = collectors[i];
        }
    }
    if (new_count < count) {
        return saveCollectors(updated, new_count);
    }
    return false;
}

Collector* findCollectorByIdOrEmail(Collector collectors[], int count, const char* identifier) {
    for (int i = 0; i < count; i++) {
        if (strcmp(collectors[i].id, identifier) == 0 || strcmp(collectors[i].email, identifier) == 0) {
            return &collectors[i];
        }
    }
    return NULL;
}

Collector* findCollectorById(Collector collectors[], int count, const char* id) {
    for (int i = 0; i < count; i++) {
        if (strcmp(collectors[i].id, id) == 0) {
            return &collectors[i];
        }
    }
    return NULL;
}

void generateNextCollectorId(char* out_id) {
    Collector collectors[MAX_COLLECTORS];
    int count = loadCollectors(collectors, MAX_COLLECTORS);
    int max_num = 100;
    for (int i = 0; i < count; i++) {
        if (strncmp(collectors[i].id, "COL", 3) == 0) {
            int num = atoi(collectors[i].id + 3);
            if (num > max_num) max_num = num;
        }
    }
    sprintf(out_id, "COL%d", max_num + 1);
}

/* ==================== TRUCK FLEET OPERATIONS ==================== */

int loadTrucks(Truck trucks[], int max_count) {
    FILE* fp = fopen(FILE_TRUCKS, "r");
    if (!fp) return 0;

    char line[512];
    int count = 0;
    while (fgets(line, sizeof(line), fp) && count < max_count) {
        trimString(line);
        if (strlen(line) == 0) continue;

        char* f[8];
        int num_fields = splitLine(line, f, 8, '|');
        if (num_fields >= 6) {
            strncpy(trucks[count].id, f[0], sizeof(trucks[count].id) - 1);
            strncpy(trucks[count].plate_number, f[1], sizeof(trucks[count].plate_number) - 1);
            strncpy(trucks[count].driver_name, f[2], sizeof(trucks[count].driver_name) - 1);
            trucks[count].capacity_kg = strtof(f[3], NULL);
            trucks[count].current_load_kg = strtof(f[4], NULL);
            strncpy(trucks[count].status, f[5], sizeof(trucks[count].status) - 1);
            count++;
        }
    }
    fclose(fp);
    return count;
}

bool saveTrucks(const Truck trucks[], int count) {
    FILE* fp = fopen(FILE_TRUCKS, "w");
    if (!fp) return false;

    for (int i = 0; i < count; i++) {
        fprintf(fp, "%s|%s|%s|%.2f|%.2f|%s\n",
                trucks[i].id,
                trucks[i].plate_number,
                trucks[i].driver_name,
                trucks[i].capacity_kg,
                trucks[i].current_load_kg,
                trucks[i].status);
    }
    fclose(fp);
    return true;
}

bool addTruck(const Truck* truck) {
    FILE* fp = fopen(FILE_TRUCKS, "a");
    if (!fp) return false;

    fprintf(fp, "%s|%s|%s|%.2f|%.2f|%s\n",
            truck->id,
            truck->plate_number,
            truck->driver_name,
            truck->capacity_kg,
            truck->current_load_kg,
            truck->status);
    fclose(fp);
    return true;
}

bool updateTruck(const Truck* truck) {
    Truck trucks[MAX_TRUCKS];
    int count = loadTrucks(trucks, MAX_TRUCKS);
    bool found = false;
    for (int i = 0; i < count; i++) {
        if (strcmp(trucks[i].id, truck->id) == 0) {
            trucks[i] = *truck;
            found = true;
            break;
        }
    }
    if (found) {
        return saveTrucks(trucks, count);
    }
    return false;
}

bool deleteTruck(const char* truck_id) {
    Truck trucks[MAX_TRUCKS];
    int count = loadTrucks(trucks, MAX_TRUCKS);
    int new_count = 0;
    Truck updated[MAX_TRUCKS];

    for (int i = 0; i < count; i++) {
        if (strcmp(trucks[i].id, truck_id) != 0) {
            updated[new_count++] = trucks[i];
        }
    }
    if (new_count < count) {
        return saveTrucks(updated, new_count);
    }
    return false;
}

Truck* findTruckById(Truck trucks[], int count, const char* id) {
    for (int i = 0; i < count; i++) {
        if (strcmp(trucks[i].id, id) == 0) {
            return &trucks[i];
        }
    }
    return NULL;
}

void generateNextTruckId(char* out_id) {
    Truck trucks[MAX_TRUCKS];
    int count = loadTrucks(trucks, MAX_TRUCKS);
    int max_num = 0;
    for (int i = 0; i < count; i++) {
        if (strncmp(trucks[i].id, "TRK-", 4) == 0) {
            int num = atoi(trucks[i].id + 4);
            if (num > max_num) max_num = num;
        }
    }
    sprintf(out_id, "TRK-%02d", max_num + 1);
}

/* ==================== RECYCLABLE SALES OPERATIONS ==================== */

int loadSales(RecyclableSale sales[], int max_count) {
    FILE* fp = fopen(FILE_SALES, "r");
    if (!fp) return 0;

    char line[512];
    int count = 0;
    while (fgets(line, sizeof(line), fp) && count < max_count) {
        trimString(line);
        if (strlen(line) == 0) continue;

        char* f[10];
        int num_fields = splitLine(line, f, 10, '|');
        if (num_fields >= 7) {
            strncpy(sales[count].id, f[0], sizeof(sales[count].id) - 1);
            strncpy(sales[count].material_type, f[1], sizeof(sales[count].material_type) - 1);
            sales[count].weight_kg = strtof(f[2], NULL);
            strncpy(sales[count].buyer_name, f[3], sizeof(sales[count].buyer_name) - 1);
            sales[count].rate_per_kg = strtof(f[4], NULL);
            sales[count].total_amount = strtof(f[5], NULL);
            strncpy(sales[count].sale_date, f[6], sizeof(sales[count].sale_date) - 1);
            count++;
        }
    }
    fclose(fp);
    return count;
}

bool addSale(const RecyclableSale* sale) {
    FILE* fp = fopen(FILE_SALES, "a");
    if (!fp) return false;

    fprintf(fp, "%s|%s|%.2f|%s|%.2f|%.2f|%s\n",
            sale->id,
            sale->material_type,
            sale->weight_kg,
            sale->buyer_name,
            sale->rate_per_kg,
            sale->total_amount,
            sale->sale_date);
    fclose(fp);
    return true;
}

void generateNextSaleId(char* out_id) {
    RecyclableSale sales[MAX_SALES];
    int count = loadSales(sales, MAX_SALES);
    int max_num = 1000;
    for (int i = 0; i < count; i++) {
        if (strncmp(sales[i].id, "SALE", 4) == 0) {
            int num = atoi(sales[i].id + 4);
            if (num > max_num) max_num = num;
        }
    }
    sprintf(out_id, "SALE%d", max_num + 1);
}

/* ==================== MUNICIPAL ADMINISTRATOR OPERATIONS ==================== */

int loadAdmins(Admin admins[], int max_count) {
    FILE* fp = fopen(FILE_ADMINS, "r");
    if (!fp) return 0;

    char line[512];
    int count = 0;
    while (fgets(line, sizeof(line), fp) && count < max_count) {
        trimString(line);
        if (strlen(line) == 0) continue;

        char* f[10];
        int num_fields = splitLine(line, f, 10, '|');
        if (num_fields >= 7) {
            strncpy(admins[count].id, f[0], sizeof(admins[count].id) - 1);
            strncpy(admins[count].username, f[1], sizeof(admins[count].username) - 1);
            strncpy(admins[count].name, f[2], sizeof(admins[count].name) - 1);
            strncpy(admins[count].phone, f[3], sizeof(admins[count].phone) - 1);
            strncpy(admins[count].password, f[4], sizeof(admins[count].password) - 1);
            strncpy(admins[count].role, f[5], sizeof(admins[count].role) - 1);
            strncpy(admins[count].created_at, f[6], sizeof(admins[count].created_at) - 1);
            count++;
        }
    }
    fclose(fp);
    return count;
}

bool saveAdmins(const Admin admins[], int count) {
    FILE* fp = fopen(FILE_ADMINS, "w");
    if (!fp) return false;

    for (int i = 0; i < count; i++) {
        fprintf(fp, "%s|%s|%s|%s|%s|%s|%s\n",
                admins[i].id,
                admins[i].username,
                admins[i].name,
                admins[i].phone,
                admins[i].password,
                admins[i].role,
                admins[i].created_at);
    }
    fclose(fp);
    return true;
}

bool addAdmin(const Admin* admin) {
    FILE* fp = fopen(FILE_ADMINS, "a");
    if (!fp) return false;

    fprintf(fp, "%s|%s|%s|%s|%s|%s|%s\n",
            admin->id,
            admin->username,
            admin->name,
            admin->phone,
            admin->password,
            admin->role,
            admin->created_at);
    fclose(fp);
    return true;
}

bool updateAdmin(const Admin* admin) {
    static Admin admins[MAX_ADMINS];
    int count = loadAdmins(admins, MAX_ADMINS);
    bool found = false;
    for (int i = 0; i < count; i++) {
        if (strcmp(admins[i].id, admin->id) == 0) {
            admins[i] = *admin;
            found = true;
            break;
        }
    }
    if (found) {
        return saveAdmins(admins, count);
    }
    return false;
}

Admin* findAdminByUsernameOrEmail(Admin admins[], int count, const char* identifier) {
    for (int i = 0; i < count; i++) {
        if (strcmp(admins[i].username, identifier) == 0) {
            return &admins[i];
        }
    }
    return NULL;
}

Admin* findAdminById(Admin admins[], int count, const char* id) {
    for (int i = 0; i < count; i++) {
        if (strcmp(admins[i].id, id) == 0) {
            return &admins[i];
        }
    }
    return NULL;
}

void generateNextAdminId(char* out_id) {
    static Admin admins[MAX_ADMINS];
    int count = loadAdmins(admins, MAX_ADMINS);
    int max_num = 100;
    for (int i = 0; i < count; i++) {
        if (strncmp(admins[i].id, "ADM", 3) == 0) {
            int num = atoi(admins[i].id + 3);
            if (num > max_num) max_num = num;
        }
    }
    sprintf(out_id, "ADM%d", max_num + 1);
}
