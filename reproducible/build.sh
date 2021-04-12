#! /bin/sh

set -e

GUIX_COMMIT=$1
CLBOSS_COMMIT=$2

# Need a consistent umask so permissions are same.
umask 0022

# Download git repo so we can generate the
# hash that Guix expects.
mkdir -p tmp
cd tmp
git init
git remote add origin https://github.com/ZmnSCPxj/clboss.git
git fetch --depth 1 origin "$CLBOSS_COMMIT"
git checkout FETCH_HEAD
rm -rf .git
cd ..
FILE_HASH=`guix hash -r tmp`

# Create manifest.
cat manifest.scm.template | \
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
tar xzf "${GUIX_PACK}"
cd ..
# Copy the README.
# Older versions might not have the file, so just
# ignore if not present.
cp tmp/reproducible/README.md "${PACK_NAME}" || true
# Repack with as little non-determinism as possible.
tar --format=gnu --sort=name --mtime="@1" --owner=0 --group=0 --numeric-owner --check-links \
	-czf "${PACK_NAME}".tar.gz "${PACK_NAME}"
# Now delete the subdirectory.
chmod -R +w "${PACK_NAME}"
rm -rf "${PACK_NAME}"
rm -rf tmp
rm -rf manifest.scm

echo "${PACK_NAME}".tar.gz
