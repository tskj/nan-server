Daeominzed Linux Web Server
===========================

This is a web server written entirely in POSIX C for DA-NAN at HSN.

Dependencies
------------

* C Compiler.

* xmlstarlet installed and on your path.

* Tests are configured to work with VS Code.

* Webroot is at `/www`, and both user id and group id is 666, with www as the name,
  but this can of course be configured by changing the `#define`s.

* The server assumes a database file exists `/api/addressbook.db`,
  relative to the webroot. Similarly it looks for `/lib/not-found.html`.

* Two named pipes called `/bin/xmlvalres` and `/bin/xmlvalreq` need to be available.

* `src/sqlite3.o` needs to exist to allow linking when building the webserver. Giving
  the command `gcc -O3 -c sqlite3.c` in the src folder should do the trick.