/* vim: set cino= fo=croql sw=8 ts=8 sts=0 noet ai cin fdm=syntax : */

/*
 * Copyright (c) 2009 Ali Polatel <alip@exherbo.org>
 *
 * This file is part of the mpdcron mpd client. mpdcron is free software;
 * you can redistribute it and/or modify it under the terms of the GNU General
 * Public License version 2, as published by the Free Software Foundation.
 *
 * mpdcron is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 * Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "eugene-defs.h"

#include <stdlib.h>
#include <stdio.h>

#include <glib.h>
#include <mpd/client.h>

static int optv = 0;
static char *expr = NULL;

static GOptionEntry options[] = {
	{"verbose", 'v', 0, G_OPTION_ARG_NONE, &optv, "Be verbose", NULL},
	{"dbpath", 'd', 0, G_OPTION_ARG_FILENAME, &euconfig.dbpath, "Path to the database", NULL},
	{"expr", 'e', 0, G_OPTION_ARG_STRING, &expr, "Unkill songs matching the given expression", NULL},
	{ NULL, -1, 0, 0, NULL, NULL, NULL },
};

static int unkill_current(void)
{
	int ret;
	struct mpd_song *song;

	if ((song = load_current_song()) == NULL)
		return 1;
	ret = db_kill_song(euconfig.dbpath, song, false);
	mpd_song_free(song);
	return ret ? 0 : 1;
}

int cmd_unkill_song(int argc, char **argv)
{
	GOptionContext *ctx;
	GError *parse_err = NULL;

	ctx = g_option_context_new("");
	g_option_context_add_main_entries(ctx, options, "eugene-unkill");
	g_option_context_set_summary(ctx, "eugene-unkill-"VERSION GITHEAD" - Unkill song");
	g_option_context_set_description(ctx, ""
"Examples:\n"
"# Unkill the current playing song\n"
"$> eugene unkill\n"
"# Unkill all songs whose rating is more than 10\n"
"$> eugene unkill --expr \"rating > 10\""
"For more information about the expression syntax, see:\n"
"http://www.sqlite.org/lang_expr.html");
	if (!g_option_context_parse(ctx, &argc, &argv, &parse_err)) {
		g_printerr("Option parsing failed: %s\n", parse_err->message);
		g_error_free(parse_err);
		g_option_context_free(ctx);
		return -1;
	}
	g_option_context_free(ctx);

	if (optv)
		euconfig.verbosity = LOG_DEBUG;

	if (euconfig.dbpath == NULL)
		load_paths();

	if (!db_init(euconfig.dbpath))
		return -1;

	if (expr != NULL)
		return db_kill_song_expr(euconfig.dbpath, expr, false,
				(euconfig.verbosity > LOG_WARNING)) ? 0 : 1;
	else
		return unkill_current();
}
