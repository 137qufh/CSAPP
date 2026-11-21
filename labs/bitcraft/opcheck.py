#!/usr/bin/env python3
"""Operator-restriction checker for bitcraft puzzles.

Reads puzzle metadata from the comment block directly above each function and
rejects control flow, calls, literals, and operators that the puzzle does not
list. This is original to this repository; it is a teaching aid, not a copy of
any course's driver.
"""

from __future__ import annotations

import sys

TYPE_WORDS = {
    "int",
    "unsigned",
    "char",
    "short",
    "long",
    "signed",
    "const",
    "void",
    "return",
    "static",
}

MULTI_OPS = [
    ">>=",
    "<<=",
    "+=",
    "-=",
    "&=",
    "|=",
    "^=",
    ">>",
    "<<",
    "!=",
    "==",
    "<=",
    ">=",
    "&&",
    "||",
    "++",
    "--",
    "->",
]

ALWAYS_BAD = {
    "for",
    "while",
    "do",
    "switch",
    "goto",
    "case",
    "default",
    "sizeof",
    "float",
    "double",
}


def strip_comments(src: str) -> str:
    out = []
    i = 0
    n = len(src)
    while i < n:
        if src.startswith("/*", i):
            j = src.find("*/", i + 2)
            if j < 0:
                break
            out.append(" ")
            i = j + 2
            continue
        if src.startswith("//", i):
            j = src.find("\n", i)
            if j < 0:
                break
            out.append("\n")
            i = j
            continue
        out.append(src[i])
        i += 1
    return "".join(out)


def parse_meta(block: str) -> dict:
    meta = {}
    for raw in block.splitlines():
        line = raw.strip().lstrip("*").strip().removeprefix("/*").strip()
        if ":" not in line:
            continue
        key, val = line.split(":", 1)
        meta[key.strip()] = val.strip()
    return meta


def match_body(src: str, brace_at: int) -> int:
    """Return index just past the closing brace. src[brace_at] must be '{'."""
    depth = 0
    i = brace_at
    n = len(src)
    while i < n:
        if src.startswith("/*", i):
            j = src.find("*/", i + 2)
            i = n if j < 0 else j + 2
            continue
        if src.startswith("//", i):
            j = src.find("\n", i)
            i = n if j < 0 else j
            continue
        c = src[i]
        if c == '"':
            i += 1
            while i < n and src[i] != '"':
                if src[i] == "\\":
                    i += 1
                i += 1
            i += 1
            continue
        if c == "{":
            depth += 1
        elif c == "}":
            depth -= 1
            if depth == 0:
                return i + 1
        i += 1
    raise SystemExit("unbalanced brace")


def const_value(tok: str) -> int:
    core = tok
    while core and core[-1] in "uUlL":
        core = core[:-1]
    return int(core, 0)


def check_body(body: str, allow: set, const_max: int, control: bool) -> list:
    errors = []
    ops = []
    i = 0
    n = len(body)
    while i < n:
        c = body[i]
        if c.isspace():
            i += 1
            continue
        if c.isalpha() or c == "_":
            j = i + 1
            while j < n and (body[j].isalnum() or body[j] == "_"):
                j += 1
            word = body[i:j]
            if word in ALWAYS_BAD:
                errors.append(f"forbidden keyword '{word}'")
            elif word in ("if", "else") and not control:
                errors.append(f"control flow '{word}' is not allowed")
            elif word not in TYPE_WORDS and word not in ("if", "else"):
                k = j
                while k < n and body[k].isspace():
                    k += 1
                if k < n and body[k] == "(":
                    errors.append(f"function call '{word}'")
            i = j
            continue
        if c.isdigit():
            j = i
            if body.startswith(("0x", "0X"), i):
                j = i + 2
                while j < n and body[j] in "0123456789abcdefABCDEF":
                    j += 1
            else:
                while j < n and body[j].isdigit():
                    j += 1
            while j < n and body[j] in "uUlL":
                j += 1
            tok = body[i:j]
            try:
                val = const_value(tok)
            except ValueError:
                errors.append(f"bad literal {tok}")
                i = j
                continue
            if val > const_max:
                errors.append(f"literal {tok} exceeds const_max {const_max}")
            i = j
            continue
        matched = False
        for m in MULTI_OPS:
            if body.startswith(m, i):
                ops.append(m)
                if m not in allow:
                    errors.append(f"operator '{m}' not in allow list")
                i += len(m)
                matched = True
                break
        if matched:
            continue
        if c in "~&|^+-<>!*/%?:":
            ops.append(c)
            if c not in allow:
                errors.append(f"operator '{c}' not in allow list")
            i += 1
            continue
        if c in "(){}[];,=":
            if c == ".":
                errors.append("floating literal or member access")
            i += 1
            continue
        errors.append(f"unexpected character {c!r}")
        i += 1
    return errors, ops


def main() -> int:
    if len(sys.argv) != 2:
        print("usage: opcheck.py bits.c", file=sys.stderr)
        return 2
    src = open(sys.argv[1], encoding="utf-8").read()
    idx = 0
    found = 0
    failed = 0
    while True:
        j = src.find("/* puzzle:", idx)
        if j < 0:
            break
        end = src.find("*/", j)
        if end < 0:
            print("unterminated puzzle comment")
            return 1
        meta = parse_meta(src[j:end])
        name = meta.get("puzzle", "?")
        allow = set(meta.get("allow", "").split())
        const_max = int(meta.get("const_max", "0"))
        control = meta.get("control", "no") == "yes"
        max_ops = int(meta.get("max_ops", "0"))
        brace = src.find("{", end)
        if brace < 0:
            print(f"{name}: missing body")
            return 1
        close = match_body(src, brace)
        body = strip_comments(src[brace + 1 : close - 1])
        errors, ops = check_body(body, allow, const_max, control)
        if len(ops) > max_ops:
            errors.append(f"{len(ops)} operators exceeds max_ops {max_ops}")
        found += 1
        if errors:
            failed += 1
            print(f"FAIL {name}: " + "; ".join(errors))
        else:
            print(f"ok   {name:24s} ops={len(ops):3d}/{max_ops}")
        idx = close
    if found < 20:
        print(f"expected at least 20 puzzles, found {found}")
        return 1
    print(f"{found} puzzles, {failed} failed")
    return 1 if failed else 0


if __name__ == "__main__":
    sys.exit(main())
