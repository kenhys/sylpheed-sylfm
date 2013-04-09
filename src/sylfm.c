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
#include "alertpanel.h"

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

#define PLUGIN_NAME N_("SylFm - SylFilter management plug-in for Sylpheed")
#define PLUGIN_DESC N_("SylFilter management plug-in for Sylpheed")

static SylPluginInfo info = {
	N_(PLUGIN_NAME),
	VERSION,
	"HAYASHI Kentaro",
	N_(PLUGIN_DESC),
};

gulong app_exit_handler_id = 0;
static int g_verbose = 0;
static SylFmOption g_opt;

void plugin_load(void)
{

  syl_init_gettext(SYLFM, "lib/locale");

  info.name = g_strdup(_(PLUGIN_NAME));
  info.description = g_strdup(_(PLUGIN_DESC));

  g_print("[PLUGIN] sylfm plug-in loaded!\n");

#if 0
  syl_plugin_add_menuitem("/Tools", NULL, NULL, NULL);
  syl_plugin_add_menuitem("/Tools", _("SylFm plugin settings [sylfm]"), exec_sylfm_menu_cb, NULL);
#endif
  
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
    exec_sylfm_popup_menu_cb(msginfo);
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

  if (g_slist_length(mlist) == 1){
    MsgInfo* msginfo = (MsgInfo*)g_slist_nth_data(mlist, 0);
    exec_sylfm_popup_menu_cb(msginfo);
  }
  g_slist_free(mlist);
}

static void messageview_show_cb(GObject *obj, gpointer msgview,
				MsgInfo *msginfo, gboolean all_headers)
{
	g_print("test: %p: messageview_show (%p), all_headers: %d: %s\n",
		obj, msgview, all_headers,
		msginfo && msginfo->subject ? msginfo->subject : "");
}

static void exec_sylfm_menu_cb(void)
{
#if 0
  /* show modal dialog */
  GtkWidget *vbox;
  GtkWidget *confirm_area;
  GtkWidget *ok_btn;
  GtkWidget *cancel_btn;

  g_opt.window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_container_set_border_width(GTK_CONTAINER(window), 8);
  gtk_window_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
  gtk_window_set_modal(GTK_WINDOW(window), TRUE);
  gtk_window_set_policy(GTK_WINDOW(window), FALSE, TRUE, FALSE);
  gtk_window_set_default_size(GTK_WINDOW(window), 300, 100);
  gtk_widget_realize(window);

  vbox = gtk_vbox_new(FALSE, 6);
  gtk_widget_show(vbox);
  gtk_container_add(GTK_CONTAINER(window), vbox);

  /* notebook */ 
  GtkWidget *notebook = gtk_notebook_new();
  /* main tab */
  create_config_main_page(notebook, g_opt.rcfile);
  /* about, copyright tab */
  create_config_about_page(notebook, g_opt.rcfile);

  gtk_widget_show(notebook);
  gtk_box_pack_start(GTK_BOX(vbox), notebook, TRUE, TRUE, 0);

  confirm_area = gtk_hbutton_box_new();
  gtk_button_box_set_layout(GTK_BUTTON_BOX(confirm_area), GTK_BUTTONBOX_END);
  gtk_box_set_spacing(GTK_BOX(confirm_area), 6);


  ok_btn = gtk_button_new_from_stock(GTK_STOCK_OK);
  GTK_WIDGET_SET_FLAGS(ok_btn, GTK_CAN_DEFAULT);
  gtk_box_pack_start(GTK_BOX(confirm_area), ok_btn, FALSE, FALSE, 0);
  gtk_widget_show(ok_btn);

  gtk_widget_show(confirm_area);
	
  gtk_box_pack_end(GTK_BOX(vbox), confirm_area, FALSE, FALSE, 0);
  gtk_widget_grab_default(ok_btn);

  gtk_window_set_title(GTK_WINDOW(window), _("SylFm Settings"));

  g_signal_connect(G_OBJECT(ok_btn), "clicked",
                   G_CALLBACK(prefs_ok_cb), window);

  /* startup settings */


      
  /* load settings */
  if (g_key_file_load_from_file(g_opt.rcfile, g_opt.rcpath, G_KEY_FILE_KEEP_COMMENTS, NULL)){
    gboolean status=g_key_file_get_boolean(g_opt.rcfile, GHOSTBIFF, "startup", NULL);
    debug_print("startup:%s\n", status ? "TRUE" : "FALSE");
    if (status!=FALSE){
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(g_opt.chk_startup), TRUE);
    }

    if (g_aquestalk2da == NULL || g_aqkanji2koe == NULL){
      g_opt.enable_aquest=FALSE;
      /* disable check */
      gtk_widget_set_sensitive(GTK_WIDGET(g_opt.chk_aquest), FALSE);
    } else {
      g_opt.enable_aquest=TRUE;
    }

    status=g_key_file_get_boolean(g_opt.rcfile, GHOSTBIFF, "aquest", NULL);
    debug_print("aquest:%s\n", status ? "TRUE" : "FALSE");

    if (status!=FALSE){
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(g_opt.chk_aquest), TRUE);
    }

    gchar *buf = g_key_file_get_string(g_opt.rcfile, GHOSTBIFF, "aq_dic_path", NULL);
    if (buf != NULL){
      gtk_entry_set_text(GTK_ENTRY(g_opt.aq_dic_entry), buf);
    }

    buf = g_key_file_get_string(g_opt.rcfile, GHOSTBIFF, "phont_path", NULL);
    if (buf != NULL){
      gtk_entry_set_text(GTK_ENTRY(g_opt.phont_entry), buf);

      GDir* gdir = g_dir_open(buf, 0, NULL);
      GList* items = NULL;
      const gchar *file =NULL;
      do {
        file = g_dir_read_name(gdir);
        if (file!=NULL){
          debug_print("%s\n", file);
          items = g_list_append(items, g_strdup(file));
        }
      } while (file!=NULL);
    
      gtk_combo_set_popdown_strings(GTK_COMBO(g_opt.phont_cmb), items);
    }

    buf = g_key_file_get_string(g_opt.rcfile, GHOSTBIFF, "phont", NULL);
    if (buf != NULL){
      gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(g_opt.phont_cmb)->entry), buf);
    }

   /*    gchar *to=g_key_file_get_string (g_opt.rcfile, GHOSTBIFF, "to", NULL);
    gtk_entry_set_text(GTK_ENTRY(g_address), to);
    */
  }
  gtk_notebook_set_current_page(GTK_NOTEBOOK(notebook), 0);

  gtk_widget_show(window);
#endif
}


static void exec_sylfm_popup_menu_cb(MsgInfo *msginfo)
{
  /* show modal dialog */
  GtkWidget *vbox;
  GtkWidget *confirm_area;
  GtkWidget *ok_btn;
  GtkWidget *mainwin;

  mainwin = syl_plugin_main_window_get();
  g_opt.window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  g_print("[PLUGIN] window %p\n", g_opt.window);
  gtk_container_set_border_width(GTK_CONTAINER(g_opt.window), 8);
  gtk_window_position(GTK_WINDOW(g_opt.window), GTK_WIN_POS_CENTER);
  gtk_window_set_modal(GTK_WINDOW(g_opt.window), TRUE);
  gtk_window_set_transient_for(GTK_WINDOW(g_opt.window), mainwin);
  gtk_window_set_policy(GTK_WINDOW(g_opt.window), FALSE, TRUE, FALSE);
  gtk_window_set_default_size(GTK_WINDOW(g_opt.window), 300, 200);
  gtk_widget_realize(g_opt.window);

  vbox = gtk_vbox_new(FALSE, 6);
  gtk_widget_show(vbox);
  gtk_container_add(GTK_CONTAINER(g_opt.window), vbox);

  /* main tab */
  GtkWidget *frame = gtk_frame_new(_("Sylfilter Result:"));
  GtkWidget *label = NULL;
  gchar *file = NULL;
  file = myprocmsg_get_message_file_path(msginfo);
  if (!file){
  } else {
    xfilter_init(XF_APP_MODE_EXT_LIBSYLPH);

    xfilter_kvs_sqlite_set_engine();

    gchar *dbpath = xfilter_utils_get_default_base_dir();
    xfilter_utils_set_base_dir(dbpath);

    if (xfilter_bayes_db_init(dbpath) < 0) {
      g_print("[PLUGIN] Database initialization error.\n");
      xfilter_done();
      return;
    }

    g_print("msg file %s\n", file);
    int retval = 0;
    retval = test_filter(0, file);
    switch(retval){
    case 0:
      /* junk */
      label = gtk_label_new(_("junk"));
      g_print("junk: %s\n", file);
      break;
    case 1:
      /* clean */
      label = gtk_label_new(_("clean"));
      g_print("clean: %s\n", file);
      break;
    case 2:
      /* unknown */
      label = gtk_label_new(_("could not be classified"));
      g_print("unknown: %s\n", file);
      break;
    case 127:
      /* error */
      label = gtk_label_new(_("error"));
      g_print("error: %s\n", file);
      break;
    }
    xfilter_bayes_db_done();
    xfilter_done();
  }
  gtk_container_add(GTK_CONTAINER(frame), label);
  
  gtk_box_pack_start(GTK_BOX(vbox), frame, TRUE, TRUE, 0);

  GtkWidget *frame2 = gtk_frame_new(_("Sylfilter by hand:"));

  GtkWidget *hbox = gtk_hbox_new (TRUE, 2);
  g_opt.junk_radio = gtk_radio_button_new_with_label(NULL, _("junk mail"));
  g_opt.clear_radio = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON (g_opt.junk_radio),
                                                                  _("clean mail"));
  GtkWidget *apply_btn = gtk_button_new_from_stock(GTK_STOCK_APPLY);
  /* Pack them into a box, then show all the widgets */
  gtk_box_pack_start (GTK_BOX (hbox), g_opt.junk_radio, TRUE, TRUE, 2);
  gtk_box_pack_start (GTK_BOX (hbox), g_opt.clear_radio, TRUE, TRUE, 2);
  gtk_box_pack_start (GTK_BOX (hbox), apply_btn, TRUE, TRUE, 2);
  gtk_container_add (GTK_CONTAINER (frame2), hbox);
   
  g_signal_connect (apply_btn, "clicked",
                    G_CALLBACK (apply_sylfilter_cb), msginfo);

  gtk_box_pack_start(GTK_BOX(vbox), frame2, FALSE, FALSE, 0);

  confirm_area = gtk_hbutton_box_new();
  gtk_button_box_set_layout(GTK_BUTTON_BOX(confirm_area), GTK_BUTTONBOX_END);
  gtk_box_set_spacing(GTK_BOX(confirm_area), 6);


  ok_btn = gtk_button_new_from_stock(GTK_STOCK_OK);
  GTK_WIDGET_SET_FLAGS(ok_btn, GTK_CAN_DEFAULT);
  gtk_box_pack_start(GTK_BOX(confirm_area), ok_btn, FALSE, FALSE, 0);
  gtk_widget_show(ok_btn);

  gtk_widget_show(confirm_area);
	
  gtk_box_pack_end(GTK_BOX(vbox), confirm_area, FALSE, FALSE, 0);
  gtk_widget_grab_default(ok_btn);

  gtk_window_set_title(GTK_WINDOW(g_opt.window), _("SylFm Settings"));

  g_signal_connect(G_OBJECT(ok_btn), "clicked",
                   G_CALLBACK(popup_ok_cb), g_opt.window);

  gtk_widget_show_all(g_opt.window);
}

static void prefs_ok_cb(GtkWidget *widget, gpointer data)
{
  gtk_widget_destroy(GTK_WIDGET(data));
}

static void popup_ok_cb(GtkWidget *widget, gpointer data)
{
  gtk_widget_destroy(GTK_WIDGET(data));
}

static void apply_sylfilter_cb( GtkWidget *widget,
                                gpointer   data )
{
  MsgInfo *msginfo = (MsgInfo*)data;

  g_print("[PLUGIN] widget %p\n", widget);
  gchar *file = NULL;
  file = myprocmsg_get_message_file_path(msginfo);
  if (!file){
  } else {
    g_print("msg file %s\n", file);
    int retval = 0;
    gboolean bjunk = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(g_opt.junk_radio));
    xfilter_init(XF_APP_MODE_EXT_LIBSYLPH);

    xfilter_kvs_sqlite_set_engine();

    gchar *dbpath = xfilter_utils_get_default_base_dir();
    xfilter_utils_set_base_dir(dbpath);

    if (xfilter_bayes_db_init(dbpath) < 0) {
      g_print("[PLUGIN] Database initialization error.\n");
      xfilter_done();
      return;
    }

    if (bjunk != FALSE){
      g_print("MODE_LEARN_JUNK file %s\n", file);
      retval = learn_filter(MODE_LEARN_JUNK, file);
    } else {
      g_print("MODE_LEARN_CLEAN file %s\n", file);
      retval = learn_filter(MODE_LEARN_CLEAN, file);
    }
    if (retval != 0) {
      /* error */
      if (bjunk != FALSE){
        syl_plugin_alertpanel_message(_("SylFm"), _("learn junk mail error"), ALERT_ERROR);
      } else {
        syl_plugin_alertpanel_message(_("SylFm"), _("learn clean mail error"), ALERT_ERROR);
      }
      g_print("error: %s\n", file);
    } else {
      if (bjunk != FALSE){
        syl_plugin_alertpanel_message(_("SylFm"), _("learn junk mail"), ALERT_NOTICE);
      } else {
        syl_plugin_alertpanel_message(_("SylFm"), _("learn clean mail"), ALERT_NOTICE);
      }
      g_print("ok: %s\n", file);
    }
    xfilter_bayes_db_done();
    xfilter_done();

    gtk_widget_destroy(g_opt.window);
  }
  /**/
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

static int learn_filter(int mode, const char *file)
{
  XFilterManager *mgr;
  XMessageData *msgdata;
  XMessageData *resdata;
  XFilterResult *res;
  XFilterStatus status;
  int retval = 0;

  static XFilterConstructorFunc learn_junk_ctors[] = {
    xfilter_textcontent_new,
    xfilter_wordsep_new,
    xfilter_ngram_new,
    xfilter_bayes_learn_junk_new,
    NULL
  };

  static XFilterConstructorFunc learn_nojunk_ctors[] = {
    xfilter_textcontent_new,
    xfilter_wordsep_new,
    xfilter_ngram_new,
    xfilter_bayes_learn_nojunk_new,
    NULL
  };

  static XFilterConstructorFunc unlearn_junk_ctors[] = {
    xfilter_textcontent_new,
    xfilter_wordsep_new,
    xfilter_ngram_new,
    xfilter_bayes_unlearn_junk_new,
    NULL
  };

  static XFilterConstructorFunc unlearn_nojunk_ctors[] = {
    xfilter_textcontent_new,
    xfilter_wordsep_new,
    xfilter_ngram_new,
    xfilter_bayes_unlearn_nojunk_new,
    NULL
  };

  if (g_verbose)
    printf("learning message file: %s\n", file);

  mgr = xfilter_manager_new();

  switch (mode) {
  case MODE_LEARN_JUNK:
    xfilter_manager_add_filters(mgr, learn_junk_ctors);
    break;
  case MODE_LEARN_CLEAN:
    xfilter_manager_add_filters(mgr, learn_nojunk_ctors);
    break;
  case MODE_UNLEARN_JUNK:
    xfilter_manager_add_filters(mgr, unlearn_junk_ctors);
    break;
  case MODE_UNLEARN_CLEAN:
    xfilter_manager_add_filters(mgr, unlearn_nojunk_ctors);
    break;
  default:
    fprintf(stderr, "Internal error: invalid learn mode\n");
    xfilter_manager_free(mgr);
    return 127;
  }

  msgdata = xfilter_message_data_read_file(file, "message/rfc822");

  res = xfilter_manager_run(mgr, msgdata);
  if (g_verbose)
    xfilter_result_print(res);
  status = xfilter_result_get_status(res);
  if (status == XF_UNSUPPORTED_TYPE || status == XF_ERROR) {
    fprintf(stderr, "%s: Error on learning mail\n", file);
    retval = 127;
  }

  if (xfilter_get_debug_mode()) {
    resdata = xfilter_result_get_message_data(res);
#if 0
    print_message_data(resdata);
#endif
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
