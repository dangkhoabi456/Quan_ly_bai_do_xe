#include "parking.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <glib.h>

SharedData shared_data;
vehicle vehicle_list[MAX_SLOTS];
int num_vehicle = 0;
double revenue = 0;
GtkWidget *label_stats;
GtkWidget *treeview_revenue;
GtkWidget *label_vehicle_count;
GtkWidget *label_thongke;

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
    double value;
    while (fgets(line, sizeof(line), file)) {
        sscanf(line, "%[^:]: %lf", date, &value);
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

 void enter_license_plate(GtkWidget *widget, gpointer data) {
	GtkWidget *label_floor = gtk_label_new("Tầng:");
	
    SharedData *info = (SharedData *)data;

    GtkWindow *parent = info->parent_window;

    GtkWidget *dialog = gtk_dialog_new_with_buttons("Nhập thông tin xe", parent,
                                                    GTK_DIALOG_MODAL,
                                                    "_OK", GTK_RESPONSE_OK,
                                                    "_Hủy", GTK_RESPONSE_CANCEL, NULL);

    GtkWidget *content = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    GtkWidget *grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(grid), 5);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 5);

    GtkWidget *label_plate = gtk_label_new("Biển số:");
    GtkWidget *entry_plate = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(entry_plate), "VD: 59A-123.45");

    GtkWidget *entry_floor = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(entry_floor), "1");

    GtkWidget *label_type = gtk_label_new("Loại xe:");
    GtkWidget *combo_type = gtk_combo_box_text_new();
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(combo_type), "Xe máy");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(combo_type), "Ô tô");
    gtk_combo_box_set_active(GTK_COMBO_BOX(combo_type), 0);

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
        int selected = gtk_combo_box_get_active(GTK_COMBO_BOX(combo_type));

        if (!check_license_plate(plate)) {
		    show_errori(parent, "Biển số không hợp lệ!\n"
		                       "Định dạng ô tô: XXA-XXX.XX\n"
		                       "Định dạng xe máy: XX-AX_XXX.XX");
		} else if ((selected == 0 && strlen(plate) != 12) || (selected == 1 && strlen(plate) != 10)) {
		    show_errori(parent, "Loại xe không khớp với định dạng biển số!\n"
		                       "Vui lòng chọn đúng loại xe.");
		} else {
		    int floor = atoi(floor_str);
		    if (floor < 1 ||floor > MAX_TANG) {
		        show_errori(parent, "Chỉ chấp nhận tầng từ 1 đến 4.");
		    } else if (license_plate_exists(plate)) {
		        show_errori(parent, "Biển số này đã tồn tại!");
		    } else {
		        vehicle new_vehicle = create_new_vehicle(plate,floor, (selected == 0) ? xe_may : o_to);
		        vehicle_list[num_vehicle++] = new_vehicle;
		
		        save_parking_data_to_file
            ();
		        update_vehicle_count_label();
		        gtk_label_set_text(GTK_LABEL(label_thongke), floor_statistics());
		
		        GtkTreeIter iter;
		        gtk_list_store_append(info->store_tangs[floor - 1], &iter);
		        gtk_list_store_set(info->store_tangs[floor - 1], &iter, 0, plate, -1);
		
		        write_log(plate, "in", 0);
		        update_history_data(info);
		        update_statistics_display();
		
		        g_print("Xe %s đã thêm vào tầng %d\n", plate,floor);
		 	}
		}
    }
	show_floor_statistics
(info);  // info chính là shared_data được truyền vào  // truyền địa chỉ &shared_data (con trỏ)
    gtk_widget_destroy(dialog);
}

 void changes(GtkTreeView *tree_view, GtkTreePath *path, GtkTreeViewColumn *column, gpointer user_data) {
    SharedData *info = (SharedData *)user_data;
    GtkTreeModel *model = gtk_tree_view_get_model(tree_view);
    GtkTreeIter iter;

    if (gtk_tree_model_get_iter(model, &iter, path)) {
        char *selected_plate;
        gtk_tree_model_get(model, &iter, 0, &selected_plate, -1);  // Cột 0 là biển số xe

        GtkWidget *dialog = gtk_dialog_new_with_buttons("Thay đổi thông tin xe",
                                                        info->parent_window,
                                                        GTK_DIALOG_MODAL,
                                                        "_OK", GTK_RESPONSE_OK,
                                                        "_Hủy", GTK_RESPONSE_CANCEL, NULL);
        GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));

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

            if (!check_license_plate(new_plate)) {
                GtkWidget *err = gtk_message_dialog_new(info->parent_window, GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR,
                                                        GTK_BUTTONS_CLOSE, "Biển số không hợp lệ!\nĐịnh dạng: XXA-XXX.XX(oto)\nĐịnh dạng: XX-AX-XXX.XX(xe máy)");
                gtk_dialog_run(GTK_DIALOG(err));
                gtk_widget_destroy(err);
            } else if (new_floor < 1 || new_floor > MAX_TANG) {
                GtkWidget *err = gtk_message_dialog_new(info->parent_window, GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR,
                                                        GTK_BUTTONS_CLOSE, "Tầng phải từ 1 đến 4!");
                gtk_dialog_run(GTK_DIALOG(err));
                gtk_widget_destroy(err);
            } else {
                bool is_duplicate = false;
                for (int i = 0; i < num_vehicle; i++) {
                    if (strcmp(vehicle_list[i].license_plate, new_plate) == 0 &&
                        strcmp(vehicle_list[i].license_plate, selected_plate) != 0) {
                        is_duplicate = true;
                        break;
                    }
                }

                if (is_duplicate) {
                    GtkWidget *err = gtk_message_dialog_new(info->parent_window, GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR,
                                                            GTK_BUTTONS_CLOSE, "Biển số đã tồn tại!");
                    gtk_dialog_run(GTK_DIALOG(err));
                    gtk_widget_destroy(err);
                } else {
                    // Cập nhật dữ liệu
                    for (int i = 0; i < num_vehicle; i++) {
                        if (strcmp(vehicle_list[i].license_plate, selected_plate) == 0) {
                            strncpy(vehicle_list[i].license_plate, new_plate, sizeof(vehicle_list[i].license_plate));
                            vehicle_list[i].floor = new_floor;
                            break;
                        }
                    }

                    save_parking_data_to_file
                ();
                    load_vehicle_tree(info);

                    GtkWidget *info_dialog = gtk_message_dialog_new(info->parent_window, GTK_DIALOG_MODAL,
                                                                    GTK_MESSAGE_INFO, GTK_BUTTONS_OK,
                                                                    "Đã thay đổi thông tin xe thành công!");
                    gtk_dialog_run(GTK_DIALOG(info_dialog));
                    gtk_widget_destroy(info_dialog);
                }
            }
        }

        gtk_widget_destroy(dialog);
        g_free(selected_plate);
    }
}
 void  pay_and_remove(GtkWidget *widget, gpointer data) {
    SharedData *info = (SharedData*)data;
    GtkWindow *parent = info->parent_window;

    GtkWidget *dialog = gtk_dialog_new_with_buttons("Thanh toán & Xóa xe",
                                                    parent, GTK_DIALOG_MODAL,
                                                    "_OK", GTK_RESPONSE_OK,
                                                    "_Hủy", GTK_RESPONSE_CANCEL, NULL);

    GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    GtkWidget *label = gtk_label_new("Nhập biển số xe:");
    GtkWidget *entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(entry), "VD: 59A-123.45");

    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_box_pack_start(GTK_BOX(box), label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(box), entry, FALSE, FALSE, 0);
    gtk_container_add(GTK_CONTAINER(content_area), box);
    gtk_widget_show_all(dialog);

    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK) {
        const gchar *plate = gtk_entry_get_text(GTK_ENTRY(entry));
        int vitri = find_vehicle_index
    (plate);

        if (vitri == -1) {
            GtkWidget *err = gtk_message_dialog_new(parent, GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR,
                                                    GTK_BUTTONS_CLOSE, "Không tìm thấy xe!");
            gtk_dialog_run(GTK_DIALOG(err));
            gtk_widget_destroy(err);
        } else {
            vehicle *veh = &vehicle_list[vitri];
            int fee = calculate_parking_fee(veh);
            veh->fee = fee;
            calculate_total_revenue(fee);
            update_daily_revenue(veh->fee);
            display_revenue_treeview(GTK_TREE_VIEW(treeview_revenue));
            write_log(plate, "out", fee);
            show_payment_result(parent, veh, fee);

            remove_vehicle_at_index(vitri);
            save_parking_data_to_file
        ();
            load_vehicle_tree(info); // Cập nhật TreeView
          	update_vehicle_count_label();
          	gtk_label_set_text(GTK_LABEL(label_thongke), floor_statistics());
            update_history_data(info);
            update_statistics_display();
			show_floor_statistics
        (info);
            save_parking_data_to_file
        ();
            load_vehicle_tree(info); // Cập nhật TreeView
        }
    }
	
    gtk_widget_destroy(dialog);
    
}
 void search(GtkEditable *entry, gpointer user_data) {
    SharedData *shared_data = (SharedData *)user_data;
    const gchar *text = gtk_entry_get_text(GTK_ENTRY(entry));
    search_filter(shared_data, text);
}