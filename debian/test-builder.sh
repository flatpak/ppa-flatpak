#!/bin/sh
set -e

if ! bwrap --ro-bind / / /bin/true; then
    echo "SKIP: Cannot run bwrap"
    exit 0
fi

tmpdir="$(mktemp -d -p /var/tmp)"

if ! setfattr -n user.test-xattr-support -v yes "$tmpdir"; then
    rm -fr "$tmpdir"
    echo "SKIP: Cannot set xattrs in /var/tmp"
    exit 0
fi

rm -fr "$tmpdir"

e=0
dh_auto_test --sourcedirectory=builder || e=$?

echo "Killing gpg-agent processes:"
pgrep --list-full --full "gpg-agent --homedir /var/tmp/test-flatpak-.*" >&2 || :
pgrep --list-full --full "gpg-agent --homedir /var/tmp/flatpak-test-.*" >&2 || :
pkill --full "gpg-agent --homedir /var/tmp/test-flatpak-.*" >&2 || :
pkill --full "gpg-agent --homedir /var/tmp/flatpak-test-.*" >&2 || :
exit "$e"
