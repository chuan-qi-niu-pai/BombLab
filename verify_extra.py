"""Run against the unmodified bomb in the current repository, under WSL."""
from pathlib import Path
import hashlib
import subprocess
import sys

root = Path.cwd()
lines = []

def log(text):
    print(text, flush=True)
    lines.append(text)
    (root / "extra-test.log").write_text("\n".join(lines) + "\n", encoding="utf-8")

def run(name, args, data=None):
    result = subprocess.run([str(root / name), *map(str, args)], input=data,
                            stdout=subprocess.PIPE, stderr=subprocess.PIPE,
                            timeout=10, cwd=root)
    if result.returncode:
        raise RuntimeError(f"{name} {args}: exit {result.returncode}\n"
                           + result.stdout.decode(errors="replace")
                           + result.stderr.decode(errors="replace"))
    return result.stdout

def check_phase(phase, flags):
    args = ["--phase", phase, *flags]
    answer = run("defuser", args)
    result = run("bomb", args, answer)
    if f"Phase {phase} defused.".encode() not in result:
        raise RuntimeError(f"Missing success marker: phase {phase}, {flags}")

def main():
    log("defuser.c SHA256: " + hashlib.sha256((root / "defuser.c").read_bytes()).hexdigest())
    total = 0
    for seed in [314159, 271828, 20261030]:
        log(f"Non-public seed: {seed}")
        for phase in range(1, 7):
            check_phase(phase, ["--seed", seed])
            total += 1
            log(f"  Phase {phase}: PASS")
        flags = ["--seed", seed]
        answers = run("defuser", flags) + run("defuser", ["--secret", *flags])
        result = run("bomb", flags, answers)
        if b"Secret phase defused." not in result:
            raise RuntimeError(f"Honor did not pass for seed {seed}")
        total += 1
        log("  Honor: PASS")
    # Exercise every low-seven-bit selector/target with nonzero high bits.
    for phase in range(1, 7):
        for low in range(128):
            challenge = 0xFEDCBA9876543200 | low
            check_phase(phase, ["--challenge", challenge])
            total += 1
        log(f"Phase {phase}: 128/128 selector/target coverage checks PASS")
    log(f"Extra checks: {total}/{total} PASS")

try:
    main()
except Exception as error:
    log("FAIL: " + str(error))
    sys.exit(1)
