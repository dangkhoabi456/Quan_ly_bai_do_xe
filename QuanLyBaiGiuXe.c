#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <stdbool.h>

#define don_gia_theo_gio 5000  // đơn giá gửi xe mỗi giờ
#define so_luong_cho 50

// Cấu trúc xe
typedef struct {
    char bien_so_xe[20];
    int phi;
    time_t thoi_gian_vao;
    clock_t bat_dau_dem;
    int tang;
} phuong_tien;

// Dữ liệu chia sẻ giữa callback
typedef struct {
    GtkListStore *store_tang1;
    GtkListStore *store_tang2;
    GtkWindow *cua_so_chinh;
} du_lieu_chia_se;

phuong_tien danh_sach_phuong_tien[so_luong_cho];
int so_luong_xe = 0;
double doanh_thu = 0;
GtkWidget *nhan_thong_ke;

// Khai báo nguyên mẫu hàm
void luu_doanh_thu();
void mo_doanh_thu();
void doc_file_bai_do();
void cap_nhat_file_bai_do();
void ghi_nhat_ky(const char *bien_so, const char *hanh_dong, int phi);
int kiem_tra_bien_so_xe(const char *bien_so);
int kiem_tra_con_cho_trong();
phuong_tien* tim_phuong_tien(const char *bien_so);
void tong_doanh_thu(double phi);
void tai_du_lieu_treeview(du_lieu_chia_se *du_lieu);
static void khi_nhap_bien_so(GtkWidget *widget, gpointer du_lieu);
static void thanh_toan_va_xoa(GtkWidget *widget, gpointer du_lieu);
static void thay_doi(GtkTreeView *tree_view, GtkTreePath *path, GtkTreeViewColumn *column, gpointer du_lieu);
static void on_search_changed(GtkEditable *entry, gpointer du_lieu);
void cap_nhat_label_thong_ke();

// Lưu doanh thu
void luu_doanh_thu() {
    FILE *f = fopen("doanh_thu.txt", "w");
    if (f) {
        fprintf(f, "%.0f", doanh_thu);
        fclose(f);
    }
}

// Mở doanh thu
void mo_doanh_thu() {
    FILE *f = fopen("doanh_thu.txt", "r");
    if (f) {
        fscanf(f, "%lf", &doanh_thu);
        fclose(f);
    }
}

// Đọc file bãi đỗ xe
void doc_file_bai_do() {
    FILE *tep = fopen("parking_data.txt", "r");
    if (!tep) {
        tep = fopen("parking_data.txt", "w");
        if (tep) fclose(tep);
        return;
    }
    phuong_tien tam;
    int nam, thang, ngay, gio, phut, giay;
    so_luong_xe = 0;
    while (fscanf(tep, "%s %d %d-%d-%d %d:%d:%d %d",
                  tam.bien_so_xe, &tam.phi,
                  &nam, &thang, &ngay,
                  &gio, &phut, &giay,
                  &tam.tang) == 9) {
        struct tm tm_time = {0};
        tm_time.tm_year = nam - 1900;
        tm_time.tm_mon = thang - 1;
        tm_time.tm_mday = ngay;
        tm_time.tm_hour = gio;
        tm_time.tm_min = phut;
        tm_time.tm_sec = giay;
        tam.thoi_gian_vao = mktime(&tm_time);
        tam.bat_dau_dem = clock() - (clock_t)(difftime(time(NULL), tam.thoi_gian_vao) * CLOCKS_PER_SEC);
        if (so_luong_xe < so_luong_cho)
            danh_sach_phuong_tien[so_luong_xe++] = tam;
    }
    fclose(tep);
}

// Ghi lại file bãi đỗ khi cập nhật
void cap_nhat_file_bai_do() {
    FILE *tep = fopen("parking_data.txt", "w");
    if (!tep) return;
    for (int i = 0; i < so_luong_xe; i++) {
        char chuoi_thoi_gian[26];
        strftime(chuoi_thoi_gian, sizeof(chuoi_thoi_gian), "%Y-%m-%d %H:%M:%S",
                 localtime(&danh_sach_phuong_tien[i].thoi_gian_vao));
        fprintf(tep, "%s %d %s %d\n",
                danh_sach_phuong_tien[i].bien_so_xe,
                danh_sach_phuong_tien[i].phi,
                chuoi_thoi_gian,
                danh_sach_phuong_tien[i].tang);
    }
    fclose(tep);
}

// Ghi log hành động
void ghi_nhat_ky(const char *bien_so, const char *hanh_dong, int phi) {
    FILE *log = fopen("log.txt", "a");
    if (!log) return;
    time_t bay_gio = time(NULL);
    char chuoi_thoi_gian[30];
    strftime(chuoi_thoi_gian, sizeof(chuoi_thoi_gian), "%Y-%m-%d %H:%M:%S", localtime(&bay_gio));
    if (strcmp(hanh_dong, "out") == 0)
        fprintf(log, "%s %s %d %s\n", bien_so, hanh_dong, phi, chuoi_thoi_gian);
    else
        fprintf(log, "%s %s %s\n", bien_so, hanh_dong, chuoi_thoi_gian);
    fclose(log);
}

// Kiểm tra định dạng biển số
int kiem_tra_bien_so_xe(const char *bien_so_xe) {
      while (1) {
        if (strlen(bien_so_xe) != 10) {
            // Độ dài không hợp lệ
        } else if (!isdigit(bien_so_xe[0]) || !isdigit(bien_so_xe[1])) {
            // 2 ký tự đầu không phải chữ số
        } else if (!isupper(bien_so_Xe[2])) {
            // Ký tự thứ 3 không phải chữ cái in hoa
        } else if (bien_so_xe[3] != '-') {
            // Ký tự thứ 4 không phải dấu '-'
        } else if (!isdigit(bien_so_xe[4]) || !isdigit(bien_so_xe[5]) || !isdigit(bien_so_xe[6])) {
            // 3 ký tự tiếp theo không phải chữ số
        } else if (bien_so_xe[7] != '.') {
            // Ký tự thứ 8 không phải dấu '.'
        } else if (!isdigit(bien_so_xe[8]) || !isdigit(bien_so_xe[9])) {
            // 2 ký tự cuối không phải chữ số
        } else {
<<<<<<< Updated upstream
            // Biển số hợp lệ
=======
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

// Hàm nhập biển số và thêm xe
static void onNhapBienSoXe(GtkWidget *widget, gpointer data) {
    SharedData *info = (SharedData *)data;
    GtkWindow *parent_window = info->parent_window;
    GtkWidget *dialog, *content_area, *entry_plate, *entry_floor, *combo_type;

    dialog = gtk_dialog_new_with_buttons("Nhập thông tin xe", parent_window,
                                         GTK_DIALOG_MODAL,
                                         "_OK", GTK_RESPONSE_OK,
                                         "_Hủy", GTK_RESPONSE_CANCEL, NULL);

    content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));

    GtkWidget *grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(grid), 5);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 5);

    GtkWidget *label_plate = gtk_label_new("Biển số:");
    entry_plate = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(entry_plate), "VD: 59A-123.45");

    GtkWidget *label_floor = gtk_label_new("Tầng (1 đến 4):");
    entry_floor = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(entry_floor), "1");

    GtkWidget *label_type = gtk_label_new("Loại xe:");
    combo_type = gtk_combo_box_text_new();
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(combo_type), "Xe máy");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(combo_type), "Ô tô");
    gtk_combo_box_set_active(GTK_COMBO_BOX(combo_type), 0); // Mặc định là Xe máy

    gtk_grid_attach(GTK_GRID(grid), label_plate, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), entry_plate, 1, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), label_floor, 0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), entry_floor, 1, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), label_type, 0, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), combo_type, 1, 2, 1, 1);

    gtk_container_add(GTK_CONTAINER(content_area), grid);
    gtk_widget_show_all(dialog);

    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK) {
        const gchar *plate_input = gtk_entry_get_text(GTK_ENTRY(entry_plate));
        const gchar *floor_input = gtk_entry_get_text(GTK_ENTRY(entry_floor));
        int selected = gtk_combo_box_get_active(GTK_COMBO_BOX(combo_type));

        if (!Check__license_plate(plate_input)) {
            GtkWidget *err = gtk_message_dialog_new(parent_window, GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR,
                                                    GTK_BUTTONS_CLOSE, "Biển số không hợp lệ!\nĐịnh dạng: XXA-XXX.XX(oto)\nĐịnh dạng: XX-AX-XXX.XX(xe máy)");
            gtk_dialog_run(GTK_DIALOG(err));
            gtk_widget_destroy(err);
            gtk_widget_destroy(dialog);
>>>>>>> Stashed changes
            return;
        }
	}

// Kiểm tra còn chỗ trống
int kiem_tra_con_cho_trong() {
    return so_luong_xe < so_luong_cho;
}

// Tìm xe theo biển số
phuong_tien* tim_phuong_tien(const char *bien_so) {
    for (int i = 0; i < so_luong_xe; i++)
        if (strcmp(danh_sach_phuong_tien[i].bien_so_xe, bien_so) == 0)
            return &danh_sach_phuong_tien[i];
    return NULL;
}

// Tính tổng doanh thu
void tong_doanh_thu(double phi) {
    doanh_thu += phi;
    luu_doanh_thu();
}

// Cập nhật TreeView
void tai_du_lieu_treeview(du_lieu_chia_se *du_lieu) {
    gtk_list_store_clear(du_lieu->store_tang1);
    gtk_list_store_clear(du_lieu->store_tang2);
    GtkTreeIter iter;
<<<<<<< Updated upstream
    for (int i = 0; i < so_luong_xe; i++) {
        if (danh_sach_phuong_tien[i].tang == 1) {
            gtk_list_store_append(du_lieu->store_tang1, &iter);
            gtk_list_store_set(du_lieu->store_tang1, &iter, 0,
                               danh_sach_phuong_tien[i].bien_so_xe, -1);
=======

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

// Hàm xử lý sự kiện khi nhấn nút
static void ThanhtoanvaXoa(GtkWidget *widget, gpointer data) {
    SharedData *info = (SharedData*)data;
    GtkWindow *parent_window = info->parent_window;

    GtkWidget *dialog, *entry;
    dialog = gtk_dialog_new_with_buttons("Thanh toán & Xóa xe",
                                         parent_window,
                                         GTK_DIALOG_MODAL,
                                         "_OK", GTK_RESPONSE_OK,
                                         "_Hủy", GTK_RESPONSE_CANCEL, NULL);

    GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    GtkWidget *label = gtk_label_new("Nhập biển số xe:");
    entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(entry), "VD: 59A-123.45");

    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_box_pack_start(GTK_BOX(box), label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(box), entry, FALSE, FALSE, 0);
    gtk_container_add(GTK_CONTAINER(content_area), box);
    gtk_widget_show_all(dialog);

    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_OK) {
        const gchar *plate_input = gtk_entry_get_text(GTK_ENTRY(entry));
        vehicle *veh = find_vehicle(plate_input);
        if (veh == NULL) {
            GtkWidget *err = gtk_message_dialog_new(parent_window, GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR,
                                                    GTK_BUTTONS_CLOSE, "Không tìm thấy xe!");
            gtk_dialog_run(GTK_DIALOG(err));
            gtk_widget_destroy(err);
>>>>>>> Stashed changes
        } else {
            gtk_list_store_append(du_lieu->store_tang2, &iter);
            gtk_list_store_set(du_lieu->store_tang2, &iter, 0,
                               danh_sach_phuong_tien[i].bien_so_xe, -1);
        }
    }
}

// Callback thêm xe
static void khi_nhap_bien_so(GtkWidget *widget, gpointer du_lieu) {
    du_lieu_chia_se *dl = (du_lieu_chia_se*)du_lieu;
    GtkWidget *hop_thoai = gtk_dialog_new_with_buttons(
        "Nhập biển số xe", dl->cua_so_chinh, GTK_DIALOG_MODAL,
        "_OK", GTK_RESPONSE_OK, "_Hủy", GTK_RESPONSE_CANCEL, NULL);
    GtkWidget *content = gtk_dialog_get_content_area(GTK_DIALOG(hop_thoai));
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    GtkWidget *entry_plate = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(entry_plate), "VD: 59A-123.45");
    GtkWidget *entry_floor = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(entry_floor), "1 hoặc 2");
    gtk_box_pack_start(GTK_BOX(vbox), gtk_label_new("Biển số:"), FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), entry_plate, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), gtk_label_new("Tầng (1 hoặc 2):"), FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), entry_floor, FALSE, FALSE, 0);
    gtk_container_add(GTK_CONTAINER(content), vbox);
    gtk_widget_show_all(hop_thoai);
    if (gtk_dialog_run(GTK_DIALOG(hop_thoai)) == GTK_RESPONSE_OK) {
        const gchar *plate = gtk_entry_get_text(GTK_ENTRY(entry_plate));
        const gchar *floor_str = gtk_entry_get_text(GTK_ENTRY(entry_floor));
        int floor = atoi(floor_str);
        if (!kiem_tra_bien_so_xe(plate)) {
            GtkWidget *err = gtk_message_dialog_new(dl->cua_so_chinh,
                GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR,
                GTK_BUTTONS_CLOSE, "Biển số không hợp lệ!\nĐịnh dạng: XXA-XXX.XX");
            gtk_dialog_run(GTK_DIALOG(err)); gtk_widget_destroy(err);
        } else if (floor < 1 || floor > 2) {
            GtkWidget *err = gtk_message_dialog_new(dl->cua_so_chinh,
                GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR,
                GTK_BUTTONS_CLOSE, "Chỉ chấp nhận tầng 1 hoặc 2.");
            gtk_dialog_run(GTK_DIALOG(err)); gtk_widget_destroy(err);
        } else {
            if (!kiem_tra_con_cho_trong()) {
                GtkWidget *warn = gtk_message_dialog_new(dl->cua_so_chinh,
                    GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR,
                    GTK_BUTTONS_CLOSE, "Bãi xe đã đầy!");
                gtk_dialog_run(GTK_DIALOG(warn)); gtk_widget_destroy(warn);
            } else {
                for (int i=0;i<so_luong_xe;i++)
                    if (strcmp(plate, danh_sach_phuong_tien[i].bien_so_xe)==0) {
                        GtkWidget *err = gtk_message_dialog_new(dl->cua_so_chinh,
                            GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR,
                            GTK_BUTTONS_CLOSE, "Biển số đã tồn tại!");
                        gtk_dialog_run(GTK_DIALOG(err)); gtk_widget_destroy(err);
                        gtk_widget_destroy(hop_thoai);
                        return;
                    }
                phuong_tien newv;
                strncpy(newv.bien_so_xe, plate, sizeof(newv.bien_so_xe));
                newv.tang=floor; newv.thoi_gian_vao=time(NULL);
                newv.bat_dau_dem=clock(); newv.phi=0;
                danh_sach_phuong_tien[so_luong_xe++]=newv;
                cap_nhat_file_bai_do();
                GtkTreeIter iter;
                if (floor==1) gtk_list_store_append(dl->store_tang1,&iter),
                    gtk_list_store_set(dl->store_tang1,&iter,0,plate,-1);
                else gtk_list_store_append(dl->store_tang2,&iter),
                    gtk_list_store_set(dl->store_tang2,&iter,0,plate,-1);
                ghi_nhat_ky(plate,"in",0);
            }
        }
    }
    gtk_widget_destroy(hop_thoai);
}

// Callback thanh toán và xóa
static void thanh_toan_va_xoa(GtkWidget *widget, gpointer du_lieu) {
    du_lieu_chia_se *dl = (du_lieu_chia_se*)du_lieu;
    GtkWidget *hop_thoai = gtk_dialog_new_with_buttons(
        "Thanh toán & Xóa xe", dl->cua_so_chinh, GTK_DIALOG_MODAL,
        "_OK", GTK_RESPONSE_OK, "_Hủy", GTK_RESPONSE_CANCEL, NULL);
    GtkWidget *content=gtk_dialog_get_content_area(GTK_DIALOG(hop_thoai));
    GtkWidget *vbox=gtk_box_new(GTK_ORIENTATION_VERTICAL,5);
    GtkWidget *entry_plate=gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(entry_plate),"VD: 59A-123.45");
    gtk_box_pack_start(GTK_BOX(vbox),gtk_label_new("Biển số:"),FALSE,FALSE,0);
    gtk_box_pack_start(GTK_BOX(vbox),entry_plate,FALSE,FALSE,0);
    gtk_container_add(GTK_CONTAINER(content),vbox);
    gtk_widget_show_all(hop_thoai);
    if(gtk_dialog_run(GTK_DIALOG(hop_thoai))==GTK_RESPONSE_OK){
        const gchar *plate=gtk_entry_get_text(GTK_ENTRY(entry_plate));
        phuong_tien *v=tim_phuong_tien(plate);
        if(!v){
            GtkWidget *err=gtk_message_dialog_new(dl->cua_so_chinh,GTK_DIALOG_MODAL,GTK_MESSAGE_ERROR,
                GTK_BUTTONS_CLOSE,"Không tìm thấy xe!");
            gtk_dialog_run(GTK_DIALOG(err));gtk_widget_destroy(err);
        } else {
            clock_t end=clock();
            double elapsed=(double)(end-v->bat_dau_dem)/CLOCKS_PER_SEC;
            int hours=(int)(elapsed/3600);
            int total_h=(elapsed+3599)/3600;
            v->phi=total_h*don_gia_theo_gio;
            tong_doanh_thu(v->phi);
            ghi_nhat_ky(plate,"out",v->phi);
            char msg[100];
            snprintf(msg,sizeof(msg),"Xe %s\nThời gian: %.2f giờ\nPhí: %d VND",
                     plate,elapsed/3600, v->phi);
            GtkWidget *info=gtk_message_dialog_new(dl->cua_so_chinh,GTK_DIALOG_MODAL,GTK_MESSAGE_INFO,
                GTK_BUTTONS_OK,"%s",msg);
            gtk_dialog_run(GTK_DIALOG(info));gtk_widget_destroy(info);
            // xóa khỏi danh sách
            for(int i=0;i<so_luong_xe;i++){
                if(strcmp(danh_sach_phuong_tien[i].bien_so_xe,plate)==0){
                    for(int j=i;j<so_luong_xe-1;j++) danh_sach_phuong_tien[j]=danh_sach_phuong_tien[j+1];
                    so_luong_xe--; break;
                }
            }
            cap_nhat_file_bai_do();
            tai_du_lieu_treeview(dl);
            cap_nhat_label_thong_ke();
        }
    }
    gtk_widget_destroy(hop_thoai);
}

// Cập nhật thống kê
void cap_nhat_label_thong_ke() {
    int in_cnt=0,out_cnt=0;
    mo_doanh_thu();
    FILE *log=fopen("log.txt","r");
    if(log){ char line[256];
        while(fgets(line,sizeof(line),log)){
            if(strstr(line," in ")) in_cnt++;
            else if(strstr(line," out ")) out_cnt++;
        }
        fclose(log);
    }
    char txt[256];
    snprintf(txt,sizeof(txt),"Xe vào: %d\nXe ra: %d\nDoanh thu: %.0f VND",
             in_cnt,out_cnt,doanh_thu);
    gtk_label_set_text(GTK_LABEL(nhan_thong_ke),txt);
}

// Lọc theo tìm kiếm
void filter_treeviews(du_lieu_chia_se *du_lieu,const char *kw){
    gtk_list_store_clear(du_lieu->store_tang1);
    gtk_list_store_clear(du_lieu->store_tang2);
    GtkTreeIter iter;
    for(int i=0;i<so_luong_xe;i++){
        if(strstr(danh_sach_phuong_tien[i].bien_so_xe,kw)){
            if(danh_sach_phuong_tien[i].tang==1){
                gtk_list_store_append(du_lieu->store_tang1,&iter);
                gtk_list_store_set(du_lieu->store_tang1,&iter,0,danh_sach_phuong_tien[i].bien_so_xe,-1);
            } else {
                gtk_list_store_append(du_lieu->store_tang2,&iter);
                gtk_list_store_set(du_lieu->store_tang2,&iter,0,danh_sach_phuong_tien[i].bien_so_xe,-1);
            }
        }
    }
}
static void on_search_changed(GtkEditable *entry,gpointer du_lieu){
    du_lieu_chia_se *dl=(du_lieu_chia_se*)du_lieu;
    const gchar *txt=gtk_entry_get_text(GTK_ENTRY(entry));
    filter_treeviews(dl,txt);
}

// Hàm activate khởi tạo giao diện
static void activate(GtkApplication *app,gpointer user_data){
    GtkWidget *window=gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window),"Quản lý bãi giữ xe");
    gtk_window_set_default_size(GTK_WINDOW(window),600,500);
    GtkWidget *vbox=gtk_box_new(GTK_ORIENTATION_VERTICAL,5);
    // Nút chức năng
    GtkWidget *hbox=gtk_box_new(GTK_ORIENTATION_HORIZONTAL,5);
    GtkWidget *btn_them=gtk_button_new_with_label("Thêm");
    GtkWidget *btn_xoa=gtk_button_new_with_label("Xóa & Thanh toán");
    gtk_box_pack_end(GTK_BOX(hbox),btn_xoa,FALSE,FALSE,0);
    gtk_box_pack_end(GTK_BOX(hbox),btn_them,FALSE,FALSE,0);
    gtk_box_pack_start(GTK_BOX(vbox),hbox,FALSE,FALSE,5);
    // Tab notebook
    GtkWidget *notebook=gtk_notebook_new();
    // Tab Thống kê
    GtkWidget *lbl_stats=gtk_label_new("Đang tải..."); nhan_thong_ke=lbl_stats;
    GtkWidget *box_stats=gtk_box_new(GTK_ORIENTATION_VERTICAL,5);
    gtk_box_pack_start(GTK_BOX(box_stats),lbl_stats,FALSE,FALSE,5);
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook),box_stats,gtk_label_new("Thống kê"));
    // Tab Bãi xe
    GtkWidget *box_parking=gtk_box_new(GTK_ORIENTATION_VERTICAL,5);
    GtkWidget *entry_search=gtk_entry_new(); gtk_entry_set_placeholder_text(GTK_ENTRY(entry_search),"Tìm biển số...");
    gtk_box_pack_start(GTK_BOX(box_parking),entry_search,FALSE,FALSE,5);
    // TreeViews tầng
    GtkListStore *store1=gtk_list_store_new(1,G_TYPE_STRING);
    GtkWidget *tree1=gtk_tree_view_new_with_model(GTK_TREE_MODEL(store1));
    GtkCellRenderer *r1=gtk_cell_renderer_text_new();
    GtkTreeViewColumn *c1=gtk_tree_view_column_new_with_attributes("Biển số xe",r1,"text",0,NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(tree1),c1);
    GtkWidget *sw1=gtk_scrolled_window_new(NULL,NULL); gtk_container_add(GTK_CONTAINER(sw1),tree1);
    gtk_box_pack_start(GTK_BOX(box_parking),sw1,TRUE,TRUE,5);
    GtkListStore *store2=gtk_list_store_new(1,G_TYPE_STRING);
    GtkWidget *tree2=gtk_tree_view_new_with_model(GTK_TREE_MODEL(store2));
    GtkCellRenderer *r2=gtk_cell_renderer_text_new();
    GtkTreeViewColumn *c2=gtk_tree_view_column_new_with_attributes("Biển số xe",r2,"text",0,NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(tree2),c2);
    GtkWidget *sw2=gtk_scrolled_window_new(NULL,NULL); gtk_container_add(GTK_CONTAINER(sw2),tree2);
    gtk_box_pack_start(GTK_BOX(box_parking),sw2,TRUE,TRUE,5);
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook),box_parking,gtk_label_new("Bãi xe"));
    // Tab Lịch sử
    GtkListStore *store_hist=gtk_list_store_new(5,G_TYPE_INT,G_TYPE_STRING,G_TYPE_STRING,G_TYPE_STRING,G_TYPE_STRING);
    GtkWidget *tree_hist=gtk_tree_view_new_with_model(GTK_TREE_MODEL(store_hist));
    const char *titles[5]={"STT","Biển số","Trạng thái","Thời gian","Phí"};
    for(int i=0;i<5;i++){ GtkCellRenderer *r=gtk_cell_renderer_text_new(); GtkTreeViewColumn *col=gtk_tree_view_column_new_with_attributes(titles[i],r,"text",i,NULL); gtk_tree_view_append_column(GTK_TREE_VIEW(tree_hist),col);}    
    GtkWidget *sw_hist=gtk_scrolled_window_new(NULL,NULL); gtk_container_add(GTK_CONTAINER(sw_hist),tree_hist);
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook),sw_hist,gtk_label_new("Lịch sử"));
    // Hiển thị
    gtk_container_add(GTK_CONTAINER(vbox),notebook);
    gtk_container_add(GTK_CONTAINER(window),vbox);
    gtk_widget_show_all(window);
    
    // Khởi tạo dữ liệu
    du_lieu_chia_se dl={store1,store2,GTK_WINDOW(window)};
    g_signal_connect(btn_them,"clicked",G_CALLBACK(khi_nhap_bien_so),&dl);
    g_signal_connect(btn_xoa,"clicked",G_CALLBACK(thanh_toan_va_xoa),&dl);
    g_signal_connect(tree1,"row-activated",G_CALLBACK(thay_doi),&dl);
    g_signal_connect(tree2,"row-activated",G_CALLBACK(thay_doi),&dl);
    g_signal_connect(entry_search,"changed",G_CALLBACK(on_search_changed),&dl);
    mo_doanh_thu(); doc_file_bai_do(); tai_du_lieu_treeview(&dl); cap_nhat_label_thong_ke();
}

int main(int argc,char **argv) {
    GtkApplication *app = gtk_application_new("com.quanly.baixe", G_APPLICATION_FLAGS_NONE);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
    int status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);
    return status;