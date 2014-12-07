// gcc -o gtk_player gtk_player.c `pkg-config --libs gtk+-3.0 libvlc` `pkg-config --cflags gtk+-3.0 libvlc`
// 修改自：http://git.videolan.org/?p=vlc.git;a=blob;f=doc/libvlc/gtk_player.c

#include <stdio.h>
#include <gtk/gtk.h>
#include <gdk/gdkx.h>
#include <glib.h>
#include <glib/gi18n.h>
#include <vlc/vlc.h>

#define BORDER_WIDTH 5
#define CTRL_WINDOW_WIDTH 700
#define CTRL_WINDOW_HEIGHT 25

void on_player_widget_on_realize(GtkWidget *widget, gpointer data);
void on_open(GtkWidget *widget, gpointer data);
void on_full_screen(GtkWidget *widget, gpointer data);
void on_playpause(GtkWidget *widget, gpointer data);
void on_stop(GtkWidget *widget, gpointer data);
void on_scale_value_change(GtkWidget *widget, gpointer data);

void open_media(const char* uri);
void play(GtkWidget *widget, libvlc_media_player_t *player);
void pause_player(GtkWidget *widget, libvlc_media_player_t *player);
gboolean _update_scale(gpointer data);
gboolean _hide_ctrl_window(gpointer data);

libvlc_media_player_t *media_player;
libvlc_instance_t *vlc_inst;

GtkWidget *play_icon_image,*pause_icon_image,*stop_icon_image;
GtkWidget *window, *player_widget, *pause_button, *stop_button, *process_scale, *hbox, *vbox;
GtkWidget *full_window, *full_screen_pause_button, *full_screen_stop_button, *ctrl_process_scale, *ctrl_window;
GtkAdjustment *process_adjuest;
gint width, height;
gboolean is_fullscreen, is_moving;

GdkCursor *cur;

float video_length, current_play_time;

void on_player_widget_on_realize(GtkWidget *widget, gpointer data) {
    libvlc_media_player_set_xwindow((libvlc_media_player_t*)data, GDK_WINDOW_XID(gtk_widget_get_window(widget)));
}

void on_open(GtkWidget *widget, gpointer data) {
    GtkWidget *dialog;
    GtkFileChooserAction action = GTK_FILE_CHOOSER_ACTION_OPEN;

    dialog = gtk_file_chooser_dialog_new("open file", GTK_WINDOW(widget), action, _("Cancel"), GTK_RESPONSE_CANCEL, _("Open"), GTK_RESPONSE_ACCEPT, NULL);

    if(gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        char *uri;
        uri = gtk_file_chooser_get_uri(GTK_FILE_CHOOSER(dialog));
        open_media(uri);
        g_free(uri);
    }
    gtk_widget_destroy(dialog);
}

void open_media(const char* uri) {
    libvlc_media_t *media;
    media = libvlc_media_new_location(vlc_inst, uri);
    libvlc_media_player_set_media(media_player, media);

    current_play_time = 0.0f;
    gtk_scale_set_value_pos(GTK_SCALE(process_scale), current_play_time/video_length*100);

    play(pause_button, media_player);

    libvlc_media_release(media);
    g_timeout_add(500,(GSourceFunc)_update_scale, process_scale);
}

void play(GtkWidget *widget, libvlc_media_player_t *player) {
    libvlc_media_player_play(player);
    pause_icon_image = gtk_image_new_from_icon_name("media-playback-pause", GTK_ICON_SIZE_BUTTON);
    gtk_button_set_image(GTK_BUTTON(widget), pause_icon_image);
}

void pause_player(GtkWidget *widget, libvlc_media_player_t *player) {
    libvlc_media_player_pause(player);
    play_icon_image = gtk_image_new_from_icon_name("media-playback-start", GTK_ICON_SIZE_BUTTON);
    gtk_button_set_image(GTK_BUTTON(widget), play_icon_image);
}

void on_playpause(GtkWidget *widget, gpointer data) {
    if(libvlc_media_player_is_playing(media_player) == 1) {
        pause_player(widget, media_player);
    }
    else if(media_player != NULL){
        play(widget, media_player);
    }
}

void on_stop(GtkWidget *widget, gpointer data) {
    play_icon_image = gtk_image_new_from_icon_name("media-playback-start", GTK_ICON_SIZE_BUTTON);
    gtk_button_set_image(GTK_BUTTON(pause_button), play_icon_image);
    libvlc_media_player_stop((libvlc_media_player_t *)data);
}

gboolean _update_scale(gpointer data){
    // 获取当前打开视频的长度，时间单位为ms
    video_length = libvlc_media_player_get_length(media_player);
    current_play_time = libvlc_media_player_get_time(media_player);
    g_signal_handlers_block_by_func(G_OBJECT(process_scale), on_scale_value_change, NULL);
    g_signal_handlers_block_by_func(G_OBJECT(ctrl_process_scale), on_scale_value_change, NULL);
    gtk_adjustment_set_value(process_adjuest, current_play_time/video_length*100);
    g_signal_handlers_unblock_by_func(G_OBJECT(process_scale), on_scale_value_change, NULL);
    g_signal_handlers_unblock_by_func(G_OBJECT(ctrl_process_scale), on_scale_value_change, NULL);
    return TRUE;
}

gboolean _hide_ctrl_window(gpointer data)
{
    if ( (!is_fullscreen) || is_moving ) {
        return FALSE;
    }
    gdk_window_set_cursor(gtk_widget_get_window(GTK_WIDGET(window)), gdk_cursor_new(GDK_BLANK_CURSOR));
    gtk_widget_hide(GTK_WIDGET(ctrl_window));
    return TRUE;
}

void on_scale_value_change(GtkWidget *widget, gpointer data)
{
    float scale_value = gtk_adjustment_get_value(process_adjuest);
    libvlc_media_player_set_position(media_player, scale_value/100);
}
void on_mouse_motion(GtkWidget *widget, gpointer data)
{
    is_moving = TRUE;
    gtk_widget_show(GTK_WIDGET(ctrl_window));
    gdk_window_set_cursor(gtk_widget_get_window(GTK_WIDGET(window)), cur);
    width = gdk_screen_get_width(gdk_screen_get_default());
    height = gdk_screen_get_height(gdk_screen_get_default());
    gtk_window_move(GTK_WINDOW(ctrl_window), (width-CTRL_WINDOW_WIDTH)/2, height-100);
    is_moving = FALSE;
}

void on_full_screen(GtkWidget *widget, gpointer data) {
    gtk_window_fullscreen(GTK_WINDOW(window));
    is_fullscreen = TRUE;
    gtk_widget_hide(hbox);
    gtk_widget_show_all(GTK_WIDGET(ctrl_window));
    // 进入全屏取消对控制窗口的移动鼠标自动浮现的阻塞状态
    g_signal_handlers_unblock_by_func(G_OBJECT(player_widget), on_mouse_motion, NULL);
    g_timeout_add(5000, (GSourceFunc)_hide_ctrl_window, NULL);
}

void on_quit_full_screen(GtkWidget *widget, gpointer data)
{
    gtk_window_unfullscreen(GTK_WINDOW(window));
    is_fullscreen = FALSE;
    gtk_widget_show(hbox);
    gtk_widget_hide(ctrl_window);
    // 非全屏状态阻塞控制窗口的移动鼠标自动浮现
    g_signal_handlers_block_by_func(G_OBJECT(player_widget), on_mouse_motion, NULL);
}

void control_window_init(libvlc_media_player_t *player)
{
    GtkWidget *ctrl_hbox, *quit_full_screen_button;

    ctrl_window = gtk_window_new(GTK_WINDOW_POPUP);
    gtk_window_set_position(GTK_WINDOW(ctrl_window), GTK_WIN_POS_CENTER);
    gtk_window_set_default_size(GTK_WINDOW(ctrl_window), CTRL_WINDOW_WIDTH, CTRL_WINDOW_HEIGHT);
    g_signal_connect(ctrl_window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    ctrl_hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, TRUE);
    gtk_container_add(GTK_CONTAINER(ctrl_window), ctrl_hbox);

    full_screen_pause_button = gtk_button_new_from_icon_name("media-playback-pause", GTK_ICON_SIZE_BUTTON);
    full_screen_stop_button = gtk_button_new_from_icon_name("media-playback-stop", GTK_ICON_SIZE_BUTTON);
    quit_full_screen_button = gtk_button_new_from_icon_name("view-restore", GTK_ICON_SIZE_BUTTON);
    gtk_box_pack_start(GTK_BOX(ctrl_hbox), full_screen_pause_button, FALSE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(ctrl_hbox), full_screen_stop_button, FALSE, TRUE, 0);
    gtk_box_pack_end(GTK_BOX(ctrl_hbox), quit_full_screen_button, FALSE, TRUE, 0);
    g_signal_connect(G_OBJECT(full_screen_pause_button), "clicked", G_CALLBACK(on_playpause), player);
    g_signal_connect(G_OBJECT(full_screen_stop_button), "clicked", G_CALLBACK(on_stop), player);
    g_signal_connect(G_OBJECT(quit_full_screen_button), "clicked", G_CALLBACK(on_quit_full_screen), NULL);

    ctrl_process_scale = gtk_scale_new(GTK_ORIENTATION_HORIZONTAL, process_adjuest);
    gtk_box_pack_start(GTK_BOX(ctrl_hbox), ctrl_process_scale, TRUE, TRUE, 0);
    gtk_scale_set_draw_value (GTK_SCALE(ctrl_process_scale), FALSE);
    gtk_scale_set_has_origin (GTK_SCALE(ctrl_process_scale), TRUE);
    gtk_scale_set_value_pos(GTK_SCALE(ctrl_process_scale), 0);
    g_signal_connect(G_OBJECT(ctrl_process_scale),"value_changed", G_CALLBACK(on_scale_value_change), NULL);

}

int main( int argc, char *argv[] ) {
    GtkWidget   *menubar,
                *filemenu,
                *fileitem,
                *filemenu_openitem,
                *hbuttonbox,
                *full_screen_button;

    const char * const vlc_args[] = {
        //libvlc 鼠标指针自动隐藏事件与gtk的“motion_notify_event”事件有冲突会导致一些问题，
        //故这里设置最大超时时间（相当于禁止vlc的该功能）
        "--mouse-hide-timeout=2147483647",//在 x 毫秒后隐藏光标和全屏控制器
        "--no-xlib"
    };

    gtk_init (&argc, &argv);

    // setup window
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_default_size(GTK_WINDOW(window), 800, 600);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    gtk_container_set_border_width (GTK_CONTAINER (window), 0);
    gtk_window_set_title(GTK_WINDOW(window), "GTK+ libVLC Demo");

    //setup box
    vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, FALSE);
    gtk_container_add(GTK_CONTAINER(window), vbox);
    hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, FALSE);

    //setup menu
    menubar = gtk_menu_bar_new();
    filemenu = gtk_menu_new();
    fileitem = gtk_menu_item_new_with_label ("File");
    filemenu_openitem = gtk_menu_item_new_with_label("Open");
    gtk_menu_shell_append(GTK_MENU_SHELL(filemenu), filemenu_openitem);
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(fileitem), filemenu);
    gtk_menu_shell_append(GTK_MENU_SHELL(menubar), fileitem);

    gtk_box_pack_start(GTK_BOX(vbox), menubar, FALSE, FALSE, 0);
    g_signal_connect(G_OBJECT(filemenu_openitem), "activate", G_CALLBACK(on_open), window);

    //setup player widget
    player_widget = gtk_drawing_area_new();
    gtk_box_pack_start(GTK_BOX(vbox), player_widget, TRUE, TRUE, 0);

    //setup vlc
    vlc_inst = libvlc_new(2 ,vlc_args);
    media_player = libvlc_media_player_new(vlc_inst);
    g_signal_connect(G_OBJECT(player_widget), "realize", G_CALLBACK(on_player_widget_on_realize), media_player);

    //setup controls
    pause_button = gtk_button_new_from_icon_name("media-playback-start", GTK_ICON_SIZE_BUTTON);
    stop_button = gtk_button_new_from_icon_name("media-playback-stop", GTK_ICON_SIZE_BUTTON);
    full_screen_button = gtk_button_new_from_icon_name("view-fullscreen", GTK_ICON_SIZE_BUTTON);

    g_signal_connect(pause_button, "clicked", G_CALLBACK(on_playpause), media_player);
    g_signal_connect(stop_button, "clicked", G_CALLBACK(on_stop), media_player);
    g_signal_connect(full_screen_button, "clicked", G_CALLBACK(on_full_screen), NULL);

    hbuttonbox = gtk_button_box_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_container_set_border_width(GTK_CONTAINER(hbuttonbox), BORDER_WIDTH);
    gtk_button_box_set_layout(GTK_BUTTON_BOX(hbuttonbox), GTK_BUTTONBOX_START);

    gtk_box_pack_start(GTK_BOX(hbuttonbox), pause_button, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbuttonbox), stop_button, FALSE, FALSE, 0);

    gtk_box_pack_start(GTK_BOX(hbox), hbuttonbox, FALSE, FALSE, 0);
    gtk_box_pack_end(GTK_BOX(hbox), full_screen_button, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, TRUE, 0);

    //setup scale
    process_adjuest = gtk_adjustment_new(0.00, 0.00, 100.00, 1.00, 0.00, 0.00);
    process_scale = gtk_scale_new(GTK_ORIENTATION_HORIZONTAL,process_adjuest);
    gtk_box_pack_start(GTK_BOX(hbox), process_scale, TRUE, TRUE, 0);
    gtk_scale_set_draw_value (GTK_SCALE(process_scale), FALSE);
    gtk_scale_set_has_origin (GTK_SCALE(process_scale), TRUE);
    gtk_scale_set_value_pos(GTK_SCALE(process_scale), 0);
    g_signal_connect(G_OBJECT(process_scale),"value_changed", G_CALLBACK(on_scale_value_change), NULL);

    gtk_widget_add_events(GTK_WIDGET(player_widget), GDK_POINTER_MOTION_MASK);
    g_signal_connect(G_OBJECT(player_widget), "motion_notify_event", G_CALLBACK(on_mouse_motion), NULL);
    g_signal_handlers_block_by_func(G_OBJECT(player_widget), on_mouse_motion, NULL);
    cur = gdk_window_get_cursor(gtk_widget_get_window(window));

    control_window_init(media_player);

    gtk_widget_show_all(window);
    gtk_main();
    libvlc_media_player_release(media_player);
    libvlc_release(vlc_inst);
    return 0;
}
