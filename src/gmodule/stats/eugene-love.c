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

#include "eugene-defs.h"

#include <stdio.h>
#include <stdlib.h>

#include <glib.h>

static int
love_artist(struct mpdcron_connection *conn, bool love, const char *expr)
{
	int changes;
	char *esc_artist, *myexpr;
	struct mpd_song *song;

	if (expr != NULL) {
		if (!mpdcron_love_artist_expr(conn, love, expr, &changes)) {
			eulog(LOG_ERR, "Failed to %s artist: %s",
					love ? "love" : "hate",
					conn->error->message);
			return 1;
		}
	}
	else {
		if ((song = load_current_song()) == NULL)
			return 1;
		else if (mpd_song_get_tag(song, MPD_TAG_ARTIST, 0) == NULL) {
			eulog(LOG_ERR, "Current playing song has no artist tag!");
			mpd_song_free(song);
			return 1;
		}

		esc_artist = quote(mpd_song_get_tag(song, MPD_TAG_ARTIST, 0));
		myexpr = g_strdup_printf("name=%s", esc_artist);
		g_free(esc_artist);
		mpd_song_free(song);

		if (!mpdcron_love_artist_expr(conn, love, myexpr, &changes)) {
			eulog(LOG_ERR, "Failed to %s current playing artist: %s",
					love ? "love" : "hate",
					conn->error->message);
			g_free(myexpr);
			return 1;
		}
		g_free(myexpr);
	}
	printf("Modified %d entries\n", changes);
	return 0;
}

static int
love_album(struct mpdcron_connection *conn, bool love, const char *expr)
{
	int changes;
	char *esc_album, *myexpr;
	struct mpd_song *song;

	if (expr != NULL) {
		if (!mpdcron_love_album_expr(conn, love, expr, &changes)) {
			eulog(LOG_ERR, "Failed to %s album: %s",
					love ? "love" : "hate",
					conn->error->message);
			return 1;
		}
	}
	else {
		if ((song = load_current_song()) == NULL)
			return 1;
		else if (mpd_song_get_tag(song, MPD_TAG_ALBUM, 0) == NULL) {
			eulog(LOG_ERR, "Current playing song has no album tag!");
			mpd_song_free(song);
			return 1;
		}

		esc_album = quote(mpd_song_get_tag(song, MPD_TAG_ALBUM, 0));
		myexpr = g_strdup_printf("name=%s", esc_album);
		g_free(esc_album);
		mpd_song_free(song);

		if (!mpdcron_love_album_expr(conn, love, myexpr, &changes)) {
			eulog(LOG_ERR, "Failed to %s current playing album: %s",
					love ? "love" : "hate",
					conn->error->message);
			g_free(myexpr);
			return 1;
		}
		g_free(myexpr);
	}
	printf("Modified %d entries\n", changes);
	return 0;
}

static int
love_genre(struct mpdcron_connection *conn, bool love, const char *expr)
{
	int changes;
	char *esc_genre, *myexpr;
	struct mpd_song *song;

	if (expr != NULL) {
		if (!mpdcron_love_genre_expr(conn, love, expr, &changes)) {
			eulog(LOG_ERR, "Failed to %s genre: %s",
					love ? "love" : "hate",
					conn->error->message);
			return 1;
		}
	}
	else {
		if ((song = load_current_song()) == NULL)
			return 1;
		else if (mpd_song_get_tag(song, MPD_TAG_GENRE, 0) == NULL) {
			eulog(LOG_ERR, "Current playing song has no genre tag!");
			mpd_song_free(song);
			return 1;
		}

		esc_genre = quote(mpd_song_get_tag(song, MPD_TAG_GENRE, 0));
		myexpr = g_strdup_printf("name=%s", esc_genre);
		g_free(esc_genre);
		mpd_song_free(song);

		if (!mpdcron_love_genre_expr(conn, love, myexpr, &changes)) {
			eulog(LOG_ERR, "Failed to %s current playing genre: %s",
					love ? "love" : "hate",
					conn->error->message);
			g_free(myexpr);
			return 1;
		}
		g_free(myexpr);
	}
	printf("Modified %d entries\n", changes);
	return 0;
}

static int
love_song(struct mpdcron_connection *conn, bool love, const char *expr)
{
	int changes;
	char *esc_uri, *myexpr;
	struct mpd_song *song;

	if (expr != NULL) {
		if (!mpdcron_love_expr(conn, love, expr, &changes)) {
			eulog(LOG_ERR, "Failed to %s song: %s",
					love ? "love" : "hate",
					conn->error->message);
			return 1;
		}
	}
	else {
		if ((song = load_current_song()) == NULL)
			return 1;

		esc_uri = quote(mpd_song_get_uri(song));
		myexpr = g_strdup_printf("uri=%s", esc_uri);
		g_free(esc_uri);
		mpd_song_free(song);

		if (!mpdcron_love_expr(conn, love, myexpr, &changes)) {
			eulog(LOG_ERR, "Failed to %s current playing song: %s",
					love ? "love" : "hate",
					conn->error->message);
			g_free(myexpr);
			return 1;
		}
		g_free(myexpr);
	}
	printf("Modified %d entries\n", changes);
	return 0;
}

static int
cmd_love_internal(bool love, const char *expr, bool artist, bool album,
		bool genre)
{
	int port, ret;
	const char *hostname, *password;
	struct mpdcron_connection *conn;

	if ((artist && album && genre) ||
			(artist && album) ||
			(artist && genre) ||
			(album && genre)) {
		g_printerr("--artist, --album and --genre options are mutually exclusive\n");
		return 1;
	}

	hostname = g_getenv(ENV_MPDCRON_HOST)
		? g_getenv(ENV_MPDCRON_HOST)
		: DEFAULT_HOSTNAME;
	port = g_getenv(ENV_MPDCRON_PORT)
		? atoi(g_getenv(ENV_MPDCRON_PORT))
		: DEFAULT_PORT;
	password = g_getenv(ENV_MPDCRON_PASSWORD);

	conn = mpdcron_connection_new(hostname, port);
	if (conn->error != NULL) {
		eulog(LOG_ERR, "Failed to connect: %s", conn->error->message);
		mpdcron_connection_free(conn);
		return 1;
	}

	if (password != NULL) {
		if (!mpdcron_password(conn, password)) {
			eulog(LOG_ERR, "Authentication failed: %s", conn->error->message);
			mpdcron_connection_free(conn);
			return 1;
		}
	}

	if (artist)
		ret = love_artist(conn, love, expr);
	else if (album)
		ret = love_album(conn, love, expr);
	else if (genre)
		ret = love_genre(conn, love, expr);
	else
		ret = love_song(conn, love, expr);
	mpdcron_connection_free(conn);
	return ret;
}

int
cmd_hate(int argc, char **argv)
{
	int opta = 0, optA = 0, optg = 0, ret;
	GError *error = NULL;
	GOptionEntry options[] = {
		{"artist", 'a', 0, G_OPTION_ARG_NONE, &opta,
			"Hate artists instead of songs", NULL},
		{"album", 'A', 0, G_OPTION_ARG_NONE, &optA,
			"Hate albums instead of songs", NULL},
		{"genre", 'g', 0, G_OPTION_ARG_NONE, &optA,
			"Hate genres instead of songs", NULL},
		{ NULL, 0, 0, 0, NULL, NULL, NULL },
	};
	GOptionContext *ctx;

	ctx = g_option_context_new("[EXPRESSION]");
	g_option_context_add_main_entries(ctx, options, "eugene-hate");
	g_option_context_set_summary(ctx, "eugene-hate-"VERSION GITHEAD" - Hate song/artist/album/genre");
	g_option_context_set_description(ctx, ""
"By default this command works on the current playing song.\n"
"For more information about the expression syntax, see:\n"
"http://www.sqlite.org/lang_expr.html");
	if (!g_option_context_parse(ctx, &argc, &argv, &error)) {
		g_printerr("Option parsing failed: %s\n", error->message);
		g_error_free(error);
		g_option_context_free(ctx);
		return 1;
	}
	g_option_context_free(ctx);

	if (argc > 1)
		ret = cmd_love_internal(false, argv[1], opta, optA, optg);
	else
		ret = cmd_love_internal(false, NULL, opta, optA, optg);
	return ret;
}

int
cmd_love(int argc, char **argv)
{
	int opta = 0, optA = 0, optg = 0, ret;
	GError *error = NULL;
	GOptionEntry options[] = {
		{"artist", 'a', 0, G_OPTION_ARG_NONE, &opta,
			"Love artists instead of songs", NULL},
		{"album", 'A', 0, G_OPTION_ARG_NONE, &optA,
			"Love albums instead of songs", NULL},
		{"genre", 'g', 0, G_OPTION_ARG_NONE, &optg,
			"Love genres instead of songs", NULL},
		{ NULL, 0, 0, 0, NULL, NULL, NULL },
	};
	GOptionContext *ctx;

	ctx = g_option_context_new("[EXPRESSION]");
	g_option_context_add_main_entries(ctx, options, "eugene-love");
	g_option_context_set_summary(ctx, "eugene-love-"VERSION GITHEAD" - Love song/artist/album/genre");
	g_option_context_set_description(ctx, ""
"By default this command works on the current playing song.\n"
"For more information about the expression syntax, see:\n"
"http://www.sqlite.org/lang_expr.html");
	if (!g_option_context_parse(ctx, &argc, &argv, &error)) {
		g_printerr("Option parsing failed: %s\n", error->message);
		g_error_free(error);
		g_option_context_free(ctx);
		return 1;
	}
	g_option_context_free(ctx);

	if (argc > 1)
		ret = cmd_love_internal(true, argv[1], opta, optA, optg);
	else
		ret = cmd_love_internal(true, NULL, opta, optA, optg);
	return ret;
}
