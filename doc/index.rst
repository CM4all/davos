Davos
=====

What is Davos?
--------------

Davos is a WebDAV server with a WAS (Web Application Socket) frontend,
to be run by beng-proxy.  There are two backends: one that stores in a
plain filesystem (called "plain") and one for CM4all OnlineDrive
(called "od").


Installation & Configuration
----------------------------

Installation
^^^^^^^^^^^^

Depending on your choice of backend, install either
`cm4all-davos-plain` or `cm4all-davos-od`.  Example::

  apt-get install cm4all-davos-plain

Configuration
^^^^^^^^^^^^^

The "plain" backend does not need any configuration.  Everything is
controlled with WAS parameters.

The "od" backend requires a `libod` configuration file.


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

- :envvar:`DAVOS_PIVOT_ROOT=path`: Make the given directory the
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
  SETENV "DAVOS_PIVOT_ROOT=/var/www"
  SETENV "DAVOS_PIVOT_ROOT_OLD=mnt"
  PAIR "DAVOS_MOUNT=/dav/"
  PAIR "DAVOS_DOCUMENT_ROOT=/"

Online-Drive
^^^^^^^^^^^^

The "od" backend expects two command-line arguments: the path of the
`libod` configuration file and the name of the "group" within this
file.

The following WAS parameters are understood by the "od" backend:

- :envvar:`DAVOS_SITE=name`: The site id.

Example translation response::

  WAS "/usr/lib/cm4all/was/bin/davos-od"
  APPEND "/etc/cm4all/davos/od.conf"
  APPEND "foo"
  PAIR "DAVOS_MOUNT=/dav/abc/"
  PAIR "DAVOS_SITE=abc"

`libod` Configuration
^^^^^^^^^^^^^^^^^^^^^

`libod` is configured with an INI-style
text file containing at least 3 groups.  Example::

  [foo]
  data = foo_data
  meta = foo_meta

  [foo_data]
  module = fs
  path = /var/www

  [foo_meta]
  module = sql
  uri = codb:postgresql:strict:dbname=od

The first section is the one whose name you pass to `davos-od`.  It
chooses a "data" group and a "meta" group.  These groups configure the
according module.  The "meta" module maintains file metadata
(directory structure, file names, attributes), and the "data" module
stores file contents.

The `fs` module stores file contents in
the local file system.  Each site has its own directory
inside the given :envvar:`path`.  Instead of
:envvar:`path`, you can specify
:envvar:`regex` and :envvar:`expand_path`::

  [foo_data]
  module = fs
  regex = ^(..)(..)(........)$
  expand_path = /var/www/data/\1/\2/\3

This assumes that site ids have 12 characters, and will assume nested
subdirectories.

The `sql` module uses `libcodb` to store metadata in a relational
database.

For more information, read the `libod` documentation.
