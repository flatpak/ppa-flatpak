#!/bin/sh
set -eu

adverb=

if [ "$DEB_HOST_ARCH_BITS" = 64 ]; then
    # reprotest sometimes uses linux32 even for x86_64 builds, and
    # Flatpak's tests don't support this.
    adverb=linux64
fi

e=0
$adverb dh_auto_test || e=$?

find . -name 'test*.log' \
-not -name test-suite.log \
-print0 | xargs -0 tail -v -c1M

echo "Killing gpg-agent processes:"
pgrep --list-full --full "gpg-agent --homedir /var/tmp/test-flatpak-.*" >&2 || :
pgrep --list-full --full "gpg-agent --homedir /var/tmp/flatpak-test-.*" >&2 || :
pkill --full "gpg-agent --homedir /var/tmp/test-flatpak-.*" >&2 || :
pkill --full "gpg-agent --homedir /var/tmp/flatpak-test-.*" >&2 || :
exit "$e"
