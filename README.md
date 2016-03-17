alphaclock
==========

![shot](http://nuclear.mutantstargoat.com/sw/misc/alphaclock.jpg)

About
-----
Alphaclock is a transparent clock for the desktop. It renders the time using
pointless eye-candy, seamlessly integrated onto your desktop, thanks to the
alpha blending capabilities of X compositors.

Alphaclock requires a compositing manager to be running, for the effect to work
properly.

Git repository: http://github.com/jtsiomb/alphaclock

License
-------
Copyright (C) 2016 John Tsiombikas <nuclear@member.fsf.org>
Alphaclock is free software. Feel free to use, modify and/or redistribute it,
under the terms of the GNU General Public License version 3, or at your option
any later version published by the Free Software Foundation. See COPYING for
details.

Dependencies
------------
In order to build and run alphaclock you will need the following libraries:
 - libdrawtext (release >= 0.3) http://github.com/jtsiomb/libdrawtext
 - freetype 2 http://www.freetype.org

Installation
------------
First install the dependencies specified in the previous section. For instance
in debian-based systems you would do the following (assuming libdrawtext isn't
available in the repository):
```
sudo apt-get install libfreetype6-dev
git clone http://github.com/jtsiomb/libdrawtext
cd libdrawtext
./configure
make
sudo make install
```

Then just change into the alphaclock directory, and type `make`, followed by
`make install` as root. This will install alphaclock by default in `/usr/local`;
if you wish to change the installation prefix then modify the `PREFIX` line at
the top of the makefile before compiling and installing.
