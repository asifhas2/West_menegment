#ifndef ADMIN_H
#define ADMIN_H

#include "models.h"

/* Main entry point for Municipal Admin portal */
void handleAdminModule(void);

/* Admin authentication & staff registration */
bool handleAdminLogin(Admin* loggedInAdmin);
bool handleAdminRegistration(void);

/* Admin dashboard & core operations */
void showAdminDashboard(Admin* admin);
void manageRequestsUI(void);
void approveOrRejectRequestUI(void);
void assignCollectorAndTruckUI(void);
void manageUsersUI(void);
void manageBinsUI(void);
void manageCollectorsUI(void);
void manageTrucksUI(void);
void routeDispatchUI(void);
void manageProcessingRecordsUI(Admin* admin);
void recordRecyclableSaleUI(Admin* admin);
void viewAdminWarehouseUI(const Admin* admin);
void viewTransparencyTrailUI(void);
void viewStorePurchasesUI(void);
void manageProblemReportsUI(void);
void generateSystemReportsUI(void);

#endif /* ADMIN_H */
