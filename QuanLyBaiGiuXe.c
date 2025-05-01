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
            // Biển số hợp lệ
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
    for (int i = 0; i < so_luong_xe; i++) {
        if (danh_sach_phuong_tien[i].tang == 1) {
            gtk_list_store_append(du_lieu->store_tang1, &iter);
            gtk_list_store_set(du_lieu->store_tang1, &iter, 0,
                               danh_sach_phuong_tien[i].bien_so_xe, -1);
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