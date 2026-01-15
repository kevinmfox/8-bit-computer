import argparse
import csv
from pathlib import Path

ADDR_BITS = 9 # CF(1), ZF(1), Opcode(4), T-Step (3)
ROM_SIZE = 1 << ADDR_BITS  # 512
ADDR_ORDER = ["CF", "ZF", "Opcode", "T-Step"]  # MSB -> LSB

FIELD_WIDTHS = {
    "Opcode": 4,
    "T-Step": 3,
    "CF": 1,
    "ZF": 1,
}

EEPROM1_BITS = {"CO":0, "RO":1, "IO":2, "AO":3, "BO":4, "EO":5, "MI":6, "II":7}
EEPROM2_BITS = {"AI":0, "BI":1, "RI":2, "DI":3, "FI":4, "CE":5, "JMP":6, "END":7}

ACTIVE_LOW_SIGNALS = set(["CO", "RO", "IO", "AO", "BO", "EO", 
                          "MI", "II", "AI", "BI", "RI", "DI", 
                          "FI", "CE", "JMP", "END"])

SIGNAL_COLUMNS = [
    "CO","RO","IO","AO","BO","EO","MI","II","AI","BI","RI","DI","FI","CE","JMP","END"
]

NON_SIGNAL_COLUMNS = {"Address","Mnemonic","Opcode","T-Step","CF","ZF"}

def parse_opcode(opcode_str: str) -> list[int]:
    s = (opcode_str or "").strip()
    if s == "*" or s == "":
        return list(range(16))  # 0..15
    return [int(s, 2)]

def parse_tstep(tstep_str: str) -> int:
    # using T1, T2, T3, etc. in the source file
    s = tstep_str.strip().upper()
    if not s.startswith("T"):
        raise ValueError(f"Bad T-Step format: {tstep_str}")
    return int(s[1:])

def parse_flag(flag_str: str) -> list[int]:
    # *, 0 or 1
    s = flag_str.strip()
    if s == "*" or s == "":
        return [0, 1]
    return [int(s)]

def compute_address(opcode: int, tstep: int, cf: int, zf: int) -> int:
    values = {
        "Opcode": opcode,
        "T-Step": tstep,
        "CF": cf,
        "ZF": zf,
    }
    addr = 0
    total = 0
    for field in ADDR_ORDER:
        w = FIELD_WIDTHS[field]
        v = values[field]
        if v < 0 or v >= (1 << w):
            raise ValueError(f"{field} out of range: {v} (width {w})")
        addr = (addr << w) | v
        total += w
    if total != ADDR_BITS:
        raise ValueError(f"ADDR_BITS mismatch: packed {total} bits but ADDR_BITS={ADDR_BITS}")
    return addr

def encode_byte(signal_values: dict, mapping: dict) -> int:
    b = 0
    for sig, bitpos in mapping.items():
        asserted = int(signal_values.get(sig, 0))
        if sig in ACTIVE_LOW_SIGNALS:
            out_bit = 0 if asserted else 1
        else:
            out_bit = 1 if asserted else 0
        b |= (out_bit & 1) << bitpos
    return b & 0xFF

def write_header(path: Path, name1: str, data1: bytearray, name2: str, data2: bytearray):
    with path.open("w", encoding="utf-8") as f:
        f.write("// AUTO-GENERATED — DO NOT EDIT\n")
        f.write("// Generated from microcode.csv by generate_microcode.py\n\n")
        f.write("#pragma once\n")
        f.write("#include <Arduino.h>\n\n")

        def write_array(name: str, data: bytearray):
            f.write(f"const uint8_t {name}[{len(data)}] PROGMEM = {{\n  ")
            for i, b in enumerate(data):
                f.write(f"0x{b:02X}, ")
                if (i + 1) % 16 == 0 and i + 1 < len(data):
                    f.write("\n  ")
            f.write("\n};\n\n")

        write_array(name1, data1)
        write_array(name2, data2)

ap = argparse.ArgumentParser(description="KF-8 Microcode Generator")
ap.add_argument("--input", type=Path, default=Path("assets/microcode.csv"), help="Input CSV truth table file (default: assets/microcode.csv)")
ap.add_argument("--output", type=Path, default=Path("assets/microcode_images.h"), help="Output header file (default: assets/microcode_images.h)")

args = ap.parse_args()

# set our input and output files
microcode_csv = args.input
output_file   = args.output

print(f"Input file: {microcode_csv}")
print(f"Output file: {output_file}")
print("Assumes active-high inputs in the CSV source file, and will output active-low signals for the EEPROMs")

# make sure we have a microcode CSV truth table
if not microcode_csv.exists():
    raise SystemExit(f"Could not find {microcode_csv.resolve()}")

# initialize the ROM bits
rom1 = bytearray([0xFF] * ROM_SIZE)
rom2 = bytearray([0xFF] * ROM_SIZE)

# track which addresses we've seen/set
seen = set()

# go through the signal table
with microcode_csv.open(newline="", encoding="utf-8") as f:
    reader = csv.DictReader(f)
    
    # loop through each row
    for row in reader:
        opcodes = parse_opcode(row["Opcode"])
        tstep = parse_tstep(row["T-Step"])
        cfs = parse_flag(row["CF"])
        zfs = parse_flag(row["ZF"])

        # build signals for this row
        sigs_base = {}
        for sig in SIGNAL_COLUMNS:
            val = row.get(sig, "").strip()
            if val == "":
                continue
            sigs_base[sig] = int(val)

        # expand wildcards across all opcode/CF/ZF combinations
        for opcode in opcodes:
            for cf in cfs:
                for zf in zfs:
                    addr = compute_address(opcode, tstep, cf, zf)
                    if addr in seen:
                        raise ValueError(f"Duplicate definition: addr={addr}; opcode={opcode:04b}; tstep={tstep}; cf={cf}; zf={zf})")
                    seen.add(addr)
                    b1 = encode_byte(sigs_base, EEPROM1_BITS)
                    b2 = encode_byte(sigs_base, EEPROM2_BITS)
                    # add it to teh rom image
                    rom1[addr] = b1
                    rom2[addr] = b2

# write header for Arduino
write_header(output_file, "EEPROM1_IMAGE", rom1, "EEPROM2_IMAGE", rom2)

# all done
print(f"Filled {len(seen)}/{ROM_SIZE} addresses from CSV.")
print("Done")