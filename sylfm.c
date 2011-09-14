/*
 * SylFm -- 
 * Copyright (C) 2011 HAYASHI Kentaro
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include <glib.h>
#include <gtk/gtk.h>

#include "sylmain.h"
#include "plugin.h"
#include "sylfm.h"
#include "folder.h"
#include "procmsg.h"

#include "sylfilter/filter.h"
#include "sylfilter/filter-kvs.h"
#include "sylfilter/filter-kvs-sqlite.h"
#include "sylfilter/filter-manager.h"
#include "sylfilter/filter-utils.h"
#include "sylfilter/textcontent-filter.h"
#include "sylfilter/blacklist-filter.h"
#include "sylfilter/whitelist-filter.h"
#include "sylfilter/wordsep-filter.h"
#include "sylfilter/ngram-filter.h"
#include "sylfilter/bayes-filter.h"

static SylPluginInfo info = {
	N_(PLUGIN_NAME),
	"0.1.0",
	"HAYASHI Kentaro",
	N_(PLUGIN_DESC),
};

static void init_done_cb(GObject *obj, gpointer data);
static void app_exit_cb(GObject *obj, gpointer data);

static void folderview_menu_popup_cb(GObject *obj, GtkItemFactory *ifactory,
				     gpointer data);
static void summaryview_menu_popup_cb(GObject *obj, GtkItemFactory *ifactory,
				      gpointer data);

static void textview_menu_popup_cb(GObject *obj, GtkMenu *menu,
				   GtkTextView *textview,
				   const gchar *uri,
				   const gchar *selected_text,
				   MsgInfo *msginfo);

static void menu_selected_cb(void);

static void messageview_show_cb(GObject *obj, gpointer msgview,
				MsgInfo *msginfo, gboolean all_headers);

static void create_window(void);
static void create_folderview_sub_widget(void);
static int test_filter(int mode, const char *file);
static gchar *myprocmsg_get_message_file_path(MsgInfo *msginfo);

gulong app_exit_handler_id = 0;
static int g_verbose = 0;

void plugin_load(void)
{
	GList *list, *cur;
	const gchar *ver;
	gpointer mainwin;

	g_print("[PLUGIN] sylfm plug-in loaded!\n");

    xfilter_init();

	xfilter_kvs_sqlite_set_engine();

    const char *dbpath = xfilter_utils_get_default_base_dir();
	xfilter_utils_set_base_dir(dbpath);

    if (xfilter_bayes_db_init(NULL) < 0) {
      g_print("Database initialization error\n");
      return 127;
	}

	syl_plugin_add_menuitem("/Tools", NULL, NULL, NULL);
	syl_plugin_add_menuitem("/Tools", _("SylFm settings [sylfm]"), create_window, NULL);

	g_signal_connect_after(syl_app_get(), "init-done", G_CALLBACK(init_done_cb),
			 NULL);
	app_exit_handler_id =
	g_signal_connect(syl_app_get(), "app-exit", G_CALLBACK(app_exit_cb),
			 NULL);
	syl_plugin_signal_connect("folderview-menu-popup",
				  G_CALLBACK(folderview_menu_popup_cb), NULL);
	syl_plugin_signal_connect("summaryview-menu-popup",
				  G_CALLBACK(summaryview_menu_popup_cb), NULL);
	syl_plugin_signal_connect("textview-menu-popup",
				  G_CALLBACK(textview_menu_popup_cb), NULL);
#ifndef RELEASE_3_1 /*SYL_PLUGIN_INTERFACE_VERSION >= 0x0108*/
	syl_plugin_signal_connect("messageview-show",
				  G_CALLBACK(messageview_show_cb), NULL);
#endif
	syl_plugin_add_factory_item("<SummaryView>", "/---", NULL, NULL);
	syl_plugin_add_factory_item("<SummaryView>", _("/Show sylfilter status [sylfm]"),
				    menu_selected_cb, NULL);

	g_print("[PLUGIN] sylfm plug-in loading done\n");
}

void plugin_unload(void)
{
	g_print("test plug-in unloaded!\n");
	g_signal_handler_disconnect(syl_app_get(), app_exit_handler_id);
}

SylPluginInfo *plugin_info(void)
{
	return &info;
}

gint plugin_interface_version(void)
{
#ifdef RELEASE_3_1
    /* emulate sylpheed 3.1.0 not svn HEAD */
    return 0x0107;
#else
 /* sylpheed 3.2.0 or later. */
    return 0x0108;
    /*return SYL_PLUGIN_INTERFACE_VERSION;*/
#endif
}

static void init_done_cb(GObject *obj, gpointer data)
{
	syl_plugin_update_check_set_check_url("http://localhost/version_pro.txt?");
	syl_plugin_update_check_set_download_url("http://localhost/download.php?sno=123&ver=VER&os=win");
	syl_plugin_update_check_set_jump_url("http://localhost/index.html");
	syl_plugin_update_check_set_check_plugin_url("http://localhost/plugin_version.txt");
	syl_plugin_update_check_set_jump_plugin_url("http://localhost/plugin.html");
	g_print("test: %p: app init done\n", obj);
}

static void app_exit_cb(GObject *obj, gpointer data)
{
	g_print("test: %p: app will exit\n", obj);
}

static void folderview_menu_popup_cb(GObject *obj, GtkItemFactory *ifactory,
				     gpointer data)
{
	g_print("test: %p: folderview menu popup\n", obj);
}

static void summaryview_menu_popup_cb(GObject *obj, GtkItemFactory *ifactory,
				      gpointer data)
{
  GtkWidget *widget;

  g_print("test: %p: summaryview menu popup\n", obj);
  widget = gtk_item_factory_get_item(ifactory, _("/Show sylfilter status [sylfm]"));
  if (widget){
	GSList *mlist = syl_plugin_summary_get_selected_msg_list();
    gtk_widget_set_sensitive(widget, FALSE);
    if (mlist != NULL && g_slist_length(mlist) == 1){
      gtk_widget_set_sensitive(widget, TRUE);
    }
    if (mlist){
      g_slist_free(mlist);
    }
  }
}

static void activate_menu_cb(GtkMenuItem *menuitem, gpointer data)
{
  MsgInfo *msginfo = (MsgInfo*)data;
  g_print("menu activated %p\n", msginfo);
  if (msginfo!=NULL){
    gchar *file = NULL;
    file = myprocmsg_get_message_file_path(msginfo);
    if (!file){
    } else {
      g_print("msg file %s\n", file);
      int retval = 0;
      retval = test_filter(0, file);
      switch(retval){
      case 0:
        /* junk */
        g_print("junk: %s\n", file);
        break;
      case 1:
        /* clean */
        g_print("clean: %s\n", file);
        break;
      case 2:
        /* unknown */
        g_print("unknown: %s\n", file);
        break;
      case 127:
        /* error */
        g_print("error: %s\n", file);
        break;
      }
    }
  }
}

static void textview_menu_popup_cb(GObject *obj, GtkMenu *menu,
				   GtkTextView *textview,
				   const gchar *uri,
				   const gchar *selected_text,
				   MsgInfo *msginfo)
{
	GtkWidget *separator, *menuitem;

	g_print("test: %p: textview menu popup\n", obj);
	g_print("test: %p: uri: %s, text: %s\n", obj, uri ? uri : "(none)",
		selected_text ? selected_text : "(none)");
	g_print("test: %p: msg: %s\n", obj,
		msginfo && msginfo->subject ? msginfo->subject : "");

	separator = gtk_separator_menu_item_new();
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), separator);
	gtk_widget_show(separator);

	menuitem = gtk_menu_item_new_with_mnemonic(_("Show sylfilter status"));
	g_signal_connect(menuitem, "activate", G_CALLBACK(activate_menu_cb), msginfo);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), menuitem);
	gtk_widget_show(menuitem);
}

static void menu_selected_cb(void)
{
	gint sel;
	GSList *mlist;

	g_print("test: summary menu selected\n");
	sel = syl_plugin_summary_get_selection_type();
	mlist = syl_plugin_summary_get_selected_msg_list();
	g_print("test: selection type: %d\n", sel);
	g_print("test: number of selected summary message: %d\n",
		g_slist_length(mlist));
	g_slist_free(mlist);
}

static void messageview_show_cb(GObject *obj, gpointer msgview,
				MsgInfo *msginfo, gboolean all_headers)
{
	g_print("test: %p: messageview_show (%p), all_headers: %d: %s\n",
		obj, msgview, all_headers,
		msginfo && msginfo->subject ? msginfo->subject : "");
}

static void button_clicked(GtkWidget *widget, gpointer data)
{
	g_print("button_clicked\n");
	/* syl_plugin_app_will_exit(TRUE); */
}

static void create_window(void)
{
	GtkWidget *window;
	GtkWidget *button;

	g_print("creating window\n");

	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	button = gtk_button_new_with_label("Click this button");
	gtk_window_set_default_size(GTK_WINDOW(window), 400, 200);
	gtk_container_add(GTK_CONTAINER(window), button);
	g_signal_connect(G_OBJECT(button), "clicked",
			 G_CALLBACK(button_clicked), NULL);
	gtk_widget_show_all(window);
}

static void create_folderview_sub_widget(void)
{
	GtkWidget *vbox;
	GtkWidget *button;

	g_print("creating sub widget\n");

	vbox = gtk_vbox_new(FALSE, 2);
	button = gtk_button_new_with_label("Test");
	gtk_box_pack_start(GTK_BOX(vbox), button, FALSE, FALSE, 0);
	gtk_widget_show_all(vbox);
	syl_plugin_folderview_add_sub_widget(vbox);
}

static int test_filter(int mode, const char *file)
{
  XFilterManager *mgr;
  XMessageData *msgdata;
  XMessageData *resdata;
  XFilterResult *res;
  XFilterStatus status;
  int retval = 0;

  XFilterConstructorFunc ctors[] = {
    xfilter_textcontent_new,
    xfilter_blacklist_new,
    xfilter_whitelist_new,
    xfilter_wordsep_new,
    xfilter_ngram_new,
    xfilter_bayes_new,
    NULL
  };

  if (g_verbose) {
    printf("testing message file: %s\n", file);
  }
  mgr = xfilter_manager_new();
  xfilter_manager_add_filters(mgr, ctors);

  msgdata = xfilter_message_data_read_file(file, "message/rfc822");

  res = xfilter_manager_run(mgr, msgdata);
  if (g_verbose)
    xfilter_result_print(res);
  status = xfilter_result_get_status(res);
  if (status == XF_JUNK) {
    g_print("%s: This is a junk mail (prob: %f)\n", file, xfilter_result_get_probability(res));
    retval = 0;
  } else if (status == XF_UNCERTAIN) {
    g_print("%s: This mail could not be classified (prob: %f)\n", file, xfilter_result_get_probability(res));
    retval = 2;
  } else if (status == XF_UNSUPPORTED_TYPE || status == XF_ERROR) {
    g_print("%s: Error on testing mail\n", file);
    retval = 127;
  } else {
    g_print("%s: This is a clean mail (prob: %f)\n", file, xfilter_result_get_probability(res));
    retval = 1;
  }

  if (xfilter_get_debug_mode()) {
    resdata = xfilter_result_get_message_data(res);
    /*print_message_data(resdata);*/
  }

  xfilter_result_free(res);
  xfilter_message_data_free(msgdata);

  xfilter_manager_free(mgr);

  return retval;
}

static gchar *myprocmsg_get_message_file_path(MsgInfo *msginfo)
{
  gchar *path, *file = NULL;

  g_return_val_if_fail(msginfo != NULL, NULL);

  if (msginfo->encinfo && msginfo->encinfo->plaintext_file){
    file = g_strdup(msginfo->encinfo->plaintext_file);
  } else if (msginfo->file_path) {
    g_print("msginfo->file_path:%p\n", msginfo->file_path);
    return g_strdup(msginfo->file_path);
  } else {
    gchar nstr[16];
    path = folder_item_get_path(msginfo->folder);
    g_print("msginfo->folder:%s\n", path);
    g_print("msginfo->msgnum:%d\n", msginfo->msgnum);
    g_snprintf(nstr, 11, "%u", msginfo->msgnum);
    file = g_strconcat(path, G_DIR_SEPARATOR_S,
                       nstr, NULL);
    g_print("file:%s", file);
    g_free(path);
  }
  return file;
}
