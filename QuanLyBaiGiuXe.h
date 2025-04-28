#ifndef QUAN_LY_BAI_GIU_XE_H
#define QUAN_LY_BAI_GIU_XE_H

#include <gtk/gtk.h>
#include <time.h>

#define HOURLY_RATE 5000
#define MAX_SLOTS 100

typedef struct {
    char license_plate[20];
    int fee;
    time_t entry_time;
    clock_t clock_start;
    int floor;
} vehicle;

typedef struct {
    GtkListStore *store_tang1;
    GtkListStore *store_tang2;
    GtkWindow *parent_window;
} SharedData;

// Biến toàn cục
extern vehicle vehicle_list[MAX_SLOTS];
extern int num_vehicles;
extern double doanh_thu;
extern GtkWidget *label_stats;

// Hàm
void save_doanh_thu();
void load_doanh_thu();
void save_parking_data();
void load_treeviews(SharedData *shared_data);
void load_history_data(GtkListStore *store);
void read_from_file();
vehicle* find_vehicle(const char *license_plate);
int Check__license_plate(const char *a);
void Cal_total(double fee);
void update_statistics_display();
void log_action(const char *license_plate, const char *action, int fee);
int has_available_slot();
void filter_treeviews(SharedData *shared_data, const char *keyword);

// Callback
void onNhapBienSoXe(GtkWidget *widget, gpointer data);
void ThanhtoanvaXoa(GtkWidget *widget, gpointer data);
void on_search_changed(GtkEditable *entry, gpointer user_data);

#endif // QUAN_LY_BAI_GIU_XE_H
