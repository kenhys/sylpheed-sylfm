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

#ifndef __SYLFM_H__
#define __SYLFM_H__

#include <glib.h>
#include <glib/gi18n-lib.h>
#include <locale.h>

#define _(String) dgettext("ghostbiff", String)
#define N_(String) gettext_noop(String)
#define gettext_noop(String) (String)

#define SYLFM "sylfm"
#define SYLFMRC "sylfmrc"

#define PLUGIN_NAME N_("SylFm - SylFilter management plug-in for Sylpheed")
#define PLUGIN_DESC N_("SylFilter management plug-in for Sylpheed")

#endif /* __SYLFM_H__ */
