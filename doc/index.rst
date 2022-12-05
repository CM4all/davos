Davos
=====

What is Davos?
--------------

Davos is a WebDAV server with a WAS (Web Application Socket) frontend,
to be run by beng-proxy.


Installation & Configuration
----------------------------

Installation
^^^^^^^^^^^^

Install the Debian package :file:`cm4all-davos-plain`::

  apt-get install cm4all-davos-plain

Configuration
^^^^^^^^^^^^^

The "plain" backend does not need any configuration.  Everything is
controlled with WAS parameters.


Reference
---------

Common
^^^^^^

The following WAS parameters are understood by all backends:

- :envvar:`DAVOS_UMASK=octal-umask`: Configure the :envvar:`umask` for this
  process.  Example: ":samp:`0022`".

- :envvar:`DAVOS_MOUNT=uri`: Declare the location where Davos was "mounted".
  Example: ":samp:`/dav/`".

- :envvar:`DAVOS_DAV_HEADER=compliance-class`: Set the value of the
  :envvar:`DAV` response header after an `OPTIONS` request (see
  :rfc:`4918#section-10.1`).  Even though this software implements only class
  1, some clients may require faking class 2 in this header to work
  properly.  Defaults to ":samp:`1`".

Plain
^^^^^

The following WAS parameters are understood by the "plain" backend:

- :envvar:`DAVOS_DOCUMENT_ROOT=path`: The base path that maps to the
  :envvar:`DAVOS_MOUNT`.  Files in this directory will be served/edited
  by Davos.

The following environment variables are understood:

- :envvar:`DAVOS_ISOLATE_PATH=path`: Make all of the filesystem but
  this directory inaccessible.  This is a security hardening option
  which for example fixes the problem with symlinks pointing outside
  this path.  It requires user namespaces, mount namespaces and a
  writable :file:`/proc`.

- :envvar:`DAVOS_PIVOT_ROOT=path` (deprecated): Make the given directory the
  filesystem root, and effectively make the rest of the file system
  inaccessible.  This is a security hardening option which for example
  fixes the problem with symlinks pointing outside the root; they
  cannot be followed anymore.  This setting requires also setting
  :envvar:`DAVOS_PIVOT_ROOT_OLD` to a relative directory below the
  given :envvar:`DAVOS_PIVOT_ROOT`.  Note that
  :envvar:`DAVOS_DOCUMENT_ROOT` is based on that new filesystem root.

Example translation response::

  WAS "/usr/lib/cm4all/was/bin/davos-plain"
  PAIR "DAVOS_MOUNT=/dav/"
  PAIR "DAVOS_DOCUMENT_ROOT=/var/www"

Example hardened translation response::

  WAS "/usr/lib/cm4all/was/bin/davos-plain"
  SETENV "DAVOS_ISOLATE_PATH=/var/www"
  PAIR "DAVOS_MOUNT=/dav/"
  PAIR "DAVOS_DOCUMENT_ROOT=/var/www"

Example (deprecated) hardened translation response::

  WAS "/usr/lib/cm4all/was/bin/davos-plain"
  SETENV "DAVOS_PIVOT_ROOT=/var/www"
  SETENV "DAVOS_PIVOT_ROOT_OLD=mnt"
  PAIR "DAVOS_MOUNT=/dav/"
  PAIR "DAVOS_DOCUMENT_ROOT=/"
