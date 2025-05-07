#ifndef PARKING_H
#define PARKING_H

#include <gtk/gtk.h>
#include <stdbool.h>
#include <time.h>

#define MAX_SLOTS 50
#define MAX_TANG 4
#define XE_MAY 2000
#define O_TO 5000
#define revenue_FILE "revenue_theo_ngay.txt"

typedef enum {
    xe_may,
    o_to
} VehicleType;

typedef struct {
    char license_plate[20];
    int fee;
    time_t entry_time;
    clock_t clock_start;
    int floor;
    VehicleType type;
} vehicle;

typedef struct {
    GtkListStore *store_tangs[MAX_TANG];
    GtkListStore *history_store;
    GtkWindow *parent_window;
    GtkWidget *home_stat_label;
} SharedData;

extern SharedData shared_data;
extern vehicle vehicle_list[MAX_SLOTS];
extern int num_vehicle;
extern double revenue;
extern GtkWidget *label_stats;
extern GtkWidget *treeview_revenue;
extern GtkWidget *label_vehicle_count;
extern GtkWidget *label_thongke;

// Function declarations
void load_history_data(GtkListStore *store);
void update_history_data(SharedData *shared_data);
void show_floor_statistics(SharedData *shared_data);
void update_vehicle_count_label();
int check_license_plate(const char *a);
void load_vehicle_tree(SharedData *shared_data);
bool license_plate_exists(const char *plate);
vehicle create_new_vehicle(const char *plate, int floor, VehicleType type);
void show_errori(GtkWindow *parent, const char *msg);
vehicle* find_vehicle(const char *license_plate);
int find_vehicle_index(const char *plate);
int calculate_parking_fee(vehicle *veh);
void remove_vehicle_at_index(int index);
void show_payment_result(GtkWindow *parent, const vehicle *veh, int fee);
void calculate_total_revenue(double fee);
char* floor_statistics(void);
char* format_currency(double amount);
void update_statistics_display();
void search_filter(SharedData *shared_data, const char *keyword);
void write_log(const char *license_plate, const char *action, int fee);
void save_parking_data_to_file();
void read_from_file();
void save_revenue();
void load_revenue();
void update_daily_revenue(int so_tien);
void summarize_revenue_by_day();
void display_revenue_treeview(GtkTreeView *treeview);
void set_font_size(int size);
void search(GtkEditable *entry, gpointer user_data);
void changes(GtkTreeView *tree_view, GtkTreePath *path, GtkTreeViewColumn *column, gpointer user_data);
void enter_license_plate(GtkWidget *widget, gpointer data);
void pay_and_remove(GtkWidget *widget, gpointer data);
#endif