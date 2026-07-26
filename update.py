import subprocess
import sys
import os
from datetime import date

BIN = "./gameoflife"
README = "README.md"

TEMPLATE = """# Streak of life

Conway's Game of Life — one step per day.

```
{grid}
```"""


def run_game():
    result = subprocess.run([BIN], capture_output=True, text=True)
    if result.returncode != 0:
        print("error running gameoflife:", result.stderr, file=sys.stderr)
        sys.exit(1)
    return result.stdout.strip()


def format_readme(output):
    lines = output.split("\n")
    header = lines[0] if lines else "--- ??? ---"
    grid = "\n".join(lines[1:]) if len(lines) > 1 else ""
    return TEMPLATE.format(grid=header + "\n" + grid)


def git_commit(date_str):
    cmds = [
        ["git", "add", "."],
        ["git", "commit", "-m", f"chore: update {date_str}"],
        ["git", "push", "--force"],
    ]
    for cmd in cmds:
        r = subprocess.run(cmd, capture_output=True, text=True)
        if r.returncode != 0:
            print(f"{' '.join(cmd)}: {r.stderr.strip()}", file=sys.stderr)
            sys.exit(1)
        print(f"ok: {' '.join(cmd)}")


def main():
    output = run_game()
    readme = format_readme(output)

    with open(README, "w") as f:
        f.write(readme)

    today = date.today().isoformat()
    git_commit(today)
    print(f"done — {today}")


if __name__ == "__main__":
    while True:
        main()
        time.sleep(86_400)
