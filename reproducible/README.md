CLBOSS Reproducible Build Binary
================================

This reproducible build for CLBOSS uses [Guix](https://guix.gnu.org)
package manager.

To use, first install Guix on your system; you will not need to install
anything else (everything needed will be installed by Guix during
build, in such a way that only the build environment can see them).
Be warned that Guix can override your system-provided compile tools
and libraries (and cause programs you compiled to fail to find the
correct library versions), so if you are not ready to transition fully
to Guix, do not install anything using Guix, *only* install Guix and
do not use it for anything other than reproducibly building CLBOSS via
the script in this directory.
Then you need to select a particular Guix commit hash, and a specific
tag of CLBOSS (or if you want to challenge a specific binary, get the
Guix commit and the CLBOSS tag from that binary).

Then, from a source directory (source tarball or git repo):

    cd reproducible/
    ./build.sh ${GUIX_COMMIT} ${CLBOSS_TAG}

The script is simple-minded and assumes the `reproducible/` directory
is `pwd`, and will fail if not.

This will generate a `.tar.gz` file which contains compiled binaries
for the current system (CPU family (e.g. `x86_64`) and kernel (e.g.
`Linux`)).
Unfortunately the underlying `guix pack` does not support cross-
compilation yet, maybe when that gets supported.

This should yield a byte-per-byte same file as on another system with
same CPU family and kernel, provided you gave the same `GUIX_COMMIT`
and `CLBOSS_TAG`.

Using The Prebuilt Binary
-------------------------

The reproducible tarball will expand to a directory containing `bin/`,
`gnu/`, and `lib/` directories.

The tarball can be expanded anywhere in your file system, and the
binary will execute correctly.

All the binaries and libraries needed by CLBOSS are contained in the
`gnu/` directory.
It will not use any libraries from your system, i.e. it is a
self-contained package.

The `bin/` directory will contain the `clboss` binary.
Just pass the path to that binary as a `--plugin=` or
`--important-plugin=` to `lightningd`.

The binary itself has been stripped of debug information, but the
tarball contains the debug data for CLBOSS as well.
The debug data is in `lib/debug/`.
If you want to debug CLBOSS using `gdb` from the reproducible
tarball, then you have to give this command to `gdb`:

    (gdb) set debug-file-directory /path/to/lib/debug

Removing
--------

The files in `gnu/` are read-only, so a simple `rm -rf` of the
expanded directory will fail.
Prior to deleting, issue this command:

    chmod -R +w /path/to/gnu
