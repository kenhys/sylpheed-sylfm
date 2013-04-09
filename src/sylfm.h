/*
 * SylFm -- SylFilter management plug-in for Sylpheed
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

#ifndef __SYLFM_H__
#define __SYLFM_H__

#include <glib.h>
#include <glib/gi18n-lib.h>
#include <locale.h>

#define _(String) dgettext("sylfm", String)
#define N_(String) gettext_noop(String)
#define gettext_noop(String) (String)

#define SYLFM "sylfm"
#define SYLFMRC "sylfmrc"

#define PLUGIN_NAME N_("SylFm - SylFilter management plug-in for Sylpheed")
#define PLUGIN_DESC N_("SylFilter management plug-in for Sylpheed")

struct _SylFmOption {
  /* full path to ghostbiffrc*/
  gchar *rcpath;
  /* rcfile */
  GKeyFile *rcfile;

  GtkWidget *window;
  /**/
  GtkWidget *junk_radio;
  GtkWidget *clear_radio;
};

enum {
	MODE_TEST_JUNK,
	MODE_LEARN_JUNK,
	MODE_LEARN_CLEAN,
	MODE_UNLEARN_JUNK,
	MODE_UNLEARN_CLEAN,
	MODE_SHOW_STATUS
};

typedef struct _SylFmOption SylFmOption;

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

static void create_folderview_sub_widget(void);
static void exec_sylfm_popup_menu_cb(MsgInfo *msginfo);
static int test_filter(int mode, const char *file);
static int learn_filter(int mode, const char *file);
static gchar *myprocmsg_get_message_file_path(MsgInfo *msginfo);
static void prefs_ok_cb(GtkWidget *widget, gpointer data);
static void popup_ok_cb(GtkWidget *widget, gpointer data);
static void apply_sylfilter_cb( GtkWidget *widget, gpointer   data);

#endif /* __SYLFM_H__ */
