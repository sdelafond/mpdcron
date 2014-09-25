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

#ifndef MPDCRON_GUARD_MODULE_H
#define MPDCRON_GUARD_MODULE_H 1

#include <stdbool.h>

#ifdef MPDCRON_MODULE
#define G_LOG_DOMAIN MPDCRON_MODULE
#endif /* MPDCRON_MODULE */

#include <glib.h>
#include <mpd/client.h>

enum mpdcron_init_retval {
	MPDCRON_INIT_SUCCESS = 0, /** Success */
	MPDCRON_INIT_FAILURE, /** Failure */
};

enum mpdcron_event_retval {
	MPDCRON_EVENT_SUCCESS = 0, /** Success **/
	MPDCRON_EVENT_RECONNECT, /** Schedule a reconnection to mpd server **/
	MPDCRON_EVENT_RECONNECT_NOW, /** Schedule a reconnection to mpd server immediately. **/
	MPDCRON_EVENT_UNLOAD, /** Unload the module **/
};

struct mpdcron_config {
	char *home_path;
	char *conf_path;
	char *pid_path;
	char *mod_path;
	const char *hostname;
	const char *port;
	const char *password;

	int no_daemon;
	int timeout;
	int reconnect;
	int killwait;
	int loglevel;

	enum mpd_idle idle;
};

struct mpdcron_module {
	/** Name of the module */
	const char *name;

	/** Initialization function */
	int (*init) (const struct mpdcron_config *, GKeyFile *);

	/** Cleanup function */
	void (*destroy) (void);

	/** Function for database event */
	int (*event_database) (const struct mpd_connection *conn, const struct mpd_stats *);

	/** Function for stored playlist event */
	int (*event_stored_playlist) (const struct mpd_connection *);

	/** Function for queue event */
	int (*event_queue) (const struct mpd_connection *);

	/** Function for player event */
	int (*event_player) (const struct mpd_connection *, const struct mpd_song *,
			const struct mpd_status *);

	/** Function for mixer event */
	int (*event_mixer) (const struct mpd_connection *, const struct mpd_status *);

	/** Function for output event */
	int (*event_output) (const struct mpd_connection *);

	/** Function for options event */
	int (*event_options) (const struct mpd_connection *, const struct mpd_status *);

	/** Function for update event */
	int (*event_update) (const struct mpd_connection *, const struct mpd_status *);
};

#ifndef MPDCRON_INTERNAL
extern struct mpdcron_module module;
#endif /* !MPDCRON_INTERNAL */

#endif /* !MPDCRON_GUARD_MODULE_H */
