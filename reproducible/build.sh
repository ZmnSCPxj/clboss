#! /bin/sh

set -e

GUIX_COMMIT=$1
CLBOSS_COMMIT=$2

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
guix time-machine --commit="${GUIX_COMMIT}" -- install -p guix-profile/build git-minimal tar

GIT="`pwd`"/guix-profile/build/bin/git
TAR="`pwd`"/guix-profile/build/bin/tar

# Download git repo so we can generate the
# hash that Guix expects.
echo Downloading git repo.
mkdir -p tmp
cd tmp
"${GIT}" init > /dev/null 2>&1
"${GIT}" remote add origin https://github.com/ZmnSCPxj/clboss.git > /dev/null 2>&1
"${GIT}" fetch --depth 1 origin "$CLBOSS_COMMIT" > /dev/null 2>&1
"${GIT}" checkout FETCH_HEAD > /dev/null 2>&1
rm -rf .git
cd ..
FILE_HASH=`guix hash -r tmp`

# Create manifest.
# Use the one from the repo if it exists,
# otherwise it might be a version that
# predates the reproducible build, in
# which case fall back to using whatever
# version is available here.
if [ -e tmp/reproducible/manifest.scm.template ]; then
	MANIFEST_TEMPLATE=tmp/reproducible/manifest.scm.template
else
	MANIFEST_TEMPLATE=../manifest.scm.template
fi
cat ${MANIFEST_TEMPLATE} | \
	sed -e "s/@CLBOSS_COMMIT@/${CLBOSS_COMMIT}/g" \
	    -e "s/@FILE_HASH@/${FILE_HASH}/" \
	> manifest.scm

# Now pack CLBOSS, using the specified Guix commit.
GUIX_PACK=`guix time-machine --commit=${GUIX_COMMIT} -- pack -RR -S /bin=bin -S /lib=lib -m manifest.scm`
echo GUIX_PACK="${GUIX_PACK}"

# The Guix pack expands to the current directory,
# so create a wrapping directory and repack it.
PACK_NAME="clboss-${CLBOSS_COMMIT}-guix-${GUIX_COMMIT}"
mkdir "${PACK_NAME}"
cd "${PACK_NAME}"
"${TAR}" xzf "${GUIX_PACK}"
cd ..
# Copy the README.
# Older versions might not have the file, so just
# ignore if not present.
cp tmp/reproducible/README.md "${PACK_NAME}" || true
# Repack with as little non-determinism as possible.
"${TAR}" --format=gnu --sort=name --mtime="@1" --owner=0 --group=0 --numeric-owner --check-links \
	-czf "${PACK_NAME}".tar.gz "${PACK_NAME}"

# Move the file and exit the directory.
mv "${PACK_NAME}".tar.gz ..
cd ..
chmod -R +w clboss-build
rm -rf clboss-build || true

echo "${PACK_NAME}".tar.gz
