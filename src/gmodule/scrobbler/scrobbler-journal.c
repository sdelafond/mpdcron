/* vim: set cino= fo=croql sw=8 ts=8 sts=0 noet cin fdm=syntax : */

/*
 * Copyright (c) 2009, 2010 Ali Polatel <alip@exherbo.org>
 * Based in part upon mpdscribble which is:
 *   Copyright (C) 2008-2009 The Music Player Daemon Project
 *   Copyright (C) 2005-2008 Kuno Woudt <kuno@frob.nl>
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

#include "scrobbler-defs.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <glib.h>

static int journal_file_empty;

static void journal_write_record(gpointer data, gpointer user_data)
{
	struct record *record = data;
	FILE *file = user_data;

	fprintf(file,
		"a = %s\nt = %s\nb = %s\nm = %s\n"
		"i = %s\nl = %i\no = %s\n\n", record->artist,
		record->track, record->album, record->mbid, record->time,
		record->length, record->source);
}

bool journal_write(const char *path, GQueue *queue)
{
	FILE *handle;

	if (g_queue_is_empty(queue) && journal_file_empty)
		return false;

	handle = fopen(path, "wb");
	if (!handle) {
		g_warning("Failed to save %s: %s\n",
				path, g_strerror(errno));
		return false;
	}

	g_queue_foreach(queue, journal_write_record, handle);

	fclose(handle);

	return true;
}

static void journal_commit_record(GQueue *queue, struct record *record)
{
	if (record->artist != NULL && record->track != NULL) {
		/* append record to the queue; reuse allocated strings */

		g_queue_push_tail(queue, g_memdup(record, sizeof(*record)));

		journal_file_empty = false;
	} else {
		/* free and clear the record, it was not used */

		record_deinit(record);
	}

	record_clear(record);
}

/* g_time_val_from_iso8601() was introduced in GLib 2.12 */
#if GLIB_CHECK_VERSION(2,12,0)

/**
 * Imports an old (protocol v1.2) timestamp, format "%Y-%m-%d
 * %H:%M:%S".
 */
static char *
import_old_timestamp(const char *p)
{
	char *q;
	bool success;
	GTimeVal time_val;

	if (strlen(p) <= 10 || p[10] != ' ')
		return NULL;

	g_debug("Importing time stamp '%s'", p);

	/* replace a space with 'T', as expected by
	   g_time_val_from_iso8601() */
	q = g_strdup(p);
	q[10] = 'T';

	success = g_time_val_from_iso8601(q, &time_val);
	g_free(q);
	if (!success) {
		g_debug("Import of '%s' failed", p);
		return NULL;
	}

	g_debug("'%s' -> %ld", p, time_val.tv_sec);
	return g_strdup_printf("%ld", time_val.tv_sec);
}

#endif

/**
 * Parses the time stamp.  If needed, converts the time stamp, and
 * returns an allocated string.
 */
static char *
parse_timestamp(const char *p)
{
#if GLIB_CHECK_VERSION(2,12,0)
	char *ret = import_old_timestamp(p);
	if (ret != NULL)
		return ret;
#endif

	return g_strdup(p);
}

void journal_read(const char *path, GQueue *queue)
{
	FILE *file;
	char line[1024];
	struct record record;

	journal_file_empty = true;

	file = fopen(path, "r");
	if (file == NULL) {
		if (errno != ENOENT)
			/* ENOENT is ignored silently, because the
			 * user might be starting mpdcron for the
			 * first time */
			g_warning("Failed to load %s: %s",
					path, g_strerror(errno));
		return;
	}

	record_clear(&record);

	while (fgets(line, sizeof(line), file) != NULL) {
		char *key, *value;

		key = g_strchug(line);
		if (*key == 0 || *key == '#')
			continue;

		value = strchr(key, '=');
		if (value == NULL || value == key)
			continue;

		*value++ = 0;

		key = g_strchomp(key);
		value = g_strstrip(value);

		if (!strcmp("a", key)) {
			journal_commit_record(queue, &record);
			record.artist = g_strdup(value);
		} else if (!strcmp("t", key))
			record.track = g_strdup(value);
		else if (!strcmp("b", key))
			record.album = g_strdup(value);
		else if (!strcmp("m", key))
			record.mbid = g_strdup(value);
		else if (!strcmp("i", key))
			record.time = parse_timestamp(value);
		else if (!strcmp("l", key))
			record.length = atoi(value);
		else if (strcmp("o", key) == 0 && value[0] == 'R')
			record.source = "R";
	}

	fclose(file);

	journal_commit_record(queue, &record);
}
