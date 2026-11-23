#ifndef MODELS_H
#define MODELS_H

#include "common.h"

/* Lifecycle stages for waste collection requests */
typedef enum {
    REQ_STATUS_PENDING = 0,         
    REQ_STATUS_APPROVED = 1,      
    REQ_STATUS_ASSIGNED = 2,        
    REQ_STATUS_IN_PROGRESS = 3,     
    REQ_STATUS_COLLECTED = 4,     
    REQ_STATUS_FINAL_OUTCOME = 5   
} RequestStatus;

/* Citizen / User account structure */
typedef struct {
    char id[16];            
    char name[64];
    char email[64];
    char phone[20];
    char password[64];
    int reward_points;
    char created_at[32];
} User;

/* Waste collection request submitted by citizens */
typedef struct {
    char id[16];                
    char user_id[16];           
    char waste_type[32];       
    float estimated_weight;    
    float actual_weight;       
    char location[128];        
    char bin_id[16];            
    char preferred_date[20];   
    char preferred_slot[32];   
    char special_notes[128];    
    RequestStatus status;     
    char assigned_collector[96];
    char assigned_truck[32];  
    char final_outcome[384];   
    int points_awarded;        
    char created_at[32];       
    char completed_at[32];     
} WasteRequest;

/* Public bin problem reports submitted by citizens */
typedef struct {
    char id[16];             
    char user_id[16];
    char bin_id[16];
    char location[128];
    char problem_type[64];      
    char description[256];
    char status[32];           
    char created_at[32];
} BinProblemReport;

/* Collection problems reported by field collectors */
typedef struct {
    char id[16];               
    char collector_id[16];
    char request_id[16];
    char problem_type[64];     
    char description[256];
    char status[32];            
    char created_at[32];
} CollectorProblemReport;

/* Verified reward point history ledger */
typedef struct {
    char id[16];             
    char user_id[16];
    char request_id[16];
    char waste_type[32];
    float weight;
    int points;
    char date[32];
} RewardEntry;

/* Public smart bin details */
typedef struct {
    char bin_id[16];
    char location[128];
    char category[32];
    char status[32];            
} SmartBin;

/* Field waste collector staff */
typedef struct {
    char id[16];               
    char name[64];
    char email[64];
    char phone[20];
    char password[64];
    char assigned_truck[16];    
    char status[32];            
    float total_collected_kg;   
    int tasks_completed;        
} Collector;

/* Municipal collection vehicle */
typedef struct {
    char id[16];               
    char plate_number[32];      
    char driver_name[64];       
    float capacity_kg;         
    float current_load_kg;     
    char status[32];            
} Truck;

/* Commercial sale record for recyclable materials */
typedef struct {
    char id[16];
    char material_type[32];
    float weight_kg;
    char buyer_name[64];
    float rate_per_kg;
    float total_amount;
    char sale_date[32];
    char batch_id[16];          /* Linked warehouse batch ID */
    char admin_id[16];          /* Admin who completed the sale */
} RecyclableSale;

/* Municipal Administrator account */
typedef struct {
    char id[16];
    char username[64];
    char name[64];
    char phone[20];
    char password[64];
    char role[32];
    char created_at[32];
    int admin_points;           /* Points earned from recycling & selling waste */
    float total_revenue;        /* Total commercial sales revenue earned */
} Admin;

/* Admin warehouse inventory batch created when a collector delivers waste */
typedef struct {
    char batch_id[16];          /* e.g., "BAT1001" */
    char request_id[16];        /* Linked waste request ID */
    char collector_id[16];      /* Collector who collected the waste */
    char waste_type[32];        /* Waste category */
    float weight_kg;            /* Verified scale weight in kg */
    char stage[32];             /* "Collected", "Recycled", "Sold", "Disposed" */
    char recycle_details[256];  /* Recycling or safe disposal details */
    char sale_id[16];           /* Linked sale ID once sold, or "N/A" */
    float sale_amount;          /* Revenue earned from sale */
    int admin_points_earned;    /* Points added to Admin Portal */
    char processed_by[16];      /* Admin ID who handled recycling/sale */
    char updated_at[32];        /* Last update timestamp */
} WarehouseBatch;

/* Eco-Store items available for points redemption */
typedef struct {
    char id[16];                /* e.g., "ITEM101" */
    char name[64];              /* e.g., "Stainless Steel Water Bottle" */
    char category[32];          /* e.g., "Eco Living", "Waste Management", "Sanitation" */
    int points_cost;            /* Cost in reward points */
    int stock_qty;              /* Available inventory stock */
    char description[128];      /* Product description */
} StoreItem;

/* Citizen store purchase / redemption record */
typedef struct {
    char purchase_id[16];       /* e.g., "PUR1001" */
    char user_id[16];           /* Citizen user ID */
    char user_name[64];         /* Citizen name */
    char item_id[16];           /* Store item ID */
    char item_name[64];         /* Store item name */
    int quantity;               /* Quantity ordered */
    int points_spent;           /* Total points spent */
    char delivery_address[128]; /* Delivery location / address */
    char status[32];            /* "Confirmed", "Dispatched", "Delivered" */
    char purchase_date[32];     /* Date and time of purchase */
} ItemPurchase;

#endif /* MODELS_H */
