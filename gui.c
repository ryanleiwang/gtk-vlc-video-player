#include <gtk/gtk.h>
#include <gdk/gdkx.h>
#include <glib.h>

#define BORDER_WIDTH 6

int main(int argc, char* argv[])
{
    GtkWidget   *window,
                *vbox,
                *hbox,
                *menubar,
                *filemenu,
                *fileitem,
                *filemenu_openitem,
                *hbuttonbox,
                *player_widget,
                *stop_button,
                *full_screen_button,
                *playpause_button,
                *process_scale,
                *play_icon_image,
                *pause_icon_image,
                *stop_icon_image;
    GtkAdjustment  *process_adjuest;

    // 每个gtk程序都必须要有的，两个参数对应mian函数的两个参数，用于在命令行执行程序时传递并解析参数
    gtk_init(&argc, &argv);

    // 创建一个window并完成初始化，如设置为顶层窗口，宽度和高度，标题等，并绑定destory信号，以便在关闭gtk窗口后程序能完全退出
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_default_size(GTK_WINDOW(window), 400, 300);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    gtk_container_set_border_width (GTK_CONTAINER (window), 0);
    gtk_window_set_title(GTK_WINDOW(window), "GTK+ libVLC Demo");

    //创建一个方向垂直间距为0的box容器，并添加到前面创建的window中
    vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add(GTK_CONTAINER(window), vbox);

    //创建一个menubar和两个menuitem分别为菜单中的“文件”和“打开”，由于它们为上下级菜单关系，
    //所以需要单独一个menu来放置"open_menu_item",也就是代码中的filemenu_openitem
    menubar = gtk_menu_bar_new();
    fileitem = gtk_menu_item_new_with_label ("File");
    filemenu_openitem = gtk_menu_item_new_with_label("Open");
    filemenu = gtk_menu_new();
    gtk_menu_shell_append(GTK_MENU_SHELL(filemenu), filemenu_openitem);

    // 将filemenu设置为上一级fileitem的子菜单，然后将fileitem添加进menubar,最后将menubar放置进vbox
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(fileitem), filemenu);
    gtk_menu_shell_append(GTK_MENU_SHELL(menubar), fileitem);
    gtk_box_pack_start(GTK_BOX(vbox), menubar, FALSE, FALSE, 0);

    //创建一个draw_area控件，用做视频播放显示区域，并放置进vbox
    player_widget = gtk_drawing_area_new();
    gtk_box_pack_start(GTK_BOX(vbox), player_widget, TRUE, TRUE, 0);

    //创建一个hbox作为vbox的子容器，一个hbuttonbox作为hbox的子容器，hbuttonbox用于放置两个button,
    // 再将一个scale（滚动条，用作视频播放进度条，原本的process控件不能拖动）添加进hbox,最后将hbox放置进最外面的vbox
    hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    hbuttonbox = gtk_button_box_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_container_set_border_width(GTK_CONTAINER(hbuttonbox), BORDER_WIDTH);
    gtk_button_box_set_layout(GTK_BUTTON_BOX(hbuttonbox), GTK_BUTTONBOX_START);

    playpause_button = gtk_button_new_from_icon_name("media-playback-start", GTK_ICON_SIZE_BUTTON);
    stop_button = gtk_button_new_from_icon_name("media-playback-stop", GTK_ICON_SIZE_BUTTON);

    gtk_box_pack_start(GTK_BOX(hbuttonbox), playpause_button, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbuttonbox), stop_button, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox), hbuttonbox, FALSE, FALSE, 0);

    //setup scale
    process_adjuest = gtk_adjustment_new(0.00, 0.00, 100.00, 1.00, 0.00, 0.00);
    process_scale = gtk_scale_new(GTK_ORIENTATION_HORIZONTAL,process_adjuest);
    gtk_box_pack_start(GTK_BOX(hbox), process_scale, TRUE, TRUE, 0);
    gtk_scale_set_draw_value (GTK_SCALE(process_scale), FALSE);
    gtk_scale_set_has_origin (GTK_SCALE(process_scale), TRUE);
    gtk_scale_set_value_pos(GTK_SCALE(process_scale), 0);
    gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, TRUE, 0);

    // 显示所有控件，并运行gtk程序
    gtk_widget_show_all(window);
    gtk_main ();

    return 0;
}
