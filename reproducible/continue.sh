#
# WARNING
#
# This file is not intended to be executed directly.
#
# Instead, this file will be invoked from build.sh
#
# The intent is that changes in how reproducible packaging is done
# will be in this continue.sh script.
# The build.sh script will then be "locked" and never change.
#
# This means that even if the reproducible build script is changed
# in the future, past builds are still performable, since we will
# use the continue.sh from past versions, while the build.sh will
# never change.

set -e
umask 0022

GUIX_COMMIT="$1"
CLBOSS_COMMIT="$2"
PROFILE="$3"

GIT="$PROFILE"/bin/git
TAR="$PROFILE"/bin/tar

echo Git download done.

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
echo Building CLBOSS.
GUIX_PACK=`guix time-machine --commit=${GUIX_COMMIT} -- pack -RR -S /bin=bin -S /lib=lib -m manifest.scm`
echo GUIX_PACK="${GUIX_PACK}"

# The Guix pack expands to the current directory,
# so create a wrapping directory and repack it.
echo Repacking CLBOSS.
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

echo Output: "${PACK_NAME}".tar.gz
