#!/bin/bash -ex

GITDATE="`git show -s --date=short --format='%ad' | sed 's/-//g'`"
GITREV="`git show -s --format='%h'`"
REV_NAME="lemonade-unified-source"

COMPAT_LIST='dist/compatibility_list/compatibility_list.json'

mkdir artifacts

pip3 install git-archive-all
touch "${COMPAT_LIST}"
#wget -q https://api.citra-emu.org/gamedb -O "${COMPAT_LIST}"
git describe --abbrev=0 --always HEAD > GIT-COMMIT
git describe --tags HEAD > GIT-TAG || echo 'unknown' > GIT-TAG
git archive-all --include "${COMPAT_LIST}" --include GIT-COMMIT --include GIT-TAG --force-submodules artifacts/"${REV_NAME}.tar"

cd artifacts/
xz -T0 -9 "${REV_NAME}.tar"
sha256sum "${REV_NAME}.tar.xz" > "sha256.txt"
mv sha256.txt "${GITHUB_WORKSPACE}/"
cd ..
grep -v "lemonade-unified-source.tar.xz" sha256.txt > temp.txt && rm sha256.txt && mv temp.txt sha256.txt
