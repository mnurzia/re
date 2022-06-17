#!/bin/sh
rsync \
  -rvz \
  --progress \
  -e 'ssh -p 2222 -i ~/Code/vms/windows-10-amd64/id_ed25519' \
  --rsync-path=/usr/bin/rsync \
  ./re build@localhost:/home/build/ \
  --exclude re/build \
  --exclude re/.git \
  --exclude re/tools/unicode_data
ssh -p 2222 -i ~/Code/vms/windows-10-amd64/id_ed25519 build@localhost "/home/build/re/tools/run_on_windows.sh"
