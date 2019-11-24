/**
 * Copyright (C) 2019 Jacob Barkdull
 * This file is part of PHP Webserver GUI.
 *
 * PHP Webserver GUI is free software: you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * PHP Webserver GUI is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with PHP Webserver GUI.  If not, see <http://www.gnu.org/licenses/>.
 */


/* SignalArgs type structure passed between signal functions */
struct signal_args
{
	GtkRevealer    *configRevealer;
	GtkFileChooser *phpButton;
	GtkEntry       *addressEntry;
	GtkSpinButton  *portSpinButton;
	GtkFileChooser *directoryButton;
	GtkFileChooser *iniButton;
	GtkSwitch      *serverSwitch;
	GtkButton      *openBrowser;
	GtkWidget      *confirmClose;
	VteTerminal    *terminal;
	gchar          *rootCommand;
};

/* Typedefs */
typedef struct signal_args SignalArgs;
