SilkJS-redis
============

redis interface for SilkJS.

This module for SilkJS includes the REDIS server as well.

## Installing the server

To build and install the server:

sudo ./build.js server

This will automatically do a git pull to update the server sources as well.

## Installing the module

To build and install the SilkJS module:

sudo ./build.js module

## Notes

You will need to run the server before using the driver.

Installing the module also installs a simple SilkJS based redis client in /usr/local/bin:

redis-client.js

It's functionality is basically the same as the redis-client written in C/C++.

