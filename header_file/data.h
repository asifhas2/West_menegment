#ifndef DATA_H
#define DATA_H

#include "models.h"

/* Storage initialization and default mock data */
bool initDataFiles(void);

/* Citizen / User management */
int loadUsers(User users[], int max_count);
bool saveUsers(const User users[], int count);
bool addUser(const User* user);
bool updateUser(const User* user);
bool deleteUser(const char* user_id);
User* findUserByEmailOrPhone(User users[], int count, const char* identifier);
User* findUserById(User users[], int count, const char* id);
void generateNextUserId(char* out_id);

/* Waste collection request operations */
int loadRequests(WasteRequest requests[], int max_count);
bool saveRequests(const WasteRequest requests[], int count);
bool addRequest(const WasteRequest* req);
bool updateRequest(const WasteRequest* req);
WasteRequest* findRequestById(WasteRequest requests[], int count, const char* id);
void generateNextRequestId(char* out_id);

/* Citizen public bin problem reports */
int loadBinReports(BinProblemReport reports[], int max_count);
bool addBinReport(const BinProblemReport* report);
bool updateBinReport(const BinProblemReport* report);
void generateNextReportId(char* out_id);

/* Field collector problem reports */
int loadCollectorReports(CollectorProblemReport reports[], int max_count);
bool addCollectorReport(const CollectorProblemReport* report);
bool updateCollectorReport(const CollectorProblemReport* report);
void generateNextCollectorReportId(char* out_id);

/* Reward point transactions */
int loadRewards(RewardEntry rewards[], int max_count);
bool addReward(const RewardEntry* reward);
void generateNextRewardId(char* out_id);

/* Public smart bins */
int loadBins(SmartBin bins[], int max_count);
bool saveBins(const SmartBin bins[], int count);
bool addBin(const SmartBin* bin);
bool updateBin(const SmartBin* bin);
bool deleteBin(const char* bin_id);

/* Waste Collector staff management */
int loadCollectors(Collector collectors[], int max_count);
bool saveCollectors(const Collector collectors[], int count);
bool addCollector(const Collector* col);
bool updateCollector(const Collector* col);
bool deleteCollector(const char* collector_id);
Collector* findCollectorByIdOrEmail(Collector collectors[], int count, const char* identifier);
Collector* findCollectorById(Collector collectors[], int count, const char* id);
void generateNextCollectorId(char* out_id);

/* Trucks / Fleet management */
int loadTrucks(Truck trucks[], int max_count);
bool saveTrucks(const Truck trucks[], int count);
bool addTruck(const Truck* truck);
bool updateTruck(const Truck* truck);
bool deleteTruck(const char* truck_id);
Truck* findTruckById(Truck trucks[], int count, const char* id);
void generateNextTruckId(char* out_id);

/* Commercial recyclable sales records */
int loadSales(RecyclableSale sales[], int max_count);
bool addSale(const RecyclableSale* sale);
void generateNextSaleId(char* out_id);

/* Municipal Administrator operations */
int loadAdmins(Admin admins[], int max_count);
bool saveAdmins(const Admin admins[], int count);
bool addAdmin(const Admin* admin);
bool updateAdmin(const Admin* admin);
Admin* findAdminByUsernameOrEmail(Admin admins[], int count, const char* identifier);
Admin* findAdminById(Admin admins[], int count, const char* id);
void generateNextAdminId(char* out_id);

#endif /* DATA_H */
