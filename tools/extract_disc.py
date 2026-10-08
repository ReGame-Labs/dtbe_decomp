#!/usr/bin/env python3
"""Extract every file of a PlayStation disc image (MODE2/2352).

This reads every directory the ISO9660 path table lists, not only those the
root directory reaches, and writes its files under OUT, keeping the paths.
XA and STR files are copied as the 2048-byte user data of their sectors.

usage: extract_disc.py "DigimonTamers - Battle Evolution (Japan).bin" disks/jp
"""

import os
import struct
import sys

SECTOR = 2352
USER_DATA = 24  # sync (12) + header (4) + XA subheader (8)


def main():
    image, out = sys.argv[1], sys.argv[2]
    f = open(image, "rb")

    def sector(lba):
        f.seek(lba * SECTOR + USER_DATA)
        return f.read(2048)

    pvd = sector(16)
    pt_size = struct.unpack("<I", pvd[132:136])[0]
    pt_lba = struct.unpack("<I", pvd[140:144])[0]
    pt = b"".join(sector(pt_lba + i) for i in range((pt_size + 2047) // 2048))[:pt_size]

    dirs = []
    i = 0
    while i < len(pt):
        name_len = pt[i]
        extent = struct.unpack("<I", pt[i + 2 : i + 6])[0]
        parent = struct.unpack("<H", pt[i + 6 : i + 8])[0]
        name = pt[i + 8 : i + 8 + name_len].decode("ascii")
        dirs.append((name, extent, parent))
        i += 8 + name_len + (name_len & 1)

    def path(k):
        name, _, parent = dirs[k]
        return "" if k == 0 else os.path.join(path(parent - 1), name)

    count = 0
    for k, (_, extent, _) in enumerate(dirs):
        first = sector(extent)
        size = struct.unpack("<I", first[10:14])[0]  # the "." record
        data = b"".join(sector(extent + j) for j in range((size + 2047) // 2048))
        j = 0
        while j < len(data):
            rec_len = data[j]
            if rec_len == 0:
                j = (j // 2048 + 1) * 2048
                continue
            rec = data[j : j + rec_len]
            j += rec_len
            flags = rec[25]
            name = rec[33 : 33 + rec[32]]
            if flags & 2 or name in (b"\x00", b"\x01"):
                continue
            lba = struct.unpack("<I", rec[2:6])[0]
            length = struct.unpack("<I", rec[10:14])[0]
            dest = os.path.join(out, path(k), name.decode("ascii").split(";")[0])
            os.makedirs(os.path.dirname(dest), exist_ok=True)
            with open(dest, "wb") as w:
                for s in range((length + 2047) // 2048):
                    w.write(sector(lba + s))
                w.truncate(length)
            count += 1
    print(f"{count} files extracted to {out}")


if __name__ == "__main__":
    main()
