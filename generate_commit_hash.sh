# generate_commit_hash.sh
echo "#pragma once" > commit_hash.h
echo "#define GIT_COMMIT_HASH \"$(git rev-parse HEAD)\"" >> commit_hash.h
echo "#define GIT_DESCRIBE \"$(git describe --tags --long --always --match=v*.* --dirty)\"" >> commit_hash.h
