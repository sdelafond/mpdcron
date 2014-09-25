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

#include "cron-defs.h"

#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <glib.h>

#include <libdaemon/dfork.h>
#include <libdaemon/dlog.h>
#include <libdaemon/dpid.h>

#include <mpd/client.h>

GMainLoop *loop = NULL;
static GKeyFile *cfd = NULL;
static int optv, optk;

static GOptionEntry options[] = {
	{"version", 'V', 0, G_OPTION_ARG_NONE, &optv, "Display version", NULL},
	{"kill", 'k', 0, G_OPTION_ARG_NONE, &optk, "Kill daemon", NULL},
	{"no-daemon", 'n', 0, G_OPTION_ARG_NONE, &conf.no_daemon, "Don't detach from console", NULL},
	{NULL, 0, 0, 0, NULL, NULL, NULL},
};

static void
about(void)
{
	printf(PACKAGE"-"VERSION GITHEAD "\n");
}

static void
internal_cleanup(
#ifndef HAVE_GMODULE
		G_GNUC_UNUSED
#endif /* !HAVE_GMODULE */
		bool signaled)
{
#ifdef HAVE_GMODULE
	module_close(signaled ? 0 : 1);
#endif /* HAVE_GMODULE */
	conf_free();
	if (cfd != NULL) {
		g_key_file_free(cfd);
		cfd = NULL;
	}
	if (loop != NULL) {
		g_main_loop_quit(loop);
		g_main_loop_unref(loop);
		loop = NULL;
	}
}

static void
cleanup(void)
{
	internal_cleanup(false);
}

static void
sig_cleanup(int signum)
{
	struct sigaction action;

	internal_cleanup(true);

	sigaction(signum, NULL, &action);
	action.sa_handler = SIG_DFL;
	sigaction(signum, &action, NULL);
	raise(signum);
}

int
main(int argc, char **argv)
{
	int pid, ret;
	struct sigaction new_action, old_action;
	GOptionContext *ctx;
	GError *parse_err = NULL;

	daemon_pid_file_ident = daemon_log_ident = daemon_ident_from_argv0(argv[0]);
	daemon_pid_file_proc = conf_pid_file_proc;
	if (conf_init() < 0)
		return EXIT_FAILURE;

	ctx = g_option_context_new("");
	g_option_context_add_main_entries(ctx, options, PACKAGE);
	g_option_context_set_summary(ctx, PACKAGE"-"VERSION GITHEAD" - mpd cron daemon");

	if (!g_option_context_parse(ctx, &argc, &argv, &parse_err)) {
		g_printerr("option parsing failed: %s\n", parse_err->message);
		g_option_context_free(ctx);
		g_error_free(parse_err);
		return EXIT_FAILURE;
	}
	g_option_context_free(ctx);

	if (optv) {
		about();
		cleanup();
		return EXIT_SUCCESS;
	}

#ifdef DAEMON_SET_VERBOSITY_AVAILABLE
	if (conf.no_daemon)
		daemon_set_verbosity(LOG_DEBUG);
#endif /* DAEMON_SET_VERBOSITY_AVAILABLE */

	/* Version to environment variable */
	g_setenv("MPDCRON_PACKAGE", PACKAGE, 1);
	g_setenv("MPDCRON_VERSION", VERSION, 1);
	g_setenv("MPDCRON_GITHEAD", GITHEAD, 1);

	/* Command line options to environment variables */
	if (conf.no_daemon)
		g_unsetenv("MCOPT_DAEMONIZE");
	else
		g_setenv("MCOPT_DAEMONIZE", "1", 1);

	/* Important! Parse configuration file before killing the daemon
	 * because the configuration file has a pidfile and killwait option.
	 */
	cfd = g_key_file_new();
	if (keyfile_load(&cfd) < 0) {
		cleanup();
		return EXIT_FAILURE;
	}

	if (optk) {
		if (daemon_pid_file_kill_wait(SIGINT, conf.killwait) < 0) {
			g_warning("Failed to kill daemon: %s", strerror(errno));
			cleanup();
			return EXIT_FAILURE;
		}
		daemon_pid_file_remove();
		cleanup();
		return EXIT_SUCCESS;
	}

	/* Logging */
	g_log_set_default_handler(log_handler, GINT_TO_POINTER(conf.no_daemon ? 5 : conf.loglevel));

	/* Signal handling */
	new_action.sa_handler = sig_cleanup;
	sigemptyset(&new_action.sa_mask);
	new_action.sa_flags = 0;

#define HANDLE_SIGNAL(sig)					\
	do {							\
		sigaction((sig), NULL, &old_action);		\
		if (old_action.sa_handler != SIG_IGN)		\
			sigaction((sig), &new_action, NULL);	\
	} while (0)

	HANDLE_SIGNAL(SIGABRT);
	HANDLE_SIGNAL(SIGSEGV);
	HANDLE_SIGNAL(SIGINT);
	HANDLE_SIGNAL(SIGTERM);

#undef HANDLE_SIGNAL

	if (conf.no_daemon) {
		/* Create the main loop */
		loop = g_main_loop_new(NULL, FALSE);

#ifdef HAVE_GMODULE
		/* Load modules which may add initial events */
		keyfile_load_modules(&cfd);
#endif /* HAVE_GMODULE */
		g_key_file_free(cfd);
		cfd = NULL;

		/* Add default initial events */
		loop_connect();

		/* Run the main loop */
		g_main_loop_run(loop);
		cleanup();
		return EXIT_SUCCESS;
	}

	/* Daemonize */
	if ((pid = daemon_pid_file_is_running()) > 0) {
		g_critical("Daemon already running on PID %u", pid);
		return EXIT_FAILURE;
	}

	daemon_retval_init();
	pid = daemon_fork();
	if (pid < 0) {
		g_critical("Failed to fork: %s", strerror(errno));
		daemon_retval_done();
		return EXIT_FAILURE;
	}
	else if (pid != 0) { /* Parent */
		cleanup();

		if ((ret = daemon_retval_wait(2)) < 0) {
			g_critical("Could not receive return value from daemon process: %s",
				strerror(errno));
			return 255;
		}

		if (ret != 0)
			g_critical("Daemon returned %i as return value", ret);
		else
			g_critical("Daemon returned %i as return value", ret);
		return ret;
	}
	else { /* Daemon */
		if (daemon_close_all(-1) < 0) {
			g_critical("Failed to close all file descriptors: %s",
					strerror(errno));
			daemon_retval_send(1);
			return EXIT_FAILURE;
		}

		if (daemon_pid_file_create() < 0) {
			g_critical("Failed to create PID file: %s",
					strerror(errno));
			daemon_retval_send(2);
			return EXIT_FAILURE;
		}

		/* Send OK to parent process */
		daemon_retval_send(0);

		/* Create the main loop */
		loop = g_main_loop_new(NULL, FALSE);

#ifdef HAVE_GMODULE
		/* Load modules which may add initial events */
		keyfile_load_modules(&cfd);
#endif /* HAVE_GMODULE */
		g_key_file_free(cfd);
		cfd = NULL;

		/* Add default initial events */
		loop_connect();

		/* Run the main loop */
		g_main_loop_run(loop);
		cleanup();
		return EXIT_SUCCESS;
	}
	return EXIT_SUCCESS;
}
