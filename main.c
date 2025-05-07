#include "parking.h"
#include <gtk/gtk.h>

static void activate(GtkApplication *app, gpointer user_data) {
    GtkWidget *window;
    GtkWidget *containerBox;
    //Cỡ chữ mặc định cho chương trình


    set_font_size(16); 


    //thống kê theo tầng
    label_thongke = gtk_label_new((const gchar*)floor_statistics());
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
    GtkWidget *label_fee = gtk_label_new("Phí giữ xe: 5.000 VND (ô tô) || 2.000 VND (xe máy)");

    GtkWidget *label_note = gtk_label_new("Phí giữ xe được tính theo giờ.Nếu bạn gửi xe chưa đủ 1 giờ thì vẫn tính tròn là 1 giờ.");

    label_vehicle_count = gtk_label_new(NULL);
	  update_vehicle_count_label();
    gtk_box_pack_start(GTK_BOX(home_box), label_welcome, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(home_box), label_fee, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(home_box), label_note, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(home_box), label_vehicle_count, FALSE, FALSE, 0);
    GtkWidget *label_thongke = gtk_label_new((const gchar*)floor_statistics());
	shared_data.home_stat_label = label_thongke;    // nếu dùng biến struct
	gtk_box_pack_start(GTK_BOX(home_box), label_thongke, FALSE, FALSE, 10);
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

	//=== TAB 4: Doanh thu ===
GtkWidget *tab_revenue = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
treeview_revenue = gtk_tree_view_new();

gtk_box_pack_start(GTK_BOX(tab_revenue), treeview_revenue, TRUE, TRUE, 0);
display_revenue_treeview(GTK_TREE_VIEW(treeview_revenue));

gtk_notebook_append_page(GTK_NOTEBOOK(notebook), tab_revenue, gtk_label_new("Doanh thu"));
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
g_signal_connect(search_entry, "changed", G_CALLBACK(search), &shared_data);

for (int i = 0; i < MAX_TANG; i++) {
   g_signal_connect(treeviews[i], "row-activated", G_CALLBACK(changes), &shared_data);
}
load_revenue();
read_from_file();
update_vehicle_count_label();
gtk_label_set_text(GTK_LABEL(label_thongke), floor_statistics());
load_vehicle_tree(&shared_data);

// Kết nối nút với callback và truyền shared_data
g_signal_connect(btn1, "clicked", G_CALLBACK(enter_license_plate), &shared_data);
g_signal_connect(btn2, "clicked", G_CALLBACK( pay_and_remove), &shared_data);    
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