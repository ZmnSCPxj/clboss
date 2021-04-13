#! /bin/sh

# WARNING
# This file is intended to not change.
#
# If you need to change anything about how reproducible builds
# are done, instead change it in continue.sh.
#
# This separation means that past reproducible builds remain
# reproducible even with the latest version of the build.sh
# from CLBOSS, since the continue.sh will be taken from the
# specific CLBOSS version being built.

set -e

GUIX_COMMIT="$1"
CLBOSS_COMMIT="$2"

# Need a consistent umask so permissions are same.
umask 0022

# Enter the directory.
if mkdir clboss-build; then :; else
	echo clboss-build directory exists. >&2
	echo If no other build is ongoing, execute these: >&2
	echo "   "chmod -R +w clboss-build >&2
	echo "   "rm -rf clboss-build >&2
	exit 1
fi
cd clboss-build

# Create our own Guix profile so that
# `git` and `tar` are specific versions.
mkdir -p guix-profile
PROFILE="`pwd`"/guix-profile/build
guix time-machine --commit="${GUIX_COMMIT}" -- install -p "$PROFILE" git-minimal tar

GIT="$PROFILE"/bin/git
TAR="$PROFILE"/bin/tar

# Download git repo so we can generate the
# hash that Guix expects.
echo Downloading git repo.
mkdir -p tmp
cd tmp
"${GIT}" init > /dev/null 2>&1
"${GIT}" remote add origin https://github.com/ZmnSCPxj/clboss.git > /dev/null 2>&1
"${GIT}" fetch --depth 1 origin "$CLBOSS_COMMIT" > /dev/null 2>&1
"${GIT}" checkout FETCH_HEAD > /dev/null 2>&1
cd ..

# Find the continue.sh file.
if [ -e tmp/reproducible/continue.sh ]; then
	CONTINUE_SH=tmp/reproducible/continue.sh
else
	CONTINUE_SH=../continue.sh
fi

# Continue.
/bin/sh "${CONTINUE_SH}" "${GUIX_COMMIT}" "${CLBOSS_COMMIT}" "$PROFILE"

# Clean up.
cd ..
chmod -R +w clboss-build
rm -rf clboss-build || true
