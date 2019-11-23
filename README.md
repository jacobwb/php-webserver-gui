PHP Web Server GUI
===
PHP has included a built-in web server in its CLI SAPI since version 5.4, it is
not a fully-featured web server and it should not be used on a public network or
to host an actual website, but it is useful for application development and
testing purposes.

The problem: As a CLI feature, it requires a specific command to use, which is
easy to forget and gets buried in your terminal's history. The solution: A GTK+
interface that lets you choose the PHP executable, IP address or hostname, port
number, root directory, and INI file graphically.

Compiling the Software
===
To compile the software you will need the following libraries and their
dependencies installed...

Name | Version | Package
---- | ------- | --------------------
GTK+ | 3.10+   | `libgtk-3-dev`
GLib | 2.0+    | `libglib2.0-dev-bin`
VTE  | 0.48+   | `libvte-2.91-dev`

Depending on which GNU/Linux distribution you are using, the package names
listed above may differ slightly. If the package names do differ, you may need
to adjust the Makefile to use these different package names. I am open to
suggestions on how to improve compatibility with different distributions.

More Info
===
[Release Announcement](https://www.barkdull.org/blog/announcing-php-webserver-gui)

[Homepage](https://www.barkdull.org/software/php-webserver-gui)
