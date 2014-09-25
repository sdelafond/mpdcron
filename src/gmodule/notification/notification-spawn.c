/* vim: set cino= fo=croql sw=8 ts=8 sts=0 noet cin fdm=syntax : */

/*
 * Copyright (c) 2009, 2010 Ali Polatel <alip@exherbo.org>
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

#include "notification-defs.h"

#include <stdlib.h>
#include <string.h>

#include <glib.h>

void
notify_send(const char *icon, const char *summary, const char *body)
{
	int i, j, len;
	char **myargv;
	GError *error;

	i = 0;
	len = 8 + (file_config.hints ? g_strv_length(file_config.hints) : 0);
	myargv = g_malloc0(sizeof(char *) * len);
	myargv[i++] = g_strdup("notify-send");
	if (file_config.urgency != NULL)
		myargv[i++] = g_strdup_printf("--urgency=%s", file_config.urgency);
	if (file_config.timeout != NULL)
		myargv[i++] = g_strdup_printf("--expire-time=%s", file_config.timeout);
	if (file_config.type != NULL)
		myargv[i++] = g_strdup_printf("--category=%s", file_config.type);
	if (icon != NULL)
		myargv[i++] = g_strdup_printf("--icon=%s", icon);
	myargv[i++] = g_strdup(summary);
	myargv[i++] = g_strdup(body);

	if (file_config.hints != NULL) {
		for (j = 0; file_config.hints[j] != NULL; j++)
			myargv[i++] = g_strdup_printf("--hint=%s", file_config.hints[j]);
	}
	myargv[i] = NULL;

	error = NULL;
	if (!g_spawn_async(NULL, myargv, NULL, G_SPAWN_SEARCH_PATH, NULL, NULL, NULL, &error)) {
			g_warning("Failed to execute notify-send: %s",
					error->message);
			g_error_free(error);
	}

	for (; i >= 0; i--)
		g_free(myargv[i]);
	g_free(myargv);
}
