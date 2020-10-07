CLBOSS The C-Lightning Node Manager
===================================

CLBOSS is an automated manager for C-Lightning forwarding nodes.

CLBOSS is effectively a bunch of heuristics modules wired together
to a regular clock to continuously monitor your node.

Its design goal is to make it so that running a Lightning Network
node is as simple as installing C-Lightning and CLBOSS, putting
some amount of funds of 0.1BTC or more, and making sure you have
continuous Internet and power to the hardware running it.

Current versions of CLBOSS might not achieve this goal yet
perfectly, but hopefully with enough effort and iteration and raw
coding and etc etc it will someday be as unusual to manually
manage a Lightning node as writing (as opposed to reading) machine
language is unusual today.

I hope CLBOSS can make the transition from pre-Lightning to
post-Lightning much smoother in practice.

Dependencies
------------

If you are installing from some official source release tarball,
you only need the below packages installed on a Debian or
Debian-derived systems:

* `build-essential`
* `pkg-config`
* `libev-dev`
* `libcurl4-gnutls-dev`
* `libsqlite3-dev`

Equivalent packages have a good probability of existing in
non-Debian-derived distributions as well.

The following dependency is technically optional, but is strongly
recommended (CLBOSS will check it at runtime so you do not need
it while building):

* `dnsutils`

If you have to build directly from github.com, you need the below
Debian packages in addition:

* `git`
* `automake`
* `autoconf-archive`

A design goal of CLBOSS is to reduce the above dependencies even
further.

Installing
----------

From an official source release, just:

    ./configure && make
    sudo make install # or su first, then make install

This will install `clboss` as a standard executable, usually in
`/usr/local/bin/` by default.
You will then need to modify your `lightning.conf` to add the
path to `which clboss` as a `plugin` or `important-plugin` of
`lightningd`.

From a git clone, you first need to execute:

    autoreconf -i

Then run the `./configure && make && sudo make install`.
