#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include <ctype.h>

/* Maximum capacities for records in memory */
#define MAX_USERS 500
#define MAX_REQUESTS 1000
#define MAX_BIN_REPORTS 500
#define MAX_COLLECTOR_REPORTS 500
#define MAX_REWARDS 1000
#define MAX_BINS 100
#define MAX_COLLECTORS 50
#define MAX_TRUCKS 50
#define MAX_SALES 500
#define MAX_ADMINS 50

/* Data file paths for system persistence */
#define FILE_USERS "data/users.txt"
#define FILE_REQUESTS "data/requests.txt"
#define FILE_BIN_REPORTS "data/bin_reports.txt"
#define FILE_COLLECTOR_REPORTS "data/collector_reports.txt"
#define FILE_REWARDS "data/rewards.txt"
#define FILE_BINS "data/bins.txt"
#define FILE_COLLECTORS "data/collectors.txt"
#define FILE_TRUCKS "data/trucks.txt"
#define FILE_SALES "data/sales.txt"
#define FILE_ADMINS "data/admins.txt"

#endif /* COMMON_H */
