#!/bin/sh
rsync \
  -rvz \
  --progress \
  -e 'ssh -p 6940 -i ~/Code/vms/freebsd131-aarch64/id_ed25519' \
  ./re vmbuild@localhost:/home/vmbuild/ \
  --exclude re/build \
  --exclude re/.git \
  --exclude re/tools/unicode_data
ssh -p 6940 -i ~/Code/vms/freebsd131-aarch64/id_ed25519 vmbuild@localhost "/home/vmbuild/re/tools/vmrun/freebsd131-aarch64/run.sh"
