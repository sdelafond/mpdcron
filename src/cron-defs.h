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

#ifndef MPDCRON_GUARD_CRON_DEFS_H
#define MPDCRON_GUARD_CRON_DEFS_H 1

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include <glib.h>
#include <libdaemon/dlog.h>
#include <mpd/client.h>

/* Configuration */
#define ENV_HOME_DIR		"MPDCRON_DIR"
#define ENV_MPD_HOST		"MPD_HOST"
#define ENV_MPD_PORT		"MPD_PORT"
#define ENV_MPD_PASSWORD	"MPD_PASSWORD"

#define DOT_MPDCRON			"." PACKAGE
#define DOT_HOOKS			"hooks"

#define DEFAULT_PID_KILL_WAIT	3
#define DEFAULT_MPD_RECONNECT	5
#define DEFAULT_MPD_TIMEOUT	0

#define DEFAULT_DATE_FORMAT		"%Y-%m-%d %H-%M-%S %Z"
#define DEFAULT_DATE_FORMAT_SIZE	64

extern char *home_path;
extern char *conf_path;
extern char *pid_path;

extern const char *hostname;
extern const char *port;
extern const char *password;

extern int timeout;
extern int reconnect;
extern int killwait;
extern enum mpd_idle idle;

extern int optnd;
extern GMainLoop *loop;

#define crlog(level, ...) daemon_log(level, __VA_ARGS__)
#define crlogv(level, ...)				\
	do {						\
		if (optnd)				\
			daemon_log(level, __VA_ARGS__);	\
	} while(0)

extern const char *conf_pid_file_proc(void);
extern int conf_init(void);
extern void conf_free(void);
extern void env_clearenv(void);
extern int env_list_all_meta(struct mpd_connection *conn);
extern int env_list_queue_meta(struct mpd_connection *conn);
extern int env_stats(struct mpd_connection *conn);
extern int env_status(struct mpd_connection *conn);
extern int env_status_currentsong(struct mpd_connection *conn);
extern int env_outputs(struct mpd_connection *conn);
extern int event_run(struct mpd_connection *conn, enum mpd_idle event);
extern int hooker_run_hook(const char *name);
extern int keyfile_load(void);
extern void loop_connect(void);
extern void loop_disconnect(void);

#endif /* !MPDCRON_GUARD_CRON_DEFS_H */