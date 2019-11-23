/**
 * Copyright (C) 2019 Jacob Barkdull
 * This file is part of PHP Webserver GUI.
 *
 * PHP Webserver GUI is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * PHP Webserver GUI is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with PHP Webserver GUI.  If not, see <http://www.gnu.org/licenses/>.
 */


#include <stdlib.h>
#include <gtk/gtk.h>
#include <gio/gio.h>
#include <vte/vte.h>
#include "php-webserver-gui.h"

/* Shorter function name */
static GObject *
get_object (GtkBuilder *builder, const gchar *name)
{
	return gtk_builder_get_object (builder, name);
}

/* Enables or disables "input-enabled" property of a GObject */
static void
set_input_state (GObject *object, gboolean value)
{
	/* Initial GValue */
	GValue g_value_boolean = G_VALUE_INIT;

	/* Set GValue for GObject property to value */
	g_value_init (&g_value_boolean, G_TYPE_BOOLEAN);
	g_value_set_boolean (&g_value_boolean, value);

	/* Set GObject input enabled property */
	g_object_set_property (object, "input-enabled", &g_value_boolean);
}

/* Callback to execute after a VTE terminal is spawned */
static void
vte_callback (VteTerminal *terminal, GPid pid, GError *error, gpointer user_data)
{
	/* Cast data to SignalArgs type */
	SignalArgs *widgets = (SignalArgs *) user_data;

	gchar       *executable;
	const gchar *address;
	gint         port;
	gchar       *directory_uri;
	gchar       *document_root;
	gchar       *ini_uri;
	gchar       *ini = "";
	gchar       *php_command;
	gchar       *script;

	/* Enable open web browser button */
	gtk_widget_set_sensitive (GTK_WIDGET (widgets->openBrowser), TRUE);

	/* Get PHP executable file name */
	executable = gtk_file_chooser_get_filename (widgets->phpButton);

	/* Get address from address entry box */
	address = gtk_entry_get_text (widgets->addressEntry);

	/* Get port from port spin button text entry */
	port = atoi (gtk_entry_get_text (GTK_ENTRY (widgets->portSpinButton)));

	/* Get root directory from directory file chooser button */
	directory_uri = gtk_file_chooser_get_uri (widgets->directoryButton);
	document_root = g_filename_from_uri (directory_uri, NULL, NULL);

	/* Get INI file URI from INI file chooser button */
	ini_uri = gtk_file_chooser_get_uri (widgets->iniButton);

	/* Check if INI file chooser button has a URI value */
	if (ini_uri != NULL) {
		/* Get INI file path from URI */
		ini = g_filename_from_uri (ini_uri, NULL, NULL);

		/* Prepend "-c " to INI file path */
		ini = g_strdup_printf (" -c %s", ini);
	}

	/* Construct PHP Development Server command */
	php_command = g_strdup_printf (
		"%s -S %s:%d -t \"%s\"%s",
		executable, address, port, document_root, ini
	);

	/* Prepend command to run as root on port 1-1023 */
	if (port < 1024) {
		php_command = g_strdup_printf (widgets->rootCommand, php_command);
	}

	/* Construct shell script */
	script = g_strdup_printf (
		/* The following commands executed one after the other */
		"%s; %s; %s; %s; %s; %s\n",

		/* Unset history file to disable history for users */
		"unset HISTFILE",

		/* Define a clear terminal function */
		"vte_clear () { echo -ne '\\033c'; }",

		/* Clear terminal */
		"vte_clear",

		/* Constructed PHP command */
		php_command,

		/* Clear terminal on success */
		"if [ $? -eq 0 ]; then vte_clear; fi",

		/* Exit terminal without any output */
		"exit &> /dev/null"
	);

	/* Run the constructed shell script */
	vte_terminal_feed_child (widgets->terminal, script, -1);

	/* Disable VTE terminal input */
	set_input_state (G_OBJECT (widgets->terminal), FALSE);

	/* Free the INI memory if set */
	if (ini_uri != NULL) {
		g_free (ini);
	}

	/* And free other variables from memory */
	g_free (php_command);
	g_free (script);
}

/* Server switch button event handler */
G_MODULE_EXPORT gboolean
server_state (GtkSwitch *widget, gboolean state, gpointer user_data)
{
	/* Cast data to SignalArgs type */
	SignalArgs *widgets = (SignalArgs *) user_data;

	/* VTE terminal shell argument vector */
	gchar **argva = NULL;

	/* Check if server is started */
	if (gtk_switch_get_active (widgets->serverSwitch) == FALSE) {
		/* If so, disable open web browser button */
		gtk_widget_set_sensitive (GTK_WIDGET (widgets->openBrowser), FALSE);

		/* Enable VTE terminal input */
		set_input_state (G_OBJECT (widgets->terminal), TRUE);

		/* Send "End of Text" character to kill VTE terminal */
		vte_terminal_feed_child (widgets->terminal, "\003", -1);

		/* Reset VTE terminal */
		vte_terminal_reset (widgets->terminal, TRUE, TRUE);

		/* And do nothing else */
		return FALSE;
	}

	/* Otherwise, start VTE shell */
	g_shell_parse_argv (vte_get_user_shell (), NULL, &argva, NULL);

	/* Asynchronously spawn a VTE pseudo-terminal */
	vte_terminal_spawn_async (
		widgets->terminal,
		VTE_PTY_DEFAULT,
		NULL,
		argva,
		NULL,
		G_SPAWN_DEFAULT,
		NULL,
		NULL,
		NULL,
		-1,
		NULL,
		(VteTerminalSpawnAsyncCallback) vte_callback,
		user_data
	);

	/* Free shell argument vector */
	g_strfreev (argva);

	return FALSE;
}

/* Resets our INI file chooser widget */
G_MODULE_EXPORT void
clear_ini (GtkWidget *widget, gpointer user_data)
{
	/* Cast data to SignalArgs type */
	SignalArgs *widgets = (SignalArgs *) user_data;

	/* Unselect all files */
	gtk_file_chooser_unselect_all (widgets->iniButton);
}

/* Window close event handler */
G_MODULE_EXPORT gboolean
on_delete_event (GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
	/* Cast data to SignalArgs type */
	SignalArgs *widgets = (SignalArgs *) user_data;

	/* Show confirmation dialog if server was started */
	if (gtk_switch_get_active (widgets->serverSwitch)) {
		gtk_widget_show (widgets->confirmClose);
		return TRUE;
	}

	return FALSE;
}

/* Open web browser button click event handler */
G_MODULE_EXPORT void
open_browser (GtkWidget *widget, gpointer user_data)
{
	/* Cast data to SignalArgs type */
	SignalArgs *widgets = (SignalArgs *) user_data;

	const gchar *address;
	gint         port;
	gchar       *url;

	/* Get address from address entry box */
	address = gtk_entry_get_text (widgets->addressEntry);

	/* Get port from port spin button text entry */
	port = atoi (gtk_entry_get_text (GTK_ENTRY (widgets->portSpinButton)));

	/* Check if port is 80 */
	if (port == 80) {
		/* If so, construct server URL without port */
		url = g_strdup_printf ("http://%s/", address);
	} else {
		/* If not, construct server URL with port */
		url = g_strdup_printf ("http://%s:%d/", address, port);
	}

	/* Open URL in desktop configured default application */
	g_app_info_launch_default_for_uri (url, 0, NULL);

	/* And free URL from memory */
	g_free (url);
}

/* Opens or closes the configuration sidebar */
G_MODULE_EXPORT void
toggle_config_sidebar (GtkToggleButton *button, gpointer user_data)
{
	/* Cast data to SignalArgs type */
	SignalArgs *widgets = (SignalArgs *) user_data;

	/* Check if the configuration sidebar is open */
	if (gtk_revealer_get_reveal_child (widgets->configRevealer) == TRUE) {
		/* If so, close it */
		gtk_revealer_set_reveal_child (widgets->configRevealer, FALSE);
	} else {
		/* If not, open it */
		gtk_revealer_set_reveal_child (widgets->configRevealer, TRUE);
	}
}

int
main (int argc, char *argv[])
{
	const gchar     *glade_path;
	GtkBuilder      *builder;
	GtkWidget       *window;
	const gchar     *icon_path;
	GdkPixbuf       *window_icon;
	const gchar     *css_path;
	GtkCssProvider  *css_provider;
	SignalArgs       signalArgs;
	GtkFileFilter   *php_filters;
	GtkFileFilter   *ini_filters;
	GtkToggleButton *config_toggle;
	GtkWidget       *clear_ini_button;

	/* Set some paths */
	glade_path = "/org/gnome/php-webserver-gui/src/gui.glade";
	icon_path = "/org/gnome/php-webserver-gui/src/icon.svg";
	css_path = "/org/gnome/php-webserver-gui/src/theme.css";

	/* Initiate GTK */
	gtk_init (&argc, &argv);

	/* Load GtkBuilder UI Definitions file; Initiating GtkBuilder */
	builder = gtk_builder_new_from_resource (glade_path);

	/* Get the window */
	window = GTK_WIDGET (get_object (builder, "window"));

	/* Get window icon pixbuf */
	window_icon = gdk_pixbuf_new_from_resource (icon_path, NULL);

	/* Set window icon */
	if (window_icon != NULL) {
		gtk_window_set_icon (GTK_WINDOW (window), window_icon);
	}

	/* Setup CSS style */
	css_provider = gtk_css_provider_new ();

	/* Load CSS theme file */
	gtk_css_provider_load_from_resource (css_provider, css_path);

	/* Apply CSS theme to default screen */
	gtk_style_context_add_provider_for_screen (
		gdk_screen_get_default (),
		GTK_STYLE_PROVIDER (css_provider),
		GTK_STYLE_PROVIDER_PRIORITY_USER
	);

	/* Get configuration sidebar revealer */
	signalArgs.configRevealer = GTK_REVEALER (get_object (builder, "config_revealer"));

	/* Get PHP executable file chooser button */
	signalArgs.phpButton = GTK_FILE_CHOOSER (get_object (builder, "php_button"));

	/* Set PHP executable file chooser MIME type filters */
	php_filters = gtk_file_filter_new ();
	gtk_file_filter_add_mime_type (php_filters, "application/x-sharedlib");
	gtk_file_filter_add_mime_type (php_filters, "application/x-executable");
	gtk_file_filter_set_name (php_filters, "Executable Files");
	gtk_file_chooser_add_filter (signalArgs.phpButton, php_filters);

	/* Set "PHP" button to reasonable default location */
	gtk_file_chooser_select_filename (signalArgs.phpButton, "/usr/bin/php");

	/* Get address entry box */
	signalArgs.addressEntry = GTK_ENTRY (get_object (builder, "address_entry"));

	/* Get port entry box */
	signalArgs.portSpinButton = GTK_SPIN_BUTTON (get_object (builder, "port_spin_button"));

	/* Get directory file chooser button */
	signalArgs.directoryButton = GTK_FILE_CHOOSER (get_object (builder, "directory_button"));

	/* Set "Root Directory" button to user's home directory */
	gtk_file_chooser_set_current_folder (signalArgs.directoryButton, g_get_home_dir ());

	/* Get INI file chooser button */
	signalArgs.iniButton = GTK_FILE_CHOOSER (get_object (builder, "ini_button"));

	/* Set INI file chooser pattern filter */
	ini_filters = gtk_file_filter_new ();
	gtk_file_filter_add_pattern (ini_filters, "*.ini");
	gtk_file_filter_set_name (ini_filters, "INI Files");
	gtk_file_chooser_add_filter (signalArgs.iniButton, ini_filters);

	/* Get VTE terminal */
	signalArgs.terminal = VTE_TERMINAL (get_object (builder, "terminal"));

	/* Get server on/off switch */
	signalArgs.serverSwitch = GTK_SWITCH (get_object (builder, "server_switch"));

	/* Open web browser button */
	signalArgs.openBrowser = GTK_BUTTON (get_object (builder, "open_browser"));

	/* Get close confirmation dialog */
	signalArgs.confirmClose = GTK_WIDGET (get_object (builder, "confirm_close"));

	/* Default no root command */
	signalArgs.rootCommand = "%s";

	/* Check if pkexec exists */
	if (access ("/usr/bin/pkexec", F_OK) != -1) {
		/* If so, set it as root command */
		signalArgs.rootCommand = "pkexec %s";
	} else {
		/* If not, use gksu if it exists instead */
		if (access ("/usr/bin/gksu", F_OK) != -1) {
			signalArgs.rootCommand = "gksu '%s'";
		}
	}

	/* Get configuration sidebar toggle buttom */
	config_toggle = GTK_TOGGLE_BUTTON (get_object (builder, "config_toggle"));

	/* Get clear INI button */
	clear_ini_button = GTK_WIDGET (get_object (builder, "clear_ini_button"));

	/* Setup event signals */
	g_signal_connect (signalArgs.serverSwitch, "state-set", G_CALLBACK (server_state), &signalArgs);
	g_signal_connect (signalArgs.openBrowser, "clicked", G_CALLBACK (open_browser), &signalArgs);
	g_signal_connect (config_toggle, "toggled", G_CALLBACK (toggle_config_sidebar), &signalArgs);
	g_signal_connect (clear_ini_button, "clicked", G_CALLBACK (clear_ini), &signalArgs);
	g_signal_connect (window, "delete-event", G_CALLBACK (on_delete_event), &signalArgs);
	g_signal_connect (window, "destroy", G_CALLBACK (gtk_main_quit), NULL);

	/* Connect GtkBuilder event signals */
	gtk_builder_connect_signals (builder, NULL);

	/* Unref builder */
	g_object_unref (G_OBJECT (builder));

	/* Show window */
	gtk_widget_show (window);

	/* Draw */
	gtk_main ();

	return 0;
}
