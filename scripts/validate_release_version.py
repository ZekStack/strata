#!/usr/bin/env python3

import json
import os
import re
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
SEMVER = re.compile(r"^[0-9]+\.[0-9]+\.[0-9]+$")


def property_value(name: str) -> str:
	for line in (ROOT / "library.properties").read_text().splitlines():
		key, separator, value = line.partition("=")
		if separator and key.strip() == name:
			return value.strip()
	raise SystemExit(f"missing {name} in library.properties")


manifest_version = json.loads((ROOT / "library.json").read_text())["version"]
properties_version = property_value("version")

if not SEMVER.fullmatch(manifest_version):
	raise SystemExit(f"library.json version is not release semver: {manifest_version}")

if manifest_version != properties_version:
	raise SystemExit(
		f"version mismatch: library.json={manifest_version}, "
		f"library.properties={properties_version}"
	)

changelog = (ROOT / "CHANGELOG.md").read_text()
section = re.search(
	rf"^## {re.escape(manifest_version)}\s*$\n(?P<body>.*?)(?=^## |\Z)",
	changelog,
	flags=re.MULTILINE | re.DOTALL,
)
if section is None:
	raise SystemExit(f"CHANGELOG.md has no {manifest_version} heading")

release_notes = section.group("body").strip()
if not release_notes or not any(line.startswith("- ") for line in release_notes.splitlines()):
	raise SystemExit(f"CHANGELOG.md {manifest_version} section has no release notes")

github_ref = os.environ.get("GITHUB_REF", "")
if github_ref.startswith("refs/tags/"):
	tag = github_ref.removeprefix("refs/tags/")
	if tag != f"v{manifest_version}":
		raise SystemExit(
			f"release tag {tag} does not match library version v{manifest_version}"
		)

print(f"Strata release metadata is consistent at {manifest_version}")
