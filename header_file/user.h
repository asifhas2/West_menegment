#ifndef USER_H
#define USER_H

#include "models.h"

/* Main entry point for Citizen / User portal */
void handleUserModule(void);

/* Registration & Login authentication */
bool handleUserRegistration(void);
bool handleUserLogin(User* loggedInUser);

/* Citizen dashboard and operational features */
void showUserDashboard(User* user);
void submitWasteRequestUI(User* user);
void trackUserRequestsUI(const User* user);
void viewRequestDetailsUI(const User* user);
void reportBinProblemUI(const User* user);
void viewCollectionHistoryUI(const User* user);
void viewRewardSummaryUI(const User* user);
void viewWasteOutcomeTransparencyUI(const User* user);
void viewUserProfileUI(const User* user);

#endif /* USER_H */
