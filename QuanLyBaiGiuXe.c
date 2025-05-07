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
#define DOANH_THU_FILE "doanh_thu_theo_ngay.txt" 
#ifndef DOANHTHU_H
#define DOANHTHU_H

void cap_nhat_doanh_thu_ngay(int so_tien);
void thong_ke_doanh_thu_theo_ngay();
int tinh_tong_doanh_thu_thang(int thang, int nam);
int tinh_tong_doanh_thu_nam(int nam);
void hien_thi_doanh_thu_treeview(GtkTreeView *treeview);  

#endif

typedef enum {
    xe_may,
    o_to
} VehicleType;
typedef struct {
    char bien_so_xe[20];   
    int phi;                  
    time_t thoi_gian_vao;        
    clock_t bo_dem;
    int so_tang;
	VehicleType type; 
} vehicle;
typedef struct {
    GtkListStore *store_tangs[MAX_TANG];
    GtkListStore *history_store;  // Thêm dòng này
    GtkWindow *parent_window;
    GtkWidget *home_stat_label;
} SharedData;
SharedData shared_data;   // biến kiểu struct	

vehicle danh_sach_xe[MAX_SLOTS];
void tai_du_lieu_lich_su(GtkListStore *store);
int so_luong_xe = 0;
double doanh_thu = 0;  
void cap_nhat_tab_lich_su(SharedData *shared_data);
GtkWidget *label_stats; 
GtkWidget *treeview_doanh_thu;
GtkWidget *label_vehicle_count; //đếm xe trong bãi	
void tinh_tong_doanh_thu(double phi);  
void tai_cay_xe(SharedData *shared_data);
int con_slot_trong();
char* dinh_dang_tien(double amount);
int tim_vi_tri_xe(const char *plate);
int tinh_phi_gui_xe(vehicle *veh);
void xoa_xe_tai_vi_tri(int index);
void hien_thi_ket_qua_thanh_toan(GtkWindow *parent, const vehicle *veh, int phi);
static void thanh_toan_va_xoa(GtkWidget *widget, gpointer data);
static void thay_doi(GtkTreeView *tree_view, GtkTreePath *path, GtkTreeViewColumn *column, gpointer user_data);
bool bien_so_da_ton_tai(const char *plate);
vehicle tao_xe_moi(const char *plate, int so_tang, VehicleType type);
void hien_thi_loi(GtkWindow *parent, const char *msg);
static void nhap_bien_so(GtkWidget *widget, gpointer data);
int kiem_tra_bien_so(const char *a);  // Khai báo prototype
void doc_du_lieu_tu_file(SharedData *shared_data);
vehicle* tim_xe(const char *bien_so_xe);
void cap_nhat_hien_thi_thong_ke();
static void tim_kiem(GtkEditable *entry, gpointer user_data);
void luu_doanh_thu();
void tai_doanh_thu();
void luu_du_lieu_file_parking();
void bo_loc_tim_kiem(SharedData *shared_data, const char *keyword);
char* thong_ke_theo_tang(void);
void ghi_nhat_ky(const char *bien_so_xe, const char *action, int phi);
GtkWidget *label_thongke;
void cap_nhat_nhan_so_luong_xe();
void hien_thi_thong_ke_tang(SharedData *shared_data);
void dat_kich_thuoc(int size);
// Hàm lưu doanh thu
void luu_doanh_thu() {
    FILE *f = fopen("doanh_thu.txt", "w");
    if (f) {
        fprintf(f, "%.0f", doanh_thu);
        fclose(f);
    }
}

// Hàm đọc doanh thu
void tai_doanh_thu() {
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
    so_luong_xe = 0;

    while (fscanf(pt, "%s %d %d-%d-%d %d:%d:%d %d %s",
                  temp.bien_so_xe, &temp.phi,
                  &year, &mon, &day, &hour, &min, &sec, &temp.so_tang, type_str) == 10) {

        struct tm tm_time = {0};
        tm_time.tm_year = year - 1900;
        tm_time.tm_mon = mon - 1;
        tm_time.tm_mday = day;
        tm_time.tm_hour = hour;
        tm_time.tm_min = min;
        tm_time.tm_sec = sec;

        temp.thoi_gian_vao = mktime(&tm_time);
        temp.bo_dem = clock() - (clock_t)(difftime(time(NULL), temp.thoi_gian_vao) * CLOCKS_PER_SEC);

        // Phân loại xe
        if (strcmp(type_str, "O_TO") == 0) {
            temp.type = o_to;
        } else {
            temp.type = xe_may;
        }

        if (so_luong_xe < MAX_SLOTS) {
            danh_sach_xe[so_luong_xe++] = temp;
        }
    }
    fclose(pt);
}

void ghi_nhat_ky(const char *bien_so_xe, const char *action, int phi) {
    FILE *log = fopen("log.txt", "a");
    if (!log) return;

    time_t now = time(NULL);
    char time_str[30];
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", localtime(&now));

    vehicle *veh = tim_xe(bien_so_xe);
    const char *vehicle_type = "Không rõ";
    if (veh != NULL) {
        vehicle_type = (veh->type == xe_may) ? "Xe_máy" : "Ô_tô";
    }

    if (strcmp(action, "out") == 0) {
        fprintf(log, "%s %s %s %d %s\n", bien_so_xe, vehicle_type, action, phi, time_str);
    } else {
        fprintf(log, "%s %s %s %s\n", bien_so_xe, vehicle_type, action, time_str);
    }
    fclose(log);
}

void luu_du_lieu_file_parking() {
    FILE *f = fopen("parking_data.txt", "w");
    if (!f) {
        printf("Lỗi mở file để ghi!\n");
        return;
    }

    char time_str[30];
    for (int i = 0; i < so_luong_xe; i++) {
        strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", 
                localtime(&danh_sach_xe[i].thoi_gian_vao));
        
        const char *type_str = (danh_sach_xe[i].type == o_to) ? "O_TO" : "XE_MAY";
        
        fprintf(f, "%s %d %s %d %s\n",
               danh_sach_xe[i].bien_so_xe,
               danh_sach_xe[i].phi,
               time_str,
               danh_sach_xe[i].so_tang,
               type_str);
    }
    fclose(f);
}

void cap_nhat_doanh_thu_ngay(int so_tien) {
    FILE *file = fopen(DOANH_THU_FILE, "r+");
    if (!file) file = fopen(DOANH_THU_FILE, "w+");

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
    remove(DOANH_THU_FILE);
    rename("temp.txt", DOANH_THU_FILE);
}

void thong_ke_doanh_thu_theo_ngay() {
    FILE *file = fopen(DOANH_THU_FILE, "r");
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
void hien_thi_doanh_thu_treeview(GtkTreeView *treeview) {
    GtkListStore *store;
    GtkTreeIter iter;
    store = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_INT);

    FILE *file = fopen(DOANH_THU_FILE, "r");
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
int con_slot_trong() {
    return so_luong_xe < MAX_SLOTS;
}

// Hàm callback được gọi khi ứng dụng khởi động
void tai_du_lieu_lich_su(GtkListStore *store) {
    FILE *log = fopen("log.txt", "r");
    if (!log) return;

    int stt = 1;
    char license[20], vehicle_type[20], status[10];
    int phi;
    char time_str[50];

    while (!feof(log)) {
        int read = fscanf(log, "%s %s %s", license, vehicle_type, status);
        if (read != 3) break;

        GtkTreeIter iter;

        if (strcmp(status, "out") == 0) {
            fscanf(log, "%d %[^\n]", &phi, time_str);

            // Định dạng số tiền có dấu chấm phân cách
            char *formatted_phi = dinh_dang_tien((double)phi);
            char phi_str[50];
            snprintf(phi_str, sizeof(phi_str), "%s VND", formatted_phi);

            gtk_list_store_append(store, &iter);
            gtk_list_store_set(store, &iter,
                0, stt++,
                1, license,
                2, vehicle_type,
                3, "Ra",
                4, time_str,
                5, phi_str,  // Sử dụng chuỗi đã định dạng
                -1
            );
            g_free(formatted_phi);  // Giải phóng bộ nhớ
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
void cap_nhat_tab_lich_su(SharedData *shared_data) {
    if (shared_data->history_store) {
        gtk_list_store_clear(shared_data->history_store);
        tai_du_lieu_lich_su(shared_data->history_store);
    }
}

void hien_thi_thong_ke_tang(SharedData *shared_data) {
    char *stats = thong_ke_theo_tang();
    if (shared_data->home_stat_label) {
        gtk_label_set_text(GTK_LABEL(shared_data->home_stat_label), stats);
    }
}

// hàm cập nhật số lượng xe
void cap_nhat_nhan_so_luong_xe() {
    char count_str[50];
    sprintf(count_str, "Xe hiện tại trong bãi: %d / %d", so_luong_xe, MAX_SLOTS * 4);
    gtk_label_set_text(GTK_LABEL(label_vehicle_count), count_str);
}



// Hàm kiểm tra cú pháp biển số
int kiem_tra_bien_so(const char *a) {
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
void tai_cay_xe(SharedData *shared_data) {
    for (int i = 0; i < MAX_TANG; i++) {
        gtk_list_store_clear(shared_data->store_tangs[i]);
    }

    GtkTreeIter iter;
    for (int i = 0; i < so_luong_xe; i++) {
        int f = danh_sach_xe[i].so_tang;
        if (f >= 1 && f <= MAX_TANG) {
            gtk_list_store_append(shared_data->store_tangs[f - 1], &iter);
            gtk_list_store_set(shared_data->store_tangs[f - 1], &iter, 0, danh_sach_xe[i].bien_so_xe, -1);
        }
    }
}

bool bien_so_da_ton_tai(const char *plate) {
    for (int i = 0; i < so_luong_xe; i++) {
        if (strcmp(danh_sach_xe[i].bien_so_xe, plate) == 0) {
            return true;
        }
    }
    return false;
}

vehicle tao_xe_moi(const char *plate, int so_tang, VehicleType type) {
    vehicle v;
    strncpy(v.bien_so_xe, plate, sizeof(v.bien_so_xe));
    v.so_tang = so_tang;
    v.type = type;
    v.thoi_gian_vao = time(NULL);
    v.bo_dem = clock();
    v.phi = 0;
    return v;
}

void hien_thi_loi(GtkWindow *parent, const char *msg) {
    GtkWidget *err = gtk_message_dialog_new(parent, GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR,
                                            GTK_BUTTONS_CLOSE, "%s", msg);
    gtk_dialog_run(GTK_DIALOG(err));
    gtk_widget_destroy(err);
}

vehicle* tim_xe(const char *bien_so_xe) {
    for (int i = 0; i < so_luong_xe; i++) {
        if (strcmp(danh_sach_xe[i].bien_so_xe, bien_so_xe) == 0) {
            return &danh_sach_xe[i];
        }
    }
    return NULL;
}

int tim_vi_tri_xe(const char *plate) {
    for (int i = 0; i < so_luong_xe; i++) {
        if (strcmp(danh_sach_xe[i].bien_so_xe, plate) == 0) {
            return i;
        }
    }
    return -1;
}

int tinh_phi_gui_xe(vehicle *veh) {
    clock_t clock_end = clock();
    double elapsed_seconds = (double)(clock_end - veh->bo_dem) / CLOCKS_PER_SEC;
    int total_hours = (elapsed_seconds + 3599) / 3600;
    int rate = (veh->type == xe_may) ? XE_MAY : O_TO;
    return total_hours * rate;
}

void xoa_xe_tai_vi_tri(int index) {
    for (int j = index; j < so_luong_xe - 1; j++) {
        danh_sach_xe[j] = danh_sach_xe[j + 1];
    }
    so_luong_xe--;
}

void hien_thi_ket_qua_thanh_toan(GtkWindow *parent, const vehicle *veh, int phi) {
    double elapsed_seconds = (double)(clock() - veh->bo_dem) / CLOCKS_PER_SEC;
    char *formatted_phi = g_strdup_printf("%'.0f", (double)phi); // Định dạng số tiền
    char msg[150];
    snprintf(msg, sizeof(msg),
             "Xe: %s\nThời gian gửi: %.1f giờ\nPhí: %s VND",
             veh->bien_so_xe, elapsed_seconds / 3600, formatted_phi);

    GtkWidget *info_dialog = gtk_message_dialog_new(parent, GTK_DIALOG_MODAL,
                                                    GTK_MESSAGE_INFO, GTK_BUTTONS_OK, "%s", msg);
    gtk_dialog_run(GTK_DIALOG(info_dialog));
    gtk_widget_destroy(info_dialog);
    g_free(formatted_phi); // Giải phóng bộ nhớ
}

void tinh_tong_doanh_thu(double phi){
	doanh_thu += phi;
	luu_doanh_thu();
}//gọi hàm tinh_tong_doanh_thu(phi) ở trong hàm void remove_vehicle(const char *bien_so_xe)

char* thong_ke_theo_tang(void) {
    static char buffer[100];
    int counts[MAX_TANG] = {0};
    
    for (int i = 0; i < so_luong_xe; i++) {
        if (danh_sach_xe[i].so_tang >= 1 && danh_sach_xe[i].so_tang <= MAX_TANG) {
            counts[danh_sach_xe[i].so_tang - 1]++;
        }
    }
    
    snprintf(buffer, sizeof(buffer), 
             "Tầng 1: %d xe | Tầng 2: %d xe\nTầng 3: %d xe | Tầng 4: %d xe",
             counts[0], counts[1], counts[2], counts[3]);
    
    return buffer;
}

// Hàm thêm dấu chấm phân cách hàng ngàn
char* dinh_dang_tien(double amount) {
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

void cap_nhat_hien_thi_thong_ke() {
    int in_count = 0, out_count = 0;
    // Luôn đọc lại doanh thu từ file
    tai_doanh_thu();

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
    char *formatted_doanh_thu = dinh_dang_tien(doanh_thu);
    snprintf(stats, sizeof(stats),
             "Xe vào: %d\nXe ra: %d\nDoanh thu: %s VND",
             in_count, out_count, formatted_doanh_thu);
    
    gtk_label_set_text(GTK_LABEL(label_stats), stats);
    g_free(formatted_doanh_thu); // Giải phóng bộ nhớ
}
void bo_loc_tim_kiem(SharedData *shared_data, const char *keyword) {
    for (int i = 0; i < MAX_TANG; i++) {
        gtk_list_store_clear(shared_data->store_tangs[i]);
    }

    GtkTreeIter iter;
    for (int i = 0; i < so_luong_xe; i++) {
        if (strstr(danh_sach_xe[i].bien_so_xe, keyword) != NULL) {
            int f = danh_sach_xe[i].so_tang;
            if (f >= 1 && f <= MAX_TANG) {
                gtk_list_store_append(shared_data->store_tangs[f - 1], &iter);
                gtk_list_store_set(shared_data->store_tangs[f - 1], &iter, 0, danh_sach_xe[i].bien_so_xe, -1);
            }
        }
    }
}

void dat_kich_thuoc(int size) {
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

static void nhap_bien_so(GtkWidget *widget, gpointer data) {
	GtkWidget *label_so_tang = gtk_label_new("Tầng:");
	
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

    GtkWidget *entry_so_tang = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(entry_so_tang), "1");

    GtkWidget *label_type = gtk_label_new("Loại xe:");
    GtkWidget *combo_type = gtk_combo_box_text_new();
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(combo_type), "Xe máy");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(combo_type), "Ô tô");
    gtk_combo_box_set_active(GTK_COMBO_BOX(combo_type), 0);

    gtk_grid_attach(GTK_GRID(grid), label_plate, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), entry_plate, 1, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), label_so_tang, 0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), entry_so_tang, 1, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), label_type, 0, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), combo_type, 1, 2, 1, 1);

    gtk_container_add(GTK_CONTAINER(content), grid);
    gtk_widget_show_all(dialog);

    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK) {
        const gchar *plate = gtk_entry_get_text(GTK_ENTRY(entry_plate));
        const gchar *so_tang_str = gtk_entry_get_text(GTK_ENTRY(entry_so_tang));
        int selected = gtk_combo_box_get_active(GTK_COMBO_BOX(combo_type));

        if (!kiem_tra_bien_so(plate)) {
		    hien_thi_loi(parent, "Biển số không hợp lệ!\n"
		                       "Định dạng ô tô: XXA-XXX.XX\n"
		                       "Định dạng xe máy: XX-AX_XXX.XX");
		} else if ((selected == 0 && strlen(plate) != 12) || (selected == 1 && strlen(plate) != 10)) {
		    hien_thi_loi(parent, "Loại xe không khớp với định dạng biển số!\n"
		                       "Vui lòng chọn đúng loại xe.");
		} else {
		    int so_tang = atoi(so_tang_str);
		    if (so_tang < 1 || so_tang > MAX_TANG) {
		        hien_thi_loi(parent, "Chỉ chấp nhận tầng từ 1 đến 4.");
		    } else if (bien_so_da_ton_tai(plate)) {
		        hien_thi_loi(parent, "Biển số này đã tồn tại!");
		    } else {
		        vehicle new_vehicle = tao_xe_moi(plate, so_tang, (selected == 0) ? xe_may : o_to);
		        danh_sach_xe[so_luong_xe++] = new_vehicle;
		
		        luu_du_lieu_file_parking
            ();
		        cap_nhat_nhan_so_luong_xe();
		        gtk_label_set_text(GTK_LABEL(label_thongke), thong_ke_theo_tang());
		
		        GtkTreeIter iter;
		        gtk_list_store_append(info->store_tangs[so_tang - 1], &iter);
		        gtk_list_store_set(info->store_tangs[so_tang - 1], &iter, 0, plate, -1);
		
		        ghi_nhat_ky(plate, "in", 0);
		        cap_nhat_tab_lich_su(info);
		        cap_nhat_hien_thi_thong_ke();
		
		        g_print("Xe %s đã thêm vào tầng %d\n", plate, so_tang);
		 	}
		}
    }
	hien_thi_thong_ke_tang
(info);  // info chính là shared_data được truyền vào  // truyền địa chỉ &shared_data (con trỏ)
    gtk_widget_destroy(dialog);
}

static void thay_doi(GtkTreeView *tree_view, GtkTreePath *path, GtkTreeViewColumn *column, gpointer user_data) {
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

        GtkWidget *label_so_tang = gtk_label_new("Nhập tầng mới (1 đến 4):");
        GtkWidget *entry_so_tang = gtk_entry_new();

        GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
        gtk_box_pack_start(GTK_BOX(box), label_plate, FALSE, FALSE, 0);
        gtk_box_pack_start(GTK_BOX(box), entry_plate, FALSE, FALSE, 0);
        gtk_box_pack_start(GTK_BOX(box), label_so_tang, FALSE, FALSE, 0);
        gtk_box_pack_start(GTK_BOX(box), entry_so_tang, FALSE, FALSE, 0);
        gtk_container_add(GTK_CONTAINER(content_area), box);
        gtk_widget_show_all(dialog);

        if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK) {
            const gchar *new_plate = gtk_entry_get_text(GTK_ENTRY(entry_plate));
            const gchar *so_tang_text = gtk_entry_get_text(GTK_ENTRY(entry_so_tang));
            int new_so_tang = atoi(so_tang_text);

            if (!kiem_tra_bien_so(new_plate)) {
                GtkWidget *err = gtk_message_dialog_new(info->parent_window, GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR,
                                                        GTK_BUTTONS_CLOSE, "Biển số không hợp lệ!\nĐịnh dạng: XXA-XXX.XX(oto)\nĐịnh dạng: XX-AX-XXX.XX(xe máy)");
                gtk_dialog_run(GTK_DIALOG(err));
                gtk_widget_destroy(err);
            } else if (new_so_tang < 1 || new_so_tang > MAX_TANG) {
                GtkWidget *err = gtk_message_dialog_new(info->parent_window, GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR,
                                                        GTK_BUTTONS_CLOSE, "Tầng phải từ 1 đến 4!");
                gtk_dialog_run(GTK_DIALOG(err));
                gtk_widget_destroy(err);
            } else {
                bool is_duplicate = false;
                for (int i = 0; i < so_luong_xe; i++) {
                    if (strcmp(danh_sach_xe[i].bien_so_xe, new_plate) == 0 &&
                        strcmp(danh_sach_xe[i].bien_so_xe, selected_plate) != 0) {
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
                    for (int i = 0; i < so_luong_xe; i++) {
                        if (strcmp(danh_sach_xe[i].bien_so_xe, selected_plate) == 0) {
                            strncpy(danh_sach_xe[i].bien_so_xe, new_plate, sizeof(danh_sach_xe[i].bien_so_xe));
                            danh_sach_xe[i].so_tang = new_so_tang;
                            break;
                        }
                    }

                    luu_du_lieu_file_parking
                ();
                    tai_cay_xe(info);

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

static void thanh_toan_va_xoa(GtkWidget *widget, gpointer data) {
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
        int vitri = tim_vi_tri_xe
    (plate);

        if (vitri == -1) {
            GtkWidget *err = gtk_message_dialog_new(parent, GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR,
                                                    GTK_BUTTONS_CLOSE, "Không tìm thấy xe!");
            gtk_dialog_run(GTK_DIALOG(err));
            gtk_widget_destroy(err);
        } else {
            vehicle *veh = &danh_sach_xe[vitri];
            int phi = tinh_phi_gui_xe(veh);
            veh->phi = phi;
            tinh_tong_doanh_thu(phi);
            cap_nhat_doanh_thu_ngay(veh->phi);
            hien_thi_doanh_thu_treeview(GTK_TREE_VIEW(treeview_doanh_thu));
            ghi_nhat_ky(plate, "out", phi);
            hien_thi_ket_qua_thanh_toan(parent, veh, phi);

            xoa_xe_tai_vi_tri(vitri);
            luu_du_lieu_file_parking
        ();
            tai_cay_xe(info); // Cập nhật TreeView
          	cap_nhat_nhan_so_luong_xe();
          	gtk_label_set_text(GTK_LABEL(label_thongke), thong_ke_theo_tang());
            cap_nhat_tab_lich_su(info);
            cap_nhat_hien_thi_thong_ke();
			hien_thi_thong_ke_tang
        (info);
            luu_du_lieu_file_parking
        ();
            tai_cay_xe(info); // Cập nhật TreeView
        }
    }
	
    gtk_widget_destroy(dialog);
    
}

static void tim_kiem(GtkEditable *entry, gpointer user_data) {
    SharedData *shared_data = (SharedData *)user_data;
    const gchar *text = gtk_entry_get_text(GTK_ENTRY(entry));
    bo_loc_tim_kiem(shared_data, text);
}

static void activate(GtkApplication *app, gpointer user_data) {
    GtkWidget *window;
    GtkWidget *button;
    GtkWidget *button_box;
    GtkWidget *label;
    GtkWidget *containerBox;
    //thống kê theo tầng
    label_thongke = gtk_label_new((const gchar*)thong_ke_theo_tang());
    // Tạo cửa sổ chính
    window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "Quản lý bãi giữ xe");
    gtk_window_set_default_size(GTK_WINDOW(window), 1920, 1080);

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
    GtkWidget *label_phi = gtk_label_new("Phí giữ xe: 5.000 VND (ô tô) || 2.000 VND (xe máy)");

    GtkWidget *label_note = gtk_label_new("Phí giữ xe được tính theo giờ.Nếu bạn gửi xe chưa đủ 1 giờ thì vẫn tính tròn là 1 giờ.");

    label_vehicle_count = gtk_label_new(NULL);
	  cap_nhat_nhan_so_luong_xe();
    gtk_box_pack_start(GTK_BOX(home_box), label_welcome, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(home_box), label_phi, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(home_box), label_note, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(home_box), label_vehicle_count, FALSE, FALSE, 0);
    GtkWidget *label_thongke = gtk_label_new((const gchar*)thong_ke_theo_tang());
	shared_data.home_stat_label = label_thongke;    // nếu dùng biến struct
	gtk_box_pack_start(GTK_BOX(home_box), label_thongke, FALSE, FALSE, 10);
    // Gắn box vào tab Trang chủ
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), home_box, tab_label1);

    // === TAB 2: Thống kê ===
    GtkWidget *tab_label2 = gtk_label_new("Thống kê");
GtkWidget *tab_content2 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);

label_stats = gtk_label_new("Đang tải thống kê...");
gtk_box_pack_start(GTK_BOX(tab_content2), label_stats, FALSE, FALSE, 10);

cap_nhat_hien_thi_thong_ke();

    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), tab_content2, tab_label2);
    
    // === TAB 3: Bãi xe ===
GtkWidget *tab_label3 = gtk_label_new("Bãi xe");

	//=== TAB 4: Doanh thu ===
GtkWidget *tab_doanh_thu = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
treeview_doanh_thu = gtk_tree_view_new();

gtk_box_pack_start(GTK_BOX(tab_doanh_thu), treeview_doanh_thu, TRUE, TRUE, 0);
hien_thi_doanh_thu_treeview(GTK_TREE_VIEW(treeview_doanh_thu));

gtk_notebook_append_page(GTK_NOTEBOOK(notebook), tab_doanh_thu, gtk_label_new("Doanh thu"));
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


for (int i = 0; i < MAX_TANG; i++) {
   shared_data.store_tangs[i] = store_tangs[i];
}
shared_data.parent_window = GTK_WINDOW(window);

// Nếu cần truyền cửa sổ chính
shared_data.parent_window = GTK_WINDOW(window);
  // Chỉ nếu bạn dùng trong hàm xử lý
g_signal_connect(search_entry, "changed", G_CALLBACK(tim_kiem), &shared_data);

for (int i = 0; i < MAX_TANG; i++) {
   g_signal_connect(treeviews[i], "row-activated", G_CALLBACK(thay_doi), &shared_data);
}
tai_doanh_thu();
read_from_file();
cap_nhat_nhan_so_luong_xe();
gtk_label_set_text(GTK_LABEL(label_thongke), thong_ke_theo_tang());
tai_cay_xe(&shared_data);

// Kết nối nút với callback và truyền shared_data
g_signal_connect(btn1, "clicked", G_CALLBACK(nhap_bien_so), &shared_data);
g_signal_connect(btn2, "clicked", G_CALLBACK(thanh_toan_va_xoa), &shared_data);    
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
shared_data.history_store = store;
  // Lưu store vào shared_data
gtk_tree_view_set_model(GTK_TREE_VIEW(history_view), GTK_TREE_MODEL(store));
g_object_unref(store);
tai_du_lieu_lich_su(store);

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