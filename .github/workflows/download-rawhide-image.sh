#!/bin/sh

set -e

[ -n "$1" ] && cd "$1"

BASE_URL="https://download.fedoraproject.org/pub/fedora/linux/development/rawhide/Cloud/x86_64/images"
GPG_URL="https://getfedora.org/static/fedora.gpg"
IMG_SUFFIX=".vagrant-virtualbox.box"

wget -r -nd -np -l 1 -H -e robots=off -A "*$IMG_SUFFIX,*-CHECKSUM" "$BASE_URL"

latest_image="$(ls -1q *$IMG_SUFFIX | tail -n 1)"
if [ -z "$latest_image" ]; then
    echo "$0: no image file downloaded!" 1>&2
    exit 1
fi

shasum -a 256 --status --ignore-missing -c ./*-CHECKSUM

echo "$(readlink -f "$latest_image")"
