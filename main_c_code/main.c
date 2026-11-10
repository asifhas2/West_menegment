#include "common.h"
#include "data.h"
#include "utils.h"
#include "user.h"
#include "collector.h"
#include "admin.h"

static void showSystemArchitecture(void) {
    clearScreen();
    printHeader("SMART WASTE MANAGEMENT SYSTEM - ARCHITECTURE OVERVIEW");
    printf(" Community Smart Waste Management & Transparency Platform\n");
    printf(" Language : C (Standard C99 / C11)\n");
    printf(" Interface: Console / Terminal\n");
    printDivider('=', 72);
    printf(" 3 Integrated System Portals:\n\n");
    printf("   1. Citizen / User Portal:\n");
    printf("      * Account registration and credential authentication\n");
    printf("      * Real-time waste collection request scheduling\n");
    printf("      * 6-stage lifecycle tracking with visual progress indicator\n");
    printf("      * Public smart bin problem and overflowing reporting\n");
    printf("      * Verified collection history and reward points balance\n");
    printf("      * End-to-end transparent waste outcome trail\n\n");
    printf("   2. Waste Collector Portal:\n");
    printf("      * Field officer login and route overview\n");
    printf("      * Assigned task inspection and on-route activation\n");
    printf("      * Physical scale weight recording and instant reward grant\n");
    printf("      * Truck payload update and field incident reporting\n\n");
    printf("   3. Municipal Administrator Portal:\n");
    printf("      * Operations dashboard with real-time system metrics\n");
    printf("      * Request validation, approval, and collector/truck scheduling\n");
    printf("      * Route generation and dispatch to field staff\n");
    printf("      * Processing outcomes (recycling facilities & chemical safe disposal)\n");
    printf("      * Commercial sale records for recyclable commodities\n");
    printf("      * Full management for citizens, smart bins, collectors, and fleet\n");
    printf("      * Incident resolution and comprehensive statistical analytics\n");
    printDivider('=', 72);
    pausePrompt();
}

int main(void) {
    /* Initialize data files and default records if first launch */
    initDataFiles();

    while (1) {
        clearScreen();
        printHeader("COMMUNITY SMART WASTE MANAGEMENT SYSTEM (C-WMS)");
        printf(" Welcome! Please select your portal to proceed:\n\n");
        printf("   [1] Citizen / User Portal\n");
        printf("   [2] Waste Collector Portal\n");
        printf("   [3] Municipal Admin Portal\n");
        printf("   [4] System Architecture & Guide\n");
        printf("   [5] Exit System\n");
        printDivider('=', 70);

        int choice = getInt(" Select Actor [1-5]: ", 1, 5);
        switch (choice) {
            case 1:
                handleUserModule();
                break;
            case 2:
                handleCollectorModule();
                break;
            case 3:
                handleAdminModule();
                break;
            case 4:
                showSystemArchitecture();
                break;
            case 5:
                clearScreen();
                printHeader("THANK YOU FOR USING SMART WASTE MANAGEMENT SYSTEM");
                printf(" Protecting our environment through transparent recycling and disposal.\n");
                printf(" Goodbye!\n\n");
                return 0;
        }
    }

    return 0;
}
