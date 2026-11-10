#ifndef COLLECTOR_H
#define COLLECTOR_H

#include "models.h"

/* Main entry point for Waste Collector actor */
void handleCollectorModule(void);

/* Collector authentication & registration */
bool handleCollectorLogin(Collector* loggedInCollector);
bool handleCollectorRegistration(void);

/* Collector dashboard and operations */
void showCollectorDashboard(Collector* collector);
void viewAssignedTasksUI(const Collector* collector);
void viewTaskDetailsUI(const Collector* collector);
void viewCollectionRouteUI(const Collector* collector);
void startCollectionUI(Collector* collector);
void completeCollectionUI(Collector* collector);
void reportCollectionProblemUI(const Collector* collector);
void viewCollectorPerformanceUI(const Collector* collector);

#endif /* COLLECTOR_H */
