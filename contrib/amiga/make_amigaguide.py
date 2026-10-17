#!/usr/bin/env python3
"""Generate a single AmigaGuide file aggregating all git-*.txt docs.

Usage:
    python3 make_amigaguide.py

Output:
    git-commands.guide in the contrib/amiga directory.
"""

from __future__ import annotations
import pathlib
import re

DOC_DIR = pathlib.Path(__file__).resolve().parent.parent.parent / "Documentation"
OUT_FILE = DOC_DIR.parent / "contrib" / "amiga" / "git-commands.guide"

HEADER = """@database GitCommands
"""


def list_command_docs():
    return sorted(DOC_DIR.glob("git-*.txt"))


def extract_title_and_summary(path: pathlib.Path):
    name = path.stem  # e.g., git-add
    summary = name
    with path.open("r", encoding="utf-8", errors="replace") as f:
        for line in f:
            if line.strip():
                summary = line.strip()
                break
    return name, summary


def safe_node_id(name: str) -> str:
    """Convert a command name to a node id friendly to AmigaGuide.

    Hyphens are fine for our generated AmigaGuide nodes, so keep the name as-is.
    """
    return name


def normalize_linkgit_name(name: str) -> str:
    """Normalize linkgit name tokens (e.g., replace {litdd} with "--")."""

    return name.replace("{litdd}", "--")


def linkgit_to_guide(text: str) -> str:
    """Convert `linkgit:cmd(section)` or `linkgit:cmd[section]` to AmigaGuide links."""

    pattern = re.compile(r"linkgit:([A-Za-z0-9_.+{}-]+)(?:\(|\[)(\d+)(?:\)|\])")

    def repl(match: re.Match[str]) -> str:
        name = normalize_linkgit_name(match.group(1))
        section = match.group(2)
        node_id = safe_node_id(name)
        label = f"{name}({section})"
        return f'@{{"{label}" link {node_id}}}'

    return pattern.sub(repl, text)


def build_index(entries):
    lines = []
    lines.append("@node MAIN \"Git Commands Index\"")
    lines.append("Git command references converted from the original git-*.txt files.\n")
    for name, summary in entries:
        node_id = safe_node_id(name)
        lines.append(f"@{{\"{name}\" link {node_id}}} - {summary}")
    lines.append("@endnode\n")
    return "\n".join(lines)


def build_command_node(name: str, content: str):
    node_id = safe_node_id(name)
    parts = []
    parts.append(f"@node {node_id} \"{name}\"")
    parts.append('@{"Back to index" link MAIN}\n')
    parts.append(content.rstrip())
    parts.append('\n\n@{"Back to index" link MAIN}')
    parts.append("@endnode\n")
    return "\n".join(parts)


def main():
    docs = list_command_docs()
    entries = []
    nodes = []

    for path in docs:
        name, summary = extract_title_and_summary(path)
        entries.append((name, summary))
        with path.open("r", encoding="utf-8", errors="replace") as f:
            content = f.read()
        content = linkgit_to_guide(content)
        nodes.append(build_command_node(name, content))

    guide = []
    guide.append(HEADER.rstrip())
    guide.append("")
    guide.append(build_index(entries))
    guide.extend(nodes)

    guide_text = "\r\n".join(guide) + "\r\n"

    # AmigaGuide readers expect CRLF and typically Latin-1; replace unsupported chars.
    encoded = guide_text.encode("latin-1", errors="replace")
    with OUT_FILE.open("wb") as f:
        f.write(encoded)
    print(f"Wrote {OUT_FILE} with {len(entries)} commands.")


if __name__ == "__main__":
    main()
