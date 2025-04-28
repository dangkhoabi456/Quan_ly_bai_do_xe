#include <gtk/gtk.h>
#include "QuanLyBaiGiuXe.h"

static void activate(GtkApplication *app, gpointer user_data) {
    GtkWidget *window;
    GtkWidget *containerBox;
    GtkWidget *top_right_box, *btn1, *btn2;
    GtkWidget *notebook;
    GtkWidget *tab_label1, *tab_label2, *tab_label3;
    GtkWidget *home_box, *label_welcome, *label_fee, *label_note;
    GtkWidget *tab_content2;
    GtkWidget *bai_xe_vbox, *search_entry;
    GtkWidget *nested_notebook;
    GtkWidget *tab_tang1, *tab_tang2;
    GtkListStore *store_tang1, *store_tang2;
    GtkWidget *treeview_tang1, *treeview_tang2;
    GtkWidget *scroll_tang1, *scroll_tang2;
    GtkWidget *scrolled_history, *history_view;
    GtkListStore *store;
    const char *titles[] = {"STT", "Biển số xe", "Trạng thái", "Thời gian lúc lấy xe", "Chi phí giữ xe"};

    // === Khởi tạo cửa sổ chính ===
    window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "Quản lý bãi giữ xe");
    gtk_window_set_default_size(GTK_WINDOW(window), 500, 500);

    containerBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    top_right_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);

    btn1 = gtk_button_new_with_label("Thêm");
    btn2 = gtk_button_new_with_label("Xóa và Thanh toán");

    gtk_box_pack_start(GTK_BOX(top_right_box), btn1, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(top_right_box), btn2, FALSE, FALSE, 0);
    gtk_widget_set_halign(top_right_box, GTK_ALIGN_END);
    gtk_box_pack_start(GTK_BOX(containerBox), top_right_box, FALSE, FALSE, 5);

    notebook = gtk_notebook_new();

    // === Trang chủ ===
    tab_label1 = gtk_label_new("Trang chủ");
    home_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    label_welcome = gtk_label_new("Chào mừng đến với hệ thống quản lý bãi giữ xe");
    label_fee = gtk_label_new("Phí giữ xe: 5.000 VND");
    label_note = gtk_label_new("Phí giữ xe được tính theo giờ. Nếu bạn gửi xe chưa đủ 1 giờ thì vẫn tính tròn là 1 giờ.");
    gtk_box_pack_start(GTK_BOX(home_box), label_welcome, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(home_box), label_fee, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(home_box), label_note, FALSE, FALSE, 0);
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), home_box, tab_label1);

    // === Thống kê ===
    tab_label2 = gtk_label_new("Thống kê");
    tab_content2 = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);

    label_stats = gtk_label_new("Đang tải thống kê...");
    gtk_box_pack_start(GTK_BOX(tab_content2), label_stats, FALSE, FALSE, 10);
    update_statistics_display();
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), tab_content2, tab_label2);

    // === Bãi xe ===
    tab_label3 = gtk_label_new("Bãi xe");
    bai_xe_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    nested_notebook = gtk_notebook_new();

    search_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(search_entry), "Tìm biển số xe...");
    gtk_box_pack_start(GTK_BOX(bai_xe_vbox), search_entry, FALSE, FALSE, 0);

    // Tầng 1
    tab_tang1 = gtk_label_new("Tầng 1");
    store_tang1 = gtk_list_store_new(1, G_TYPE_STRING);
    treeview_tang1 = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store_tang1));
    GtkCellRenderer *renderer1 = gtk_cell_renderer_text_new();
    GtkTreeViewColumn *column1 = gtk_tree_view_column_new_with_attributes("Biển số xe", renderer1, "text", 0, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeview_tang1), column1);
    scroll_tang1 = gtk_scrolled_window_new(NULL, NULL);
    gtk_container_add(GTK_CONTAINER(scroll_tang1), treeview_tang1);
    gtk_widget_set_vexpand(scroll_tang1, TRUE);
    gtk_widget_set_hexpand(scroll_tang1, TRUE);
    gtk_notebook_append_page(GTK_NOTEBOOK(nested_notebook), scroll_tang1, tab_tang1);

    // Tầng 2
    tab_tang2 = gtk_label_new("Tầng 2");
    store_tang2 = gtk_list_store_new(1, G_TYPE_STRING);
    treeview_tang2 = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store_tang2));
    GtkCellRenderer *renderer2 = gtk_cell_renderer_text_new();
    GtkTreeViewColumn *column2 = gtk_tree_view_column_new_with_attributes("Biển số xe", renderer2, "text", 0, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeview_tang2), column2);
    scroll_tang2 = gtk_scrolled_window_new(NULL, NULL);
    gtk_container_add(GTK_CONTAINER(scroll_tang2), treeview_tang2);
    gtk_widget_set_vexpand(scroll_tang2, TRUE);
    gtk_widget_set_hexpand(scroll_tang2, TRUE);
    gtk_notebook_append_page(GTK_NOTEBOOK(nested_notebook), scroll_tang2, tab_tang2);

    gtk_box_pack_start(GTK_BOX(bai_xe_vbox), nested_notebook, TRUE, TRUE, 0);
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), bai_xe_vbox, tab_label3);

    // SharedData
    SharedData *shared_data = g_new(SharedData, 1);
    shared_data->store_tang1 = store_tang1;
    shared_data->store_tang2 = store_tang2;
    shared_data->parent_window = GTK_WINDOW(window);

    g_signal_connect(search_entry, "changed", G_CALLBACK(on_search_changed), shared_data);
    g_signal_connect(btn1, "clicked", G_CALLBACK(onNhapBienSoXe), shared_data);
    g_signal_connect(btn2, "clicked", G_CALLBACK(ThanhtoanvaXoa), shared_data);

    load_doanh_thu();
    read_from_file();
    load_treeviews(shared_data);

    // Lịch sử giữ xe
    scrolled_history = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_history), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    history_view = gtk_tree_view_new();
    gtk_container_add(GTK_CONTAINER(scrolled_history), history_view);
    store = gtk_list_store_new(5, G_TYPE_INT, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);
    gtk_tree_view_set_model(GTK_TREE_VIEW(history_view), GTK_TREE_MODEL(store));
    g_object_unref(store);

    load_history_data(store);

    for (int i = 0; i < 5; i++) {
        GtkCellRenderer *renderer = gtk_cell_renderer_text_new();
        GtkTreeViewColumn *column = gtk_tree_view_column_new_with_attributes(titles[i], renderer, "text", i, NULL);
        gtk_tree_view_append_column(GTK_TREE_VIEW(history_view), column);
    }

    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), scrolled_history, gtk_label_new("Lịch sử giữ xe"));

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
