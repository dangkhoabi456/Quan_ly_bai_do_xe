#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <stdbool.h>  
#define MAX_SLOTS 50
#define MAX_TANG 4
#define XE_MAY 2000
#define O_TO 5000
#include <glib.h> // Để sử dụng g_strdup và g_free
#define revenue_FILE "revenue_theo_ngay.txt" 
#ifndef DOANHTHU_H
#define DOANHTHU_H

void update_daily_revenue(int so_tien);
void summarize_revenue_by_day();
int calculate_monthly_total_revenue(int thang, int nam);
int calculate_yearly_total_revenue(int nam);
void display_revenue_treeview(GtkTreeView *treeview);  

#endif

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
    GtkListStore *history_store;  // Thêm dòng này
    GtkWindow *parent_window;
    GtkWidget *home_stat_label;
} SharedData;
SharedData shared_data;   // biến kiểu struct	

vehicle vehicle_list[MAX_SLOTS];
void load_history_data(GtkListStore *store);
int num_vehicle = 0;
double revenue = 0;  
void update_history_data(SharedData *shared_data);
GtkWidget *label_stats; 
GtkWidget *treeview_revenue;
GtkWidget *label_vehicle_count; //đếm xe trong bãi	
void calculate_total_revenue(double fee);  
void load_vehicle_tree(SharedData *shared_data);
int check_empty_slot();
char* format_currency(double amount);
int find_vehicle_index(const char *plate);
int calculate_parking_fee(vehicle *veh);
void remove_vehicle_at_index(int index);
void show_payment_result(GtkWindow *parent, const vehicle *veh, int fee);
static void  pay_and_remove(GtkWidget *widget, gpointer data);
static void changes(GtkTreeView *tree_view, GtkTreePath *path, GtkTreeViewColumn *column, gpointer user_data);
bool license_plate_exists(const char *plate);
vehicle create_new_vehicle(const char *plate, int floor, VehicleType type);
void show_errori(GtkWindow *parent, const char *msg);
static void enter_license_plate(GtkWidget *widget, gpointer data);
int check_license_plate(const char *a);  // Khai báo prototype
void read_data_from_file(SharedData *shared_data);
vehicle*  find_vehicle(const char *license_plate);
void update_statistics_display();
static void search(GtkEditable *entry, gpointer user_data);
void save_revenue();
void load_revenue();
void save_parking_data_to_file();
void search_filter(SharedData *shared_data, const char *keyword);
char* floor_statistics(void);
void write_log(const char *license_plate, const char *action, int fee);
GtkWidget *label_thongke;
void update_vehicle_count_label();
void show_floor_statistics(SharedData *shared_data);
void set_font_size(int size);

// Hàm lưu doanh thu
void save_revenue() {
    FILE *f = fopen("revenue.txt", "w");
    if (f) {
        fprintf(f, "%.0f", revenue);
        fclose(f);
    }
}

// Hàm đọc doanh thu
void load_revenue() {
    FILE *f = fopen("revenue.txt", "r");
    if (f) {
        fscanf(f, "%lf", &revenue);
        fclose(f);
    }
}

// Thay đổi hàm read_from_file() để tương thích Windows
void read_from_file() {
    FILE *pt = fopen("parking_data.txt", "r");
    if (pt == NULL) {
        pt = fopen("parking_data.txt", "w"); // Tạo file nếu chưa có
        if (pt) fclose(pt);
        return;
    }

    vehicle temp;
    int year, mon, day, hour, min, sec;
    char type_str[10];
    num_vehicle = 0;

    while (fscanf(pt, "%s %d %d-%d-%d %d:%d:%d %d %s",
                  temp.license_plate, &temp.fee,
                  &year, &mon, &day, &hour, &min, &sec, &temp.floor, type_str) == 10) {

        struct tm tm_time = {0};
        tm_time.tm_year = year - 1900;
        tm_time.tm_mon = mon - 1;
        tm_time.tm_mday = day;
        tm_time.tm_hour = hour;
        tm_time.tm_min = min;
        tm_time.tm_sec = sec;

        temp.entry_time = mktime(&tm_time);
        temp.clock_start = clock() - (clock_t)(difftime(time(NULL), temp.entry_time) * CLOCKS_PER_SEC);

        // Phân loại xe
        if (strcmp(type_str, "O_TO") == 0) {
            temp.type = o_to;
        } else {
            temp.type = xe_may;
        }

        if (num_vehicle < MAX_SLOTS) {
            vehicle_list[num_vehicle++] = temp;
        }
    }
    fclose(pt);
}

void write_log(const char *license_plate, const char *action, int fee) {
    FILE *log = fopen("log.txt", "a");
    if (!log) return;

    time_t now = time(NULL);
    char time_str[30];
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", localtime(&now));

    vehicle *veh =  find_vehicle(license_plate);
    const char *vehicle_type = "Không rõ";
    if (veh != NULL) {
        vehicle_type = (veh->type == xe_may) ? "Xe_máy" : "Ô_tô";
    }

    if (strcmp(action, "out") == 0) {
        fprintf(log, "%s %s %s %d %s\n", license_plate, vehicle_type, action, fee, time_str);
    } else {
        fprintf(log, "%s %s %s %s\n", license_plate, vehicle_type, action, time_str);
    }
    fclose(log);
}

void save_parking_data_to_file() {
    FILE *f = fopen("parking_data.txt", "w");
    if (!f) {
        printf("Lỗi mở file để ghi!\n");
        return;
    }

    char time_str[30];
    for (int i = 0; i < num_vehicle; i++) {
        strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", 
                localtime(&vehicle_list[i].entry_time));
        
        const char *type_str = (vehicle_list[i].type == o_to) ? "O_TO" : "XE_MAY";
        
        fprintf(f, "%s %d %s %d %s\n",
               vehicle_list[i].license_plate,
               vehicle_list[i].fee,
               time_str,
               vehicle_list[i].floor,
               type_str);
    }
    fclose(f);
}

void update_daily_revenue(int so_tien) {
    FILE *file = fopen(revenue_FILE, "r+");
    if (!file) file = fopen(revenue_FILE, "w+");

    char date_str[20];
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    strftime(date_str, sizeof(date_str), "%Y-%m-%d", t);

    char line[100];
    int found = 0;
    FILE *temp = fopen("temp.txt", "w");

    while (fgets(line, sizeof(line), file)) {
        char date[20]; int tien;
        sscanf(line, "%[^:]: %d", date, &tien);
        if (strcmp(date, date_str) == 0) {
            fprintf(temp, "%s: %d\n", date, tien + so_tien);
            found = 1;
        } else {
            fputs(line, temp);
        }
    }

    if (!found) {
        fprintf(temp, "%s: %d\n", date_str, so_tien);
    }

    fclose(file);
    fclose(temp);
    remove(revenue_FILE);
    rename("temp.txt", revenue_FILE);
}

void summarize_revenue_by_day() {
    FILE *file = fopen(revenue_FILE, "r");
    if (!file) {
        printf("Chua co du lieu doanh thu.\n");
        return;
    }
    printf("\n== Lich su doanh thu theo ngay ==\n");
    char line[100];
    while (fgets(line, sizeof(line), file)) {
        printf("%s", line);
    }
    fclose(file);
}
void display_revenue_treeview(GtkTreeView *treeview) {
    GtkListStore *store;
    GtkTreeIter iter;
    store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_INT);

    FILE *file = fopen(revenue_FILE, "r");
    if (!file) return;

    char line[100], date[20];
    int value;
    while (fgets(line, sizeof(line), file)) {
        sscanf(line, "%[^:]: %d", date, &value);
        gtk_list_store_append(store, &iter);
        gtk_list_store_set(store, &iter, 0, date, 1, value, -1);
    }
    fclose(file);

    gtk_tree_view_set_model(treeview, GTK_TREE_MODEL(store));
    g_object_unref(store);

    GtkCellRenderer *renderer;
    GtkTreeViewColumn *column;

    column = gtk_tree_view_get_column(treeview, 0);
    if (!column) {
        renderer = gtk_cell_renderer_text_new();
        column = gtk_tree_view_column_new_with_attributes("Ngày", renderer, "text", 0, NULL);
        gtk_tree_view_append_column(treeview, column);
    }

    column = gtk_tree_view_get_column(treeview, 1);
    if (!column) {
        renderer = gtk_cell_renderer_text_new();
        column = gtk_tree_view_column_new_with_attributes("Doanh thu (VND)", renderer, "text", 1, NULL);
        gtk_tree_view_append_column(treeview, column);
    }
}

// Kiểm tra còn chỗ không
int check_empty_slot() {
    return num_vehicle < MAX_SLOTS;
}

// Hàm callback được gọi khi ứng dụng khởi động
void load_history_data(GtkListStore *store) {
    FILE *log = fopen("log.txt", "r");
    if (!log) return;

    int stt = 1;
    char license[20], vehicle_type[20], status[10];
    int fee;
    char time_str[50];

    while (!feof(log)) {
        int read = fscanf(log, "%s %s %s", license, vehicle_type, status);
        if (read != 3) break;

        GtkTreeIter iter;

        if (strcmp(status, "out") == 0) {
            fscanf(log, "%d %[^\n]", &fee, time_str);

            // Định dạng số tiền có dấu chấm phân cách
            char *formatted_fee = format_currency((double)fee);
            char fee_str[50];
            snprintf(fee_str, sizeof(fee_str), "%s VND", formatted_fee);

            gtk_list_store_append(store, &iter);
            gtk_list_store_set(store, &iter,
                0, stt++,
                1, license,
                2, vehicle_type,
                3, "Ra",
                4, time_str,
                5, fee_str,  // Sử dụng chuỗi đã định dạng
                -1
            );
            g_free(formatted_fee);  // Giải phóng bộ nhớ
        } else {
            fscanf(log, "%[^\n]", time_str);
            gtk_list_store_append(store, &iter);
            gtk_list_store_set(store, &iter,
                0, stt++,
                1, license,
                2, vehicle_type,
                3, "Vào",
                4, time_str,
                5, "-", // Không có phí lúc vào
                -1
            );
        }
    }
    fclose(log);
}
void update_history_data(SharedData *shared_data) {
    if (shared_data->history_store) {
        gtk_list_store_clear(shared_data->history_store);
        load_history_data(shared_data->history_store);
    }
}

void show_floor_statistics(SharedData *shared_data) {
    char *stats = floor_statistics();
    if (shared_data->home_stat_label) {
        gtk_label_set_text(GTK_LABEL(shared_data->home_stat_label), stats);
    }
}

// hàm cập nhật số lượng xe
void update_vehicle_count_label() {
    char count_str[50];
    sprintf(count_str, "Xe hiện tại trong bãi: %d / %d", num_vehicle, MAX_SLOTS * 4);
    gtk_label_set_text(GTK_LABEL(label_vehicle_count), count_str);
}



// Hàm kiểm tra cú pháp biển số
int check_license_plate(const char *a) {
   	if (strlen(a) == 10){// hàm check biển số xe ô tô theo định dạng XXA-XXX.XX
	    if (!isdigit(a[0]) || !isdigit(a[1])) return 0;
	    if (!isupper(a[2])) return 0;
	    if (a[3] != '-') return 0;
	    if (!isdigit(a[4]) || !isdigit(a[5]) || !isdigit(a[6])) return 0;
	    if (a[7] != '.') return 0;
	    if (!isdigit(a[8]) || !isdigit(a[9])) return 0;
	    return 1;
	}else if (strlen(a) == 12){ //check biển số xe máy theo định dạng XX-AX_XXX.XX
		if (!isdigit(a[0]) || !isdigit(a[1])) return 0;
		if (a[2] != '-') return 0;
	    if (!isupper(a[3])) return 0;
	    if (!isdigit(a[4])) return 0;
	    if (a[5] != '_') return 0;
	    if (!isdigit(a[6]) || !isdigit(a[7]) || !isdigit(a[8])) return 0;
	    if (a[9] != '.') return 0;
	    if (!isdigit(a[10]) || !isdigit(a[11])) return 0;
	    return 1;
	}else return 0 ;
}
//load dữ liệu từ file parking_data.txt
void load_vehicle_tree(SharedData *shared_data) {
    for (int i = 0; i < MAX_TANG; i++) {
        gtk_list_store_clear(shared_data->store_tangs[i]);
    }

    GtkTreeIter iter;
    for (int i = 0; i < num_vehicle; i++) {
        int f = vehicle_list[i].floor;
        if (f >= 1 && f <= MAX_TANG) {
            gtk_list_store_append(shared_data->store_tangs[f - 1], &iter);
            gtk_list_store_set(shared_data->store_tangs[f - 1], &iter, 0, vehicle_list[i].license_plate, -1);
        }
    }
}

bool license_plate_exists(const char *plate) {
    for (int i = 0; i < num_vehicle; i++) {
        if (strcmp(vehicle_list[i].license_plate, plate) == 0) {
            return true;
        }
    }
    return false;
}

vehicle create_new_vehicle(const char *plate, int floor, VehicleType type) {
    vehicle v;
    strncpy(v.license_plate, plate, sizeof(v.license_plate));
    v.floor =floor;
    v.type = type;
    v.entry_time = time(NULL);
    v.clock_start = clock();
    v.fee = 0;
    return v;
}

void show_errori(GtkWindow *parent, const char *msg) {
    GtkWidget *err = gtk_message_dialog_new(parent, GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR,
                                            GTK_BUTTONS_CLOSE, "%s", msg);
    gtk_dialog_run(GTK_DIALOG(err));
    gtk_widget_destroy(err);
}

vehicle*  find_vehicle(const char *license_plate) {
    for (int i = 0; i < num_vehicle; i++) {
        if (strcmp(vehicle_list[i].license_plate, license_plate) == 0) {
            return &vehicle_list[i];
        }
    }
    return NULL;
}

int find_vehicle_index(const char *plate) {
    for (int i = 0; i < num_vehicle; i++) {
        if (strcmp(vehicle_list[i].license_plate, plate) == 0) {
            return i;
        }
    }
    return -1;
}

int calculate_parking_fee(vehicle *veh) {
    clock_t clock_end = clock();
    double elapsed_seconds = (double)(clock_end - veh->clock_start) / CLOCKS_PER_SEC;
    int total_hours = (elapsed_seconds + 3599) / 3600;
    int rate = (veh->type == xe_may) ? XE_MAY : O_TO;
    return total_hours * rate;
}

void remove_vehicle_at_index(int index) {
    for (int j = index; j < num_vehicle - 1; j++) {
        vehicle_list[j] = vehicle_list[j + 1];
    }
    num_vehicle--;
}

void show_payment_result(GtkWindow *parent, const vehicle *veh, int fee) {
    double elapsed_seconds = (double)(clock() - veh->clock_start) / CLOCKS_PER_SEC;
    char *formatted_fee = g_strdup_printf("%'.0f", (double)fee); // Định dạng số tiền
    char msg[150];
    snprintf(msg, sizeof(msg),
             "Xe: %s\nThời gian gửi: %.1f giờ\nPhí: %s VND",
             veh->license_plate, elapsed_seconds / 3600, formatted_fee);

    GtkWidget *info_dialog = gtk_message_dialog_new(parent, GTK_DIALOG_MODAL,
                                                    GTK_MESSAGE_INFO, GTK_BUTTONS_OK, "%s", msg);
    gtk_dialog_run(GTK_DIALOG(info_dialog));
    gtk_widget_destroy(info_dialog);
    g_free(formatted_fee); // Giải phóng bộ nhớ
}

void calculate_total_revenue(double fee){
	revenue += fee;
	save_revenue();
}//gọi hàm calculate_total_revenue(fee) ở trong hàm void remove_vehicle(const char *license_plate)

char* floor_statistics(void) {
    static char buffer[100];
    int counts[MAX_TANG] = {0};
    
    for (int i = 0; i < num_vehicle; i++) {
        if (vehicle_list[i].floor >= 1 && vehicle_list[i].floor <= MAX_TANG) {
            counts[vehicle_list[i].floor - 1]++;
        }
    }
    
    snprintf(buffer, sizeof(buffer), 
             "Tầng 1: %d xe | Tầng 2: %d xe\nTầng 3: %d xe | Tầng 4: %d xe",
             counts[0], counts[1], counts[2], counts[3]);
    
    return buffer;
}

// Hàm thêm dấu chấm phân cách hàng ngàn
char* format_currency(double amount) {
    static char buffer[50];
    snprintf(buffer, sizeof(buffer), "%.0f", amount);

    int len = strlen(buffer);
    int pos = 0;
    char formatted[50] = {0};

    for (int i = 0; i < len; i++) {
        if ((len - i) % 3 == 0 && i != 0) {
            formatted[pos++] = '.';
        }
        formatted[pos++] = buffer[i];
    }
    formatted[pos] = '\0';
    return g_strdup(formatted);  // Trả về chuỗi đã được cấp phát động
}

void update_statistics_display() {
    int in_count = 0, out_count = 0;
    // Luôn đọc lại doanh thu từ file
    load_revenue();

    FILE *log = fopen("log.txt", "r");
    if (log) {
        char line[256];
        while (fgets(line, sizeof(line), log) != NULL) {  // Đã sửa dòng này
            if (strstr(line, " in ") != NULL) in_count++;
            else if (strstr(line, " out ") != NULL) out_count++;
        }
        fclose(log);
    }

    char stats[256];
    char *formatted_revenue = format_currency(revenue);
    snprintf(stats, sizeof(stats),
             "Xe vào: %d\nXe ra: %d\nDoanh thu: %s VND",
             in_count, out_count, formatted_revenue);
    
    gtk_label_set_text(GTK_LABEL(label_stats), stats);
    g_free(formatted_revenue); // Giải phóng bộ nhớ
}
void search_filter(SharedData *shared_data, const char *keyword) {
    for (int i = 0; i < MAX_TANG; i++) {
        gtk_list_store_clear(shared_data->store_tangs[i]);
    }

    GtkTreeIter iter;
    for (int i = 0; i < num_vehicle; i++) {
        if (strstr(vehicle_list[i].license_plate, keyword) != NULL) {
            int f = vehicle_list[i].floor;
            if (f >= 1 && f <= MAX_TANG) {
                gtk_list_store_append(shared_data->store_tangs[f - 1], &iter);
                gtk_list_store_set(shared_data->store_tangs[f - 1], &iter, 0, vehicle_list[i].license_plate, -1);
            }
        }
    }
}

void set_font_size(int size) {
    GtkCssProvider *provider = gtk_css_provider_new();
    char css[100];
    snprintf(css, sizeof(css), "* { font-size: %dpt; }", size); // Áp dụng cho tất cả widget

    gtk_css_provider_load_from_data(provider, css, -1, NULL);

    gtk_style_context_add_provider_for_screen(
        gdk_screen_get_default(),
        GTK_STYLE_PROVIDER(provider),
        GTK_STYLE_PROVIDER_PRIORITY_USER
    );

    g_object_unref(provider);
}

static void enter_license_plate(GtkWidget *widget, gpointer data) {
    SharedData *info = (SharedData *)data;
    GtkWindow *parent = info->parent_window;

    // Tạo hộp thoại nhập thông tin xe
    GtkWidget *dialog = gtk_dialog_new_with_buttons("Nhập thông tin xe", parent,
                                                    GTK_DIALOG_MODAL,
                                                    "_OK", GTK_RESPONSE_OK,
                                                    "_Hủy", GTK_RESPONSE_CANCEL, NULL);

    GtkWidget *content = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    GtkWidget *grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(grid), 5);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 5);

    // Các widget nhập liệu
    GtkWidget *label_plate = gtk_label_new("Biển số:");
    GtkWidget *entry_plate = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(entry_plate), "VD: 59A-123.45");

    GtkWidget *label_floor = gtk_label_new("Tầng:");
    GtkWidget *entry_floor = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(entry_floor), "1");

    GtkWidget *label_type = gtk_label_new("Loại xe:");
    GtkWidget *combo_type = gtk_combo_box_text_new();
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(combo_type), "Xe máy");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(combo_type), "Ô tô");
    gtk_combo_box_set_active(GTK_COMBO_BOX(combo_type), 0); // Mặc định chọn "Xe máy"

    // Sắp xếp các widget trên lưới
    gtk_grid_attach(GTK_GRID(grid), label_plate, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), entry_plate, 1, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), label_floor, 0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), entry_floor, 1, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), label_type, 0, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), combo_type, 1, 2, 1, 1);

    gtk_container_add(GTK_CONTAINER(content), grid);
    gtk_widget_show_all(dialog);

    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK) {
        const gchar *plate = gtk_entry_get_text(GTK_ENTRY(entry_plate));
        const gchar *floor_str = gtk_entry_get_text(GTK_ENTRY(entry_floor));
        int selected = gtk_combo_box_get_active(GTK_COMBO_BOX(combo_type)); // 0 = Xe máy, 1 = Ô tô

        // Kiểm tra định dạng biển số
        if (!check_license_plate(plate)) {
            show_errori(parent, "Biển số không hợp lệ!\n"
                                "Định dạng ô tô: XXA-XXX.XX\n"
                                "Định dạng xe máy: XX-AX_XXX.XX");
        }
        // Kiểm tra loại xe khớp với độ dài biển số
        else if ((selected == 0 && strlen(plate) != 12) || (selected == 1 && strlen(plate) != 10)) {
            show_errori(parent, "Loại xe không khớp với định dạng biển số!\nVui lòng chọn đúng loại xe.");
        }
        else {
            int floor = atoi(floor_str);
            if (floor < 1 || floor > MAX_TANG) {
                show_errori(parent, "Chỉ chấp nhận tầng từ 1 đến 4.");
            }
            else if (license_plate_exists(plate)) {
                show_errori(parent, "Biển số này đã tồn tại!");
            }
            else {
                // Tạo mới xe và lưu vào danh sách
                vehicle new_vehicle = create_new_vehicle(plate, floor, (selected == 0) ? xe_may : o_to);
                vehicle_list[num_vehicle++] = new_vehicle;

                // Ghi file và cập nhật giao diện
                save_parking_data_to_file();
                update_vehicle_count_label();
                gtk_label_set_text(GTK_LABEL(label_thongke), floor_statistics());

                // Thêm vào TreeView của tầng tương ứng
                GtkTreeIter iter;
                gtk_list_store_append(info->store_tangs[floor - 1], &iter);
                gtk_list_store_set(info->store_tangs[floor - 1], &iter, 0, plate, -1);

                // Ghi log và cập nhật thống kê
                write_log(plate, "in", 0);
                update_history_data(info);
                update_statistics_display();

                g_print("Xe %s đã thêm vào tầng %d\n", plate, floor);
            }
        }
    }

    // Hiển thị thống kê tầng sau khi thêm
    show_floor_statistics(info);
    gtk_widget_destroy(dialog);
}


// Hàm xử lý thay đổi thông tin xe khi người dùng chỉnh sửa trên TreeView
static void changes(GtkTreeView *tree_view, GtkTreePath *path, GtkTreeViewColumn *column, gpointer user_data) {
    SharedData *info = (SharedData *)user_data;
    GtkTreeModel *model = gtk_tree_view_get_model(tree_view);
    GtkTreeIter iter;

    if (gtk_tree_model_get_iter(model, &iter, path)) {
        // Lấy biển số xe từ dòng được chọn
        char *selected_plate;
        gtk_tree_model_get(model, &iter, 0, &selected_plate, -1);  // Cột 0 là biển số

        // Tạo hộp thoại để nhập thông tin mới
        GtkWidget *dialog = gtk_dialog_new_with_buttons(
            "Thay đổi thông tin xe",
            info->parent_window,
            GTK_DIALOG_MODAL,
            "_OK", GTK_RESPONSE_OK,
            "_Hủy", GTK_RESPONSE_CANCEL,
            NULL
        );

        GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));

        // Tạo các thành phần nhập dữ liệu
        GtkWidget *label_plate = gtk_label_new("Nhập biển số mới:");
        GtkWidget *entry_plate = gtk_entry_new();
        gtk_entry_set_text(GTK_ENTRY(entry_plate), selected_plate);

        GtkWidget *label_floor = gtk_label_new("Nhập tầng mới (1 đến 4):");
        GtkWidget *entry_floor = gtk_entry_new();

        GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
        gtk_box_pack_start(GTK_BOX(box), label_plate, FALSE, FALSE, 0);
        gtk_box_pack_start(GTK_BOX(box), entry_plate, FALSE, FALSE, 0);
        gtk_box_pack_start(GTK_BOX(box), label_floor, FALSE, FALSE, 0);
        gtk_box_pack_start(GTK_BOX(box), entry_floor, FALSE, FALSE, 0);
        gtk_container_add(GTK_CONTAINER(content_area), box);
        gtk_widget_show_all(dialog);

        if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK) {
            const gchar *new_plate = gtk_entry_get_text(GTK_ENTRY(entry_plate));
            const gchar *floor_text = gtk_entry_get_text(GTK_ENTRY(entry_floor));
            int new_floor = atoi(floor_text);

            // Kiểm tra tính hợp lệ của biển số
            if (!check_license_plate(new_plate)) {
                GtkWidget *err = gtk_message_dialog_new(
                    info->parent_window, GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR,
                    GTK_BUTTONS_CLOSE, "Biển số không hợp lệ!\n"
                    "Định dạng ô tô: XXA-XXX.XX\n"
                    "Định dạng xe máy: XX-AX-XXX.XX"
                );
                gtk_dialog_run(GTK_DIALOG(err));
                gtk_widget_destroy(err);
            }
            // Kiểm tra tầng có hợp lệ không
            else if (new_floor < 1 || new_floor > MAX_TANG) {
                GtkWidget *err = gtk_message_dialog_new(
                    info->parent_window, GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR,
                    GTK_BUTTONS_CLOSE, "Tầng phải từ 1 đến 4!"
                );
                gtk_dialog_run(GTK_DIALOG(err));
                gtk_widget_destroy(err);
            }
            else {
                // Kiểm tra biển số mới có trùng với xe khác không
                bool is_duplicate = false;
                for (int i = 0; i < num_vehicle; i++) {
                    if (strcmp(vehicle_list[i].license_plate, new_plate) == 0 &&
                        strcmp(vehicle_list[i].license_plate, selected_plate) != 0) {
                        is_duplicate = true;
                        break;
                    }
                }

                if (is_duplicate) {
                    GtkWidget *err = gtk_message_dialog_new(
                        info->parent_window, GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR,
                        GTK_BUTTONS_CLOSE, "Biển số đã tồn tại!"
                    );
                    gtk_dialog_run(GTK_DIALOG(err));
                    gtk_widget_destroy(err);
                }
                else {
                    // Tìm và cập nhật dữ liệu xe tương ứng
                    for (int i = 0; i < num_vehicle; i++) {
                        if (strcmp(vehicle_list[i].license_plate, selected_plate) == 0) {
                            strncpy(vehicle_list[i].license_plate, new_plate, sizeof(vehicle_list[i].license_plate) - 1);
                            vehicle_list[i].license_plate[sizeof(vehicle_list[i].license_plate) - 1] = '\0';
                            vehicle_list[i].floor = new_floor;
                            break;
                        }
                    }

                    // Lưu thay đổi và cập nhật giao diện
                    save_parking_data_to_file();
                    load_vehicle_tree(info);

                    GtkWidget *info_dialog = gtk_message_dialog_new(
                        info->parent_window, GTK_DIALOG_MODAL,
                        GTK_MESSAGE_INFO, GTK_BUTTONS_OK,
                        "Đã thay đổi thông tin xe thành công!"
                    );
                    gtk_dialog_run(GTK_DIALOG(info_dialog));
                    gtk_widget_destroy(info_dialog);
                }
            }
        }

        // Dọn dẹp
        gtk_widget_destroy(dialog);
        g_free(selected_plate);
    }
}

// Hàm xử lý khi người dùng nhấn nút "Xóa và Thanh toán"
static void pay_and_remove(GtkWidget *widget, gpointer data) {
    // Lấy dữ liệu dùng chung từ tham số truyền vào
    SharedData *info = (SharedData *)data;
    GtkWindow *parent = info->parent_window;

    // Tạo hộp thoại nhập biển số xe cần thanh toán
    GtkWidget *dialog = gtk_dialog_new_with_buttons(
        "Thanh toán & Xóa xe",
        parent,
        GTK_DIALOG_MODAL,
        "_OK", GTK_RESPONSE_OK,
        "_Hủy", GTK_RESPONSE_CANCEL,
        NULL
    );

    // Lấy vùng nội dung của dialog
    GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));

    // Tạo nhãn và ô nhập biển số
    GtkWidget *label = gtk_label_new("Nhập biển số xe:");
    GtkWidget *entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(entry), "VD: 59A-123.45");

    // Tạo một box dọc để chứa nhãn và ô nhập
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_box_pack_start(GTK_BOX(box), label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(box), entry, FALSE, FALSE, 0);
    gtk_container_add(GTK_CONTAINER(content_area), box);
    gtk_widget_show_all(dialog);

    // Khi người dùng nhấn "OK"
    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK) {
        const gchar *plate = gtk_entry_get_text(GTK_ENTRY(entry));

        // Tìm vị trí của xe trong danh sách
        int vitri = find_vehicle_index(plate);

        if (vitri == -1) {
            // Nếu không tìm thấy, hiện thông báo lỗi
            GtkWidget *err = gtk_message_dialog_new(
                parent, GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR,
                GTK_BUTTONS_CLOSE, "Không tìm thấy xe!"
            );
            gtk_dialog_run(GTK_DIALOG(err));
            gtk_widget_destroy(err);
        } else {
            // Nếu tìm thấy, tiến hành xử lý thanh toán
            vehicle *veh = &vehicle_list[vitri];

            // Tính phí giữ xe
            int fee = calculate_parking_fee(veh);
            veh->fee = fee;

            // Cập nhật doanh thu
            calculate_total_revenue(fee);
            update_daily_revenue(fee);
            display_revenue_treeview(GTK_TREE_VIEW(treeview_revenue));

            // Ghi nhật ký thanh toán
            write_log(plate, "out", fee);

            // Hiển thị kết quả thanh toán cho người dùng
            show_payment_result(parent, veh, fee);

            // Xóa xe khỏi danh sách đang gửi
            remove_vehicle_at_index(vitri);

            // Lưu dữ liệu bãi xe sau khi cập nhật
            save_parking_data_to_file();

            // Cập nhật TreeView trong giao diện
            load_vehicle_tree(info);

            // Cập nhật tổng số lượng xe
            update_vehicle_count_label();

            // Cập nhật thống kê theo tầng
            gtk_label_set_text(GTK_LABEL(label_thongke), floor_statistics());
            show_floor_statistics(info);

            // Cập nhật tab lịch sử giữ xe
            update_history_data(info);

            // Cập nhật thống kê tổng hợp
            update_statistics_display();
        }
    }

    // Hủy dialog sau khi hoàn tất
    gtk_widget_destroy(dialog);
}

// Callback khi người dùng thay đổi nội dung trong ô tìm kiếm
static void search(GtkEditable *entry, gpointer user_data) {
    // Ép kiểu dữ liệu con trỏ người dùng sang kiểu SharedData
    SharedData *shared_data = (SharedData *)user_data;

    // Lấy nội dung văn bản hiện tại trong ô tìm kiếm
    const gchar *text = gtk_entry_get_text(GTK_ENTRY(entry));

    // Gọi hàm lọc tìm kiếm, truyền dữ liệu dùng chung và từ khóa
    search_filter(shared_data, text);
}

static void activate(GtkApplication *app, gpointer user_data) {
    GtkWidget *window;
    GtkWidget *containerBox;

    // Thiết lập cỡ chữ mặc định cho toàn bộ giao diện
    set_font_size(16); 

    // Tạo cửa sổ chính
    window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "Quản lý bãi giữ xe");
    gtk_window_set_default_size(GTK_WINDOW(window), 1920, 1080);

    // Hộp chứa chính theo chiều dọc
    containerBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);

    // === TẠO 2 NÚT GÓC TRÊN PHẢI: Thêm và Xóa/Thanh toán ===
    GtkWidget *top_right_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    GtkWidget *btn1 = gtk_button_new_with_label("Thêm");
    GtkWidget *btn2 = gtk_button_new_with_label("Xóa và Thanh toán");
    gtk_box_pack_start(GTK_BOX(top_right_box), btn1, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(top_right_box), btn2, FALSE, FALSE, 0);
    gtk_widget_set_halign(top_right_box, GTK_ALIGN_END);
    gtk_box_pack_start(GTK_BOX(containerBox), top_right_box, FALSE, FALSE, 5);

    // === TẠO NOTEBOOK CHỨA CÁC TAB ===
    GtkWidget *notebook = gtk_notebook_new();

    // === TAB 1: TRANG CHỦ ===
    GtkWidget *tab_label1 = gtk_label_new("Trang chủ");
    GtkWidget *home_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    GtkWidget *label_welcome = gtk_label_new("Chào mừng đến với hệ thống quản lý bãi giữ xe");
    GtkWidget *label_fee = gtk_label_new("Phí giữ xe: 5.000 VND (ô tô) || 2.000 VND (xe máy)");
    GtkWidget *label_note = gtk_label_new("Phí giữ xe được tính theo giờ. Nếu bạn gửi xe chưa đủ 1 giờ thì vẫn tính tròn là 1 giờ.");
    label_vehicle_count = gtk_label_new(NULL);
    update_vehicle_count_label();  // Cập nhật số lượng xe đang gửi

    // Gán thống kê theo tầng vào label
    label_thongke = gtk_label_new((const gchar*)floor_statistics());
    shared_data.home_stat_label = label_thongke;

    // Thêm vào giao diện Trang chủ
    gtk_box_pack_start(GTK_BOX(home_box), label_welcome, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(home_box), label_fee, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(home_box), label_note, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(home_box), label_vehicle_count, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(home_box), label_thongke, FALSE, FALSE, 10);
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), home_box, tab_label1);

    // === TAB 2: THỐNG KÊ ===
    GtkWidget *tab_label2 = gtk_label_new("Thống kê");
    GtkWidget *tab_content2 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    label_stats = gtk_label_new("Đang tải thống kê...");
    gtk_box_pack_start(GTK_BOX(tab_content2), label_stats, FALSE, FALSE, 10);
    update_statistics_display();  // Hàm cập nhật dữ liệu thống kê
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), tab_content2, tab_label2);

    // === TAB 3: BÃI XE ===
    GtkWidget *tab_label3 = gtk_label_new("Bãi xe");
    GtkWidget *bai_xe_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);

    // Ô tìm kiếm biển số
    GtkWidget *search_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(search_entry), "Tìm biển số xe...");
    gtk_box_pack_start(GTK_BOX(bai_xe_vbox), search_entry, FALSE, FALSE, 0);

    // Tạo notebook con chứa các tầng
    GtkWidget *nested_notebook = gtk_notebook_new();
    GtkListStore *store_tangs[MAX_TANG];
    GtkWidget *treeviews[MAX_TANG];

    for (int i = 0; i < MAX_TANG; i++) {
        char tab_name[20];
        snprintf(tab_name, sizeof(tab_name), "Tầng %d", i + 1);
        GtkWidget *tab_label = gtk_label_new(tab_name);

        // Tạo store và treeview cho từng tầng
        store_tangs[i] = gtk_list_store_new(1, G_TYPE_STRING);
        GtkWidget *treeview = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store_tangs[i]));
        treeviews[i] = treeview;

        GtkCellRenderer *renderer = gtk_cell_renderer_text_new();
        GtkTreeViewColumn *column = gtk_tree_view_column_new_with_attributes("Biển số xe", renderer, "text", 0, NULL);
        gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);

        GtkWidget *scroll = gtk_scrolled_window_new(NULL, NULL);
        gtk_container_add(GTK_CONTAINER(scroll), treeview);
        gtk_widget_set_vexpand(scroll, TRUE);
        gtk_widget_set_hexpand(scroll, TRUE);

        gtk_notebook_append_page(GTK_NOTEBOOK(nested_notebook), scroll, tab_label);
    }

    // Gắn treeview từng tầng vào notebook
    gtk_box_pack_start(GTK_BOX(bai_xe_vbox), nested_notebook, TRUE, TRUE, 0);
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), bai_xe_vbox, tab_label3);

    // Lưu trữ store tầng vào shared_data
    for (int i = 0; i < MAX_TANG; i++) {
        shared_data.store_tangs[i] = store_tangs[i];
    }
    shared_data.parent_window = GTK_WINDOW(window);

    // Gắn callback tìm kiếm
    g_signal_connect(search_entry, "changed", G_CALLBACK(search), &shared_data);

    // Gắn callback khi nhấn 2 lần vào xe trong treeview
    for (int i = 0; i < MAX_TANG; i++) {
        g_signal_connect(treeviews[i], "row-activated", G_CALLBACK(changes), &shared_data);
    }

    // === TAB 4: DOANH THU ===
    GtkWidget *tab_revenue = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    treeview_revenue = gtk_tree_view_new();
    gtk_box_pack_start(GTK_BOX(tab_revenue), treeview_revenue, TRUE, TRUE, 0);
    display_revenue_treeview(GTK_TREE_VIEW(treeview_revenue));
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), tab_revenue, gtk_label_new("Doanh thu"));

    // === TAB 5: LỊCH SỬ GIỮ XE ===
    GtkWidget *scrolled_history = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_history), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

    GtkWidget *history_view = gtk_tree_view_new();
    gtk_container_add(GTK_CONTAINER(scrolled_history), history_view);

    GtkListStore *store = gtk_list_store_new(6, G_TYPE_INT, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);
    gtk_tree_view_set_model(GTK_TREE_VIEW(history_view), GTK_TREE_MODEL(store));
    g_object_unref(store);
    shared_data.history_store = store;
    load_history_data(store);

    // Thêm cột vào TreeView lịch sử
    const char *titles[] = {"STT", "Biển số xe", "Loại xe", "Trạng thái", "Thời gian", "Chi phí"};
    for (int i = 0; i < 6; i++) {
        GtkCellRenderer *renderer = gtk_cell_renderer_text_new();
        GtkTreeViewColumn *column = gtk_tree_view_column_new_with_attributes(titles[i], renderer, "text", i, NULL);
        gtk_tree_view_append_column(GTK_TREE_VIEW(history_view), column);
    }

    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), scrolled_history, gtk_label_new("Lịch sử giữ xe"));

    // Đọc dữ liệu từ file
    load_revenue();
    read_from_file();
    update_vehicle_count_label();
    gtk_label_set_text(GTK_LABEL(label_thongke), floor_statistics());
    load_vehicle_tree(&shared_data);

    // Gắn callback cho nút Thêm và Thanh toán
    g_signal_connect(btn1, "clicked", G_CALLBACK(enter_license_plate), &shared_data);
    g_signal_connect(btn2, "clicked", G_CALLBACK(pay_and_remove), &shared_data);

    // Thêm notebook vào cửa sổ chính và hiển thị
    gtk_box_pack_start(GTK_BOX(containerBox), notebook, TRUE, TRUE, 5);
    gtk_container_add(GTK_CONTAINER(window), containerBox);
    gtk_widget_show_all(window);
}


int main(int argc, char **argv) {
    GtkApplication *app;
    int status;
	app = gtk_application_new("hello.world", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
    status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);
    return status;   
}
