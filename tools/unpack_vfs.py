#!/usr/bin/env python3
"""Unpack the disc's VFS2 archive (A.VFS): the code overlays and the game's data.

Every file is written under OUT at its path in the archive, inflated if it is
compressed, so the overlays are OUT/bin/<name>.bin.

The format, as the game reads it (src/engine/cd/cdfs.c, file_load.c):

- sector 0 is the head, a VfsHeader: "VFS2", the entry count, the sector the
  files start at (dataSector) and a word the game does not read;
- from sector 1 to dataSector is the table, zlib-compressed: count VfsEntry of
  12 bytes (at, size, nameOffset, flags as <IiHH), then their names,
  NUL-terminated, nameOffset bytes into them;
- an entry with VFS_DIRECTORY (0x800) in its flags is a directory of size
  entries starting at byte at of the table, "." and ".." among them; entry 0
  is the root. Any other is a file of size bytes at sector dataSector + at,
  (flags & 0x7FF) bytes into it;
- a compressed file starts with a CompressedHeader, "ZP00" and its inflated
  size, followed by its zlib stream.

usage: unpack_vfs.py disks/jp/A.VFS assets/jp
"""

import os
import struct
import sys
import zlib

SECTOR = 2048
ENTRY = struct.Struct("<IiHH")
VFS_DIRECTORY = 0x800
VFS_OFFSET_MASK = 0x7FF
COMPRESSED_MAGIC = b"ZP00"


def main():
    archive, out = sys.argv[1], sys.argv[2]
    data = open(archive, "rb").read()

    magic, count, data_sector, _ = struct.unpack("<4sIII", data[:16])
    if magic != b"VFS2":
        sys.exit(f"{archive}: not a VFS2 archive")
    table = zlib.decompressobj().decompress(data[SECTOR : data_sector * SECTOR])
    entries = [ENTRY.unpack_from(table, i * ENTRY.size) for i in range(count)]
    names = table[count * ENTRY.size :]

    def name(offset):
        return names[offset : names.index(b"\0", offset)].decode("ascii")

    files = compressed = 0
    root_at, root_size, _, _ = entries[0]
    pending = [(root_at // ENTRY.size, root_size, out)]
    while pending:
        first, size, path = pending.pop()
        for at, length, name_offset, flags in entries[first : first + size]:
            entry_name = name(name_offset)
            if entry_name in (".", ".."):
                continue
            dest = os.path.join(path, entry_name)
            if flags & VFS_DIRECTORY:
                pending.append((at // ENTRY.size, length, dest))
                continue
            start = (data_sector + at) * SECTOR + (flags & VFS_OFFSET_MASK)
            body = data[start : start + length]
            if body[:4] == COMPRESSED_MAGIC:
                inflated_size = struct.unpack("<i", body[4:8])[0]
                body = zlib.decompress(body[8:])
                if len(body) != inflated_size:
                    sys.exit(f"{dest}: inflated to {len(body)} bytes, not {inflated_size}")
                compressed += 1
            os.makedirs(path, exist_ok=True)
            with open(dest, "wb") as w:
                w.write(body)
            files += 1
    print(f"{files} files ({compressed} compressed) unpacked to {out}")


if __name__ == "__main__":
    main()
