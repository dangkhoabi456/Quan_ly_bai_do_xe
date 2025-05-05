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
int num_vehicles = 0;
double doanh_thu = 0;  
void refresh_history_tab(SharedData *shared_data);
GtkWidget *label_stats; 
GtkWidget *label_vehicle_count; //đếm xe trong bãi	
void Cal_total(double fee);  
void load_treeviews(SharedData *shared_data);
int has_available_slot();
int TimViTriXe(const char *plate);
int TinhPhiGuiXe(vehicle *veh);
void XoaXeTaiViTri(int index);
void HienThiKetQuaThanhToan(GtkWindow *parent, const vehicle *veh, int fee);
static void ThanhtoanvaXoa(GtkWidget *widget, gpointer data);
static void Thaydoi(GtkTreeView *tree_view, GtkTreePath *path, GtkTreeViewColumn *column, gpointer user_data);
bool BienSoDaTonTai(const char *plate);
vehicle TaoXeMoi(const char *plate, int floor, VehicleType type);
void HienThiLoi(GtkWindow *parent, const char *msg);
static void onNhapBienSoXe(GtkWidget *widget, gpointer data);
int Check__license_plate(const char *a);  // Khai báo prototype
void read_form_file(SharedData *shared_data);
vehicle* find_vehicle(const char *license_plate);
void update_statistics_display();
static void on_search_changed(GtkEditable *entry, gpointer user_data);
void save_doanh_thu();
void load_doanh_thu();
char* thong_ke_theo_tang(void);
void log_action(const char *license_plate, const char *action, int fee);

// hàm cập nhật số lượng xe
void update_vehicle_count_label() {
    char count_str[50];
    sprintf(count_str, "Xe hiện tại trong bãi: %d / %d", num_vehicles, MAX_SLOTS * 4);
    gtk_label_set_text(GTK_LABEL(label_vehicle_count), count_str);
}


// Hàm lưu doanh thu
void save_doanh_thu() {
    FILE *f = fopen("doanh_thu.txt", "w");
    if (f) {
        fprintf(f, "%.0f", doanh_thu);
        fclose(f);
    }
}

// Hàm đọc doanh thu
void load_doanh_thu() {
    FILE *f = fopen("doanh_thu.txt", "r");
    if (f) {
        fscanf(f, "%lf", &doanh_thu);
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
    num_vehicles = 0;

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

        if (num_vehicles < MAX_SLOTS) {
            vehicle_list[num_vehicles++] = temp;
        }
    }
    fclose(pt);
}

void log_action(const char *license_plate, const char *action, int fee) {
    FILE *log = fopen("log.txt", "a");
    if (!log) return;

    time_t now = time(NULL);
    char time_str[30];
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", localtime(&now));

    vehicle *veh = find_vehicle(license_plate);
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

void save_parking_data() {
    FILE *f = fopen("parking_data.txt", "w");
    if (!f) {
        printf("Lỗi mở file để ghi!\n");
        return;
    }

    char time_str[30];
    for (int i = 0; i < num_vehicles; i++) {
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

// Kiểm tra còn chỗ không
int has_available_slot() {
    return num_vehicles < MAX_SLOTS;
}

// Hàm callback được gọi khi ứng dụng khởi động
static void activate(GtkApplication *app, gpointer user_data) {
    GtkWidget *window;
    GtkWidget *button;
    GtkWidget *button_box;
    GtkWidget *label;
    GtkWidget *containerBox;
    // Tạo cửa sổ chính
    window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "Quản lý bãi giữ xe");
    gtk_window_set_default_size(GTK_WINDOW(window), 500, 500);

    // Tạo hộp chứa theo chiều dọc
    containerBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);

    // ==== TẠO VÀ THÊM 3 NÚT GÓC TRÊN PHẢI ====
    GtkWidget *top_right_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);

    GtkWidget *btn1 = gtk_button_new_with_label("Thêm");
    GtkWidget *btn2 = gtk_button_new_with_label("Xóa và Thanh toán");

    gtk_box_pack_start(GTK_BOX(top_right_box), btn1, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(top_right_box), btn2, FALSE, FALSE, 0);
 
    gtk_widget_set_halign(top_right_box, GTK_ALIGN_END);
    gtk_box_pack_start(GTK_BOX(containerBox), top_right_box, FALSE, FALSE, 5);

       // ==== TẠO NOTEBOOK (CÁC TAB) ====
    GtkWidget *notebook = gtk_notebook_new();

    // === TAB 1: Trang chủ ===
    GtkWidget *tab_label1 = gtk_label_new("Trang chủ");
    GtkWidget *home_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    GtkWidget *label_welcome = gtk_label_new("Chào mừng đến với hệ thống quản lý bãi giữ xe");
    GtkWidget *label_fee = gtk_label_new("Phí giữ xe: 5.000 VND (ô tô) || 2.000 VND (xe máy)");

    GtkWidget *label_note = gtk_label_new("Phí giữ xe được tính theo giờ.Nếu bạn gửi xe chưa đủ 1 giờ thì vẫn tính tròn là 1 giờ.");
    GtkWidget *label_thongke = gtk_label_new((const gchar*)thong_ke_theo_tang());
	  shared_data.home_stat_label = label_thongke;   // nếu dùng biến struct
	  gtk_box_pack_start(GTK_BOX(home_box), label_thongke, FALSE, FALSE, 10);
    label_vehicle_count = gtk_label_new(NULL);
	  update_vehicle_count_label();
    gtk_box_pack_start(GTK_BOX(home_box), label_welcome, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(home_box), label_fee, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(home_box), label_note, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(home_box), label_vehicle_count, FALSE, FALSE, 0);
    // Gắn box vào tab Trang chủ
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), home_box, tab_label1);

    // === TAB 2: Thống kê ===
    GtkWidget *tab_label2 = gtk_label_new("Thống kê");
GtkWidget *tab_content2 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);

label_stats = gtk_label_new("Đang tải thống kê...");
gtk_box_pack_start(GTK_BOX(tab_content2), label_stats, FALSE, FALSE, 10);

update_statistics_display();

    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), tab_content2, tab_label2);

    // === TAB 3: Bãi xe ===
GtkWidget *tab_label3 = gtk_label_new("Bãi xe");

// Tạo notebook con để chứa các tầng
GtkWidget *nested_notebook = gtk_notebook_new();
GtkWidget *bai_xe_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);

// Thanh tìm kiếm
GtkWidget *search_entry = gtk_entry_new();
gtk_entry_set_placeholder_text(GTK_ENTRY(search_entry), "Tìm biển số xe...");
gtk_box_pack_start(GTK_BOX(bai_xe_vbox), search_entry, FALSE, FALSE, 0);


GtkListStore *store_tangs[MAX_TANG];
GtkWidget *treeviews[MAX_TANG];

for (int i = 0; i < MAX_TANG; i++) {
    char tab_name[20];
    snprintf(tab_name, sizeof(tab_name), "Tầng %d", i + 1);
    GtkWidget *tab_label = gtk_label_new(tab_name);

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

// Cuối cùng: Thêm notebook con vào tab chính "Bãi xe"
gtk_box_pack_start(GTK_BOX(bai_xe_vbox), nested_notebook, TRUE, TRUE, 0);
gtk_notebook_append_page(GTK_NOTEBOOK(notebook), bai_xe_vbox, tab_label3);


SharedData *shared_data = g_new(SharedData, 1);
for (int i = 0; i < MAX_TANG; i++) {
    shared_data->store_tangs[i] = store_tangs[i];
}
shared_data->parent_window = GTK_WINDOW(window);

// Nếu cần truyền cửa sổ chính
shared_data->parent_window = GTK_WINDOW(window);  // Chỉ nếu bạn dùng trong hàm xử lý
g_signal_connect(search_entry, "changed", G_CALLBACK(on_search_changed), shared_data);
for (int i = 0; i < MAX_TANG; i++) {
    g_signal_connect(treeviews[i], "row-activated", G_CALLBACK(Thaydoi), shared_data);
}

g_signal_connect(btn2, "clicked", G_CALLBACK(ThanhtoanvaXoa), shared_data);

load_doanh_thu();
read_from_file();
update_vehicle_count_label();
load_treeviews(shared_data);

// Kết nối nút với callback và truyền shared_data
g_signal_connect(btn1, "clicked", G_CALLBACK(onNhapBienSoXe), shared_data);
    
// Thêm notebook vào container chính
gtk_box_pack_start(GTK_BOX(containerBox), notebook, TRUE, TRUE, 5);

// Tạo scrolled window chứa bảng lịch sử
GtkWidget *scrolled_history = gtk_scrolled_window_new(NULL, NULL);
gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_history), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

// Tạo TreeView
GtkWidget *history_view = gtk_tree_view_new();
gtk_container_add(GTK_CONTAINER(scrolled_history), history_view);

// Tạo model cho TreeView
GtkListStore *store = gtk_list_store_new(6,
    G_TYPE_INT,     // STT
    G_TYPE_STRING,  // Biển số xe
    G_TYPE_STRING,  // Loại xe
    G_TYPE_STRING,  // Trạng thái (Vào/Ra)
    G_TYPE_STRING,  // Thời gian
    G_TYPE_STRING   // Chi phí giữ xe
);
shared_data->history_store = store;  // Lưu store vào shared_data
gtk_tree_view_set_model(GTK_TREE_VIEW(history_view), GTK_TREE_MODEL(store));
g_object_unref(store);
load_history_data(store);


// Thêm các cột
const char *titles[] = {"STT", "Biển số xe", "Loại xe", "Trạng thái", "Thời gian", "Chi phí"};
for (int i = 0; i < 6; i++) {
    GtkCellRenderer *renderer = gtk_cell_renderer_text_new();
    GtkTreeViewColumn *column = gtk_tree_view_column_new_with_attributes(titles[i], renderer, "text", i, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(history_view), column);
}


// Thêm tab vào notebook
gtk_notebook_append_page(GTK_NOTEBOOK(notebook), scrolled_history, gtk_label_new("Lịch sử giữ xe"));

// Thêm vào cửa sổ & hiển thị
gtk_container_add(GTK_CONTAINER(window), containerBox);
gtk_widget_show_all(window);
    // Đọc dữ liệu và load vào TreeView khi app mở lên

}


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

            char fee_str[20];
            snprintf(fee_str, sizeof(fee_str), "%d VND", fee);

            gtk_list_store_append(store, &iter);
            gtk_list_store_set(store, &iter,
                0, stt++,
                1, license,
                2, vehicle_type,
                3, "Ra",
                4, time_str,
                5, fee_str,
                -1
            );
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

void refresh_history_tab(SharedData *shared_data) {
    if (shared_data->history_store) {
        gtk_list_store_clear(shared_data->history_store);
        load_history_data(shared_data->history_store);
    }
}

void show_floor_statistics(SharedData *shared_data) {
    char *stats = thong_ke_theo_tang();
    if (shared_data->home_stat_label) {
        gtk_label_set_text(GTK_LABEL(shared_data->home_stat_label), stats);
    }
}

// Hàm kiểm tra cú pháp biển số
int Check__license_plate(const char *a) {
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
void load_treeviews(SharedData *shared_data) {
    for (int i = 0; i < MAX_TANG; i++) {
        gtk_list_store_clear(shared_data->store_tangs[i]);
    }

    GtkTreeIter iter;
    for (int i = 0; i < num_vehicles; i++) {
        int f = vehicle_list[i].floor;
        if (f >= 1 && f <= MAX_TANG) {
            gtk_list_store_append(shared_data->store_tangs[f - 1], &iter);
            gtk_list_store_set(shared_data->store_tangs[f - 1], &iter, 0, vehicle_list[i].license_plate, -1);
        }
    }
}

bool BienSoDaTonTai(const char *plate) {
    for (int i = 0; i < num_vehicles; i++) {
        if (strcmp(vehicle_list[i].license_plate, plate) == 0) {
            return true;
        }
    }
    return false;
}

vehicle TaoXeMoi(const char *plate, int floor, VehicleType type) {
    vehicle v;
    strncpy(v.license_plate, plate, sizeof(v.license_plate));
    v.floor = floor;
    v.type = type;
    v.entry_time = time(NULL);
    v.clock_start = clock();
    v.fee = 0;
    return v;
}

void HienThiLoi(GtkWindow *parent, const char *msg) {
    GtkWidget *err = gtk_message_dialog_new(parent, GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR,
                                            GTK_BUTTONS_CLOSE, "%s", msg);
    gtk_dialog_run(GTK_DIALOG(err));
    gtk_widget_destroy(err);
}

static void onNhapBienSoXe(GtkWidget *widget, gpointer data) {
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

        if (!Check__license_plate(plate)) {
            HienThiLoi(parent, "Biển số không hợp lệ!\n"
                               "Định dạng: XXA-XXX.XX (ô tô)\n"
                               "Định dạng: XX-AX_XXX.XX (xe máy)");
        } else {
            int floor = atoi(floor_str);
            if (floor < 1 || floor > MAX_TANG) {
                HienThiLoi(parent, "Chỉ chấp nhận tầng từ 1 đến 4.");
            } else if (BienSoDaTonTai(plate)) {
                HienThiLoi(parent, "Biển số này đã tồn tại!");
            } else {
                vehicle new_vehicle = TaoXeMoi(plate, floor, (selected == 0) ? xe_may : o_to);
                vehicle_list[num_vehicles++] = new_vehicle;

                save_parking_data();
                update_vehicle_count_label();
        

                GtkTreeIter iter;
                gtk_list_store_append(info->store_tangs[floor - 1], &iter);
                gtk_list_store_set(info->store_tangs[floor - 1], &iter, 0, plate, -1);


                log_action(plate, "in", 0);
                refresh_history_tab(info);
                update_statistics_display();

                g_print("Xe %s đã thêm vào tầng %d\n", plate, floor);
            }
        }
    }
    gtk_widget_destroy(dialog);
    show_floor_statistics(&shared_data);  // truyền địa chỉ &shared_data (con trỏ)
}

vehicle* find_vehicle(const char *license_plate) {
    for (int i = 0; i < num_vehicles; i++) {
        if (strcmp(vehicle_list[i].license_plate, license_plate) == 0) {
            return &vehicle_list[i];
        }
    }
    return NULL;
}
void Cal_total(double fee){
	doanh_thu += fee;
	save_doanh_thu();
}//gọi hàm Cal_total(fee) ở trong hàm void remove_vehicle(const char *license_plate)

static void Thaydoi(GtkTreeView *tree_view, GtkTreePath *path, GtkTreeViewColumn *column, gpointer user_data) {
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

            if (!Check__license_plate(new_plate)) {
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
                for (int i = 0; i < num_vehicles; i++) {
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
                    for (int i = 0; i < num_vehicles; i++) {
                        if (strcmp(vehicle_list[i].license_plate, selected_plate) == 0) {
                            strncpy(vehicle_list[i].license_plate, new_plate, sizeof(vehicle_list[i].license_plate));
                            vehicle_list[i].floor = new_floor;
                            break;
                        }
                    }

                    save_parking_data();
                    load_treeviews(info);

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


int TimViTriXe(const char *plate) {
    for (int i = 0; i < num_vehicles; i++) {
        if (strcmp(vehicle_list[i].license_plate, plate) == 0) {
            return i;
        }
    }
    return -1;
}

int TinhPhiGuiXe(vehicle *veh) {
    clock_t clock_end = clock();
    double elapsed_seconds = (double)(clock_end - veh->clock_start) / CLOCKS_PER_SEC;
    int total_hours = (elapsed_seconds + 3599) / 3600;
    int rate = (veh->type == xe_may) ? XE_MAY : O_TO;
    return total_hours * rate;
}

void XoaXeTaiViTri(int index) {
    for (int j = index; j < num_vehicles - 1; j++) {
        vehicle_list[j] = vehicle_list[j + 1];
    }
    num_vehicles--;
}

void HienThiKetQuaThanhToan(GtkWindow *parent, const vehicle *veh, int fee) {
    double elapsed_seconds = (double)(clock() - veh->clock_start) / CLOCKS_PER_SEC;
    char msg[150];
    snprintf(msg, sizeof(msg),
             "Xe: %s\nThời gian gửi: %.1f giờ\nPhí: %d VND",
             veh->license_plate, elapsed_seconds / 3600, fee);

    GtkWidget *info_dialog = gtk_message_dialog_new(parent, GTK_DIALOG_MODAL,
                                                    GTK_MESSAGE_INFO, GTK_BUTTONS_OK, "%s", msg);
    gtk_dialog_run(GTK_DIALOG(info_dialog));
    gtk_widget_destroy(info_dialog);
}

static void ThanhtoanvaXoa(GtkWidget *widget, gpointer data) {
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
        int vitri = TimViTriXe(plate);

        if (vitri == -1) {
            GtkWidget *err = gtk_message_dialog_new(parent, GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR,
                                                    GTK_BUTTONS_CLOSE, "Không tìm thấy xe!");
            gtk_dialog_run(GTK_DIALOG(err));
            gtk_widget_destroy(err);
        } else {
            vehicle *veh = &vehicle_list[vitri];
            int fee = TinhPhiGuiXe(veh);
            veh->fee = fee;
            Cal_total(fee);
            log_action(plate, "out", fee);
            HienThiKetQuaThanhToan(parent, veh, fee);

            XoaXeTaiViTri(vitri);
            save_parking_data();
            load_treeviews(info); // Cập nhật TreeView
          	update_vehicle_count_label();
            refresh_history_tab(info);
            update_statistics_display();
            save_parking_data();
            load_treeviews(info); // Cập nhật TreeView
        }
    }

    gtk_widget_destroy(dialog);
    show_floor_statistics(info);
}

void update_statistics_display() {
    int in_count = 0, out_count = 0;
    // Luôn đọc lại doanh thu từ file
    load_doanh_thu();

    FILE *log = fopen("log.txt", "r");
    if (log) {
        char line[256];
        while (fgets(line, sizeof(line), log)) {
            if (strstr(line, " in ") != NULL) in_count++;
            else if (strstr(line, " out ") != NULL) out_count++;
        }
        fclose(log);
    }

    char stats[256];
    snprintf(stats, sizeof(stats),
             "Xe vào: %d\nXe ra: %d\nDoanh thu: %.0f VND",
             in_count, out_count, doanh_thu);

    gtk_label_set_text(GTK_LABEL(label_stats), stats);
}

void filter_treeviews(SharedData *shared_data, const char *keyword) {
    for (int i = 0; i < MAX_TANG; i++) {
        gtk_list_store_clear(shared_data->store_tangs[i]);
    }

    GtkTreeIter iter;
    for (int i = 0; i < num_vehicles; i++) {
        if (strstr(vehicle_list[i].license_plate, keyword) != NULL) {
            int f = vehicle_list[i].floor;
            if (f >= 1 && f <= MAX_TANG) {
                gtk_list_store_append(shared_data->store_tangs[f - 1], &iter);
                gtk_list_store_set(shared_data->store_tangs[f - 1], &iter, 0, vehicle_list[i].license_plate, -1);
            }
        }
    }
}

static void on_search_changed(GtkEditable *entry, gpointer user_data) {
    SharedData *shared_data = (SharedData *)user_data;
    const gchar *text = gtk_entry_get_text(GTK_ENTRY(entry));
    filter_treeviews(shared_data, text);
}

char* thong_ke_theo_tang(void) {
    static char buffer[100];
    int counts[MAX_TANG] = {0};
    
    for (int i = 0; i < num_vehicles; i++) {
        if (vehicle_list[i].floor >= 1 && vehicle_list[i].floor <= MAX_TANG) {
            counts[vehicle_list[i].floor - 1]++;
        }
    }
    
    snprintf(buffer, sizeof(buffer), 
             "Tầng 1: %d xe | Tầng 2: %d xe\nTầng 3: %d xe | Tầng 4: %d xe",
             counts[0], counts[1], counts[2], counts[3]);
    
    return buffer;
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