#!/bin/sh
set -e
main() {
    for b in "/dev/block/sd"* "/dev/block/mmcblk"* "/dev/sd"* "/dev/mmcblk"* "/dev/vd"*
    do
        [ ! -b "$b" ] && continue
        blockdev "$1" "$b"
    done
}
mark="/dev/device_read_only"
[ -f "$mark" ] && {
    main --setrw
    unlink "$mark"
    echo "已切换为读写。"
} || {
    main --setro
    touch "$mark"
    echo "已切换为只读。"
}
