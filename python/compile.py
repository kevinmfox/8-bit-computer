import argparse
import re
from pathlib import Path
from dataclasses import dataclass
from typing import Dict, List, Optional, Tuple

@dataclass(frozen=True)
class InstrDef:
    opcode: int                     # 0..15
    operand_type: str               # "none", "imm4", "addr4", "vec4"
    aliases: Tuple[str, ...] = ()   # optional extra mnemonics

ISA: Dict[str, InstrDef] = {
    "NOP":    InstrDef(0x0, "none"),
    "LDA":    InstrDef(0x1, "addr4", aliases=("LD")),
    
    "LDB":    InstrDef(0x2, "addr4"),
    "STA":    InstrDef(0x3, "addr4", aliases=("ST")),
    "ADD":    InstrDef(0x4, "addr4"),
    "ADI":    InstrDef(0x5, "imm4"),
    "JMP":    InstrDef(0x6, "addr4", aliases=("JM")),
    "JMPV":   InstrDef(0x7, "vec4", aliases=("JV")),
    "JC":     InstrDef(0x8, "addr4"),
    "JZ":     InstrDef(0x9, "addr4"),
    "OUT":    InstrDef(0xA, "none"),
    "MOVA_B": InstrDef(0xB, "none", aliases=("MOVAB", "MOV_A_B", "MOVA")),
    "MOVB_A": InstrDef(0xC, "none", aliases=("MOVBA", "MOV_B_A", "MOVB")),
    "LDI":    InstrDef(0xD, "imm4"),
    "SUB":    InstrDef(0xE, "addr4"),
    "HLT":    InstrDef(0xF, "none", aliases=("STOP")),
}

# build alias lookup
ALIAS_LOOKUP: Dict[str, str] = {}
for original, idef in ISA.items():
    for a in idef.aliases:
        ALIAS_LOOKUP[a.upper()] = original

COMMENT_RE = re.compile(r";.*$")
LABEL_RE = re.compile(r"^[A-Za-z_]\w*$")

@dataclass
class LineInfo:
    lineNumber: int
    raw: str
    label: Optional[str]
    tokens: List[str]  # [opname, operand]
    comment: Optional[str]

def tokenize(line: str, lineNumber: int) -> LineInfo:
    # split once on ';' to preserve original comment (if any)
    code_part, sep, comment_part = line.partition(";")
    comment = f";{comment_part.rstrip()}" if sep else None

    # get the code piece
    stripped = code_part.strip()
    
    # check for blank line
    if len(stripped) == 0:
        return LineInfo(lineNumber=lineNumber, raw=line, label=None, tokens=[], comment=comment)

    # check for label
    if stripped.endswith(":"):
        matches = LABEL_RE.match(stripped[:-1])
        if matches:
            return LineInfo(lineNumber=lineNumber, raw=line, label=matches.group(0), tokens=[], comment=comment)
    
    # split the line (opname & operand)
    parts = stripped.split()
    
    # get the opname (check for alias)
    opname = [ALIAS_LOOKUP.get(parts[0].upper(), parts[0].upper())]

    # create the token
    tokens = opname + parts[1:2]
    return LineInfo(lineNumber=lineNumber, raw=line, label=None, tokens=tokens, comment=comment)

def first_pass(lines: List[LineInfo]) -> Dict[str, int]:
    labels: Dict[str, int] = {}
    pc = 0
    for line in lines:
        # line is a label; account for it
        if line.label:
            if line.label in labels:
                raise ValueError(f"Line {line.lineNumber}: Duplicate label - {line.label}")
            labels[line.label] = pc
            continue
        # line is not a label (must be a command)
        if line.tokens:
            pc += 1
    return labels

def resolve_operand(token: str, labels: Dict[str, int], lineNumber: int) -> int:
    # if it's a label (or looks like a label) check it
    if LABEL_RE.match(token):
        if token not in labels:
            raise ValueError(f"Line {lineNumber}: Unknown label - {token}")
        return labels[token]
    # try to parse the operand to an integer
    try:
        if token.lower().startswith("0x"):
            return int(token, 16)
        if token.lower().startswith("0b"):
            return int(token, 2)
        return int(token, 10)
    except ValueError:
        raise ValueError(f"Line {lineNumber}: Invalid operand - {token}")

def encode(line: LineInfo, labels: Dict[str, int]) -> int:
    tokens = line.tokens
    lineNumber = line.lineNumber

    # unknown command/instruction
    opname = tokens[0]
    if opname not in ISA:
         raise ValueError(f"Line {lineNumber}: Unknown instruction - {opname}")

    # get the instruction definition
    idef = ISA[opname]
    opcode = idef.opcode

    # commands that cannot have an operand (e.g. NOP)
    if idef.operand_type == "none":
        if len(tokens) != 1:
            raise ValueError(f"Line {lineNumber}: {opname} takes no operand")
        return (opcode << 4)

    # commands that must have exactly one operand (e.g. ADD)
    if len(tokens) != 2:
        raise ValueError(f"Line {lineNumber}: {opname} expects exactly 1 operand")

    # get the value of the operand
    val = resolve_operand(tokens[1], labels, lineNumber)
    if val < 0 or val > 0x0F:
        raise ValueError(f"Line {lineNumber}: Operand out of range (0..15) - Value = {val}; Original = {line.raw}")

    # return the full instruction
    return (opcode << 4) | (val & 0x0F)

def assemble(text: str) -> Tuple[List[int], List[LineInfo], Dict[str, int]]:
    raw_lines = text.splitlines()
    parsed = [tokenize(line, i + 1) for i, line in enumerate(raw_lines)]
    labels = first_pass(parsed)

    out: List[int] = []
    for line in parsed:
        if not line.tokens:
            continue
        b = encode(line, labels)
        out.append(b)

    return out, parsed, labels

def print_summary(program: List[int], labels: Dict[str, int], parsed: List[LineInfo]) -> None:
    lines: List[str] = []
    index = 0
    colWidth1 = 2
    colWidth2 = 8
    colWidth3 = 11

    blank_prefix = (
        f"{'':<{colWidth1}} | "
        f"{'':<{colWidth2}} | "
        f"{'':<{colWidth3}} | "
    )

    for line in parsed:
        if not line.tokens:
            if line.raw:
                lines.append(blank_prefix + line.raw)
            continue

        b = program[index]
        col1 = f"{index:02x}"
        col2 = f"{b:08b}"
        
        if len(line.tokens) == 1:
            col3 = f"{line.tokens[0]:<6} {'':>4}"
        else:
            val = resolve_operand(line.tokens[1], labels, line.lineNumber)
            col3 = (
                f"{line.tokens[0]:<6} "
                f"{val:#04x}"
            )
        
        if len(line.tokens) == 1:
            col4 = f"{line.tokens[0]:<6}"
        else:
            col4 = f"{line.tokens[0]:<6} {line.tokens[1]}"
        lines.append(f"{col1:<{colWidth1}} | {col2:<{colWidth2}} | {col3:<{colWidth3}} | {col4}")
        index += 1
    print("\n".join(lines))

def print_progmem(program: List[int], parsed: List[LineInfo], labels: Dict[str, int], name: str = "PROG", pad_comment: str = "PAD") -> str:
    lines: List[str] = []
    index = 0

    for line in parsed:
        # blank line (nothing to output)
        if not line.tokens and not line.comment:
            continue

        # comment only line
        if not line.tokens and line.comment:
            comment = line.comment
            lines.append(f"  // {comment}")
            continue

        byte = program[index]
        opname = line.tokens[0]

        if len(line.tokens) == 1:
            asm = opname
        else:
            val = resolve_operand(line.tokens[1], labels, line.lineNumber)
            asm = f"{opname} 0x{val:02X}"

        comment = f" {line.comment}" if line.comment else ""
        lines.append(f"  0x{byte:02X}, // {asm}{comment}")
        index += 1

    # add the padding
    while index < len(program):
        byte = program[index]
        lines.append(f"  0x{byte:02X}, // {pad_comment}")
        index += 1

    return f"const uint8_t {name}[{len(program)}]" + "PROGMEM = {\n" + "\n".join(lines) + "\n};"

ap = argparse.ArgumentParser(description="KF-8 Assembler (opcode high nibble, operand low nibble)")
ap.add_argument("input", type=Path, help="Input .asm file")
ap.add_argument("--program-name", type=str, default="PROG", help="Variable name of the program")
ap.add_argument("--pad-to", type=int, default=16, help="Pad program to N bytes (0 = no padding). Default is 16.")
ap.add_argument("--pad-byte", default="0x00", help="Pad byte. Default is NOP (0x00).")
ap.add_argument("--print-summary", action="store_true", help="Prints the parsed program to the console.")
args = ap.parse_args()

# grab some arguments
program_name = args.program_name.strip().upper()
if not re.fullmatch(r"[A-Z_][A-Z0-9_]*", program_name):
    raise SystemExit(f"Invalid --program-name '{args.program_name}'. Must be a valid C identifier (letters, digits, underscores).")
pad_byte = int(args.pad_byte, 0)
if pad_byte < 0 or pad_byte > 0xFF:
    raise ValueError(f"Pad bytes (--pad-byte) is invalid (0..255): {pad_byte}")
pad_to = args.pad_to

# go assemble the program
text = args.input.read_text(encoding="utf-8")
program, parsed, labels = assemble(text)

# check if the padding is less than the length of the program
if pad_to and len(program) > pad_to:
    raise SystemExit(f"Cannot pad less than the program length: Padding to (--pad-to) = {pad_to}; Program length = {len(program)}")

# add padding if we want it
if pad_to:
    program = program + [pad_byte] * (pad_to - len(program))

# check if we want to print the summary
if args.print_summary:
    print_summary(program, labels, parsed)

# pring the program to the console
progmem = print_progmem(program, parsed, labels, program_name)
print('--------------------------------')
print(progmem)
print('--------------------------------')