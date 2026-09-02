# Releasing Strata

Strata `v0.1.0` is designed to be released directly from a validated `main` commit by pushing a matching version tag.

## Release prerequisites

Before tagging a release:

1. Merge the intended release changes to `main`.
2. Confirm CI is green on the release commit.
3. Confirm `library.json` and `library.properties` contain the same semantic version.
4. Confirm `CHANGELOG.md` has a non-empty section for that version.
5. Confirm the release commit is the commit that should become the public source archive.

`scripts/validate_release_version.py` enforces the metadata/version/changelog requirements in CI.

## Create v0.1.0

Tag the validated `main` commit and push the tag:

```bash
git checkout main
git pull --ff-only
git tag v0.1.0
git push origin v0.1.0
```

The `CI` workflow runs again for the tag. The release job waits for metadata validation, source audits, host contracts, ESP32 backend contracts, and the full ESP32/ESP32-S3/ESP32-C3/ESP32-P4 example matrix.

Only after those jobs pass does the workflow create the GitHub release. The release job verifies that the tag already exists and uses the matching `CHANGELOG.md` section as the release notes.

## Version rules

- Git tag: `vMAJOR.MINOR.PATCH`
- `library.json`: `MAJOR.MINOR.PATCH`
- `library.properties`: `MAJOR.MINOR.PATCH`
- `CHANGELOG.md`: `## MAJOR.MINOR.PATCH`

All four must refer to the same version.

## Failed release run

Do not create a separate manual GitHub release to bypass a failed tag workflow. Fix the repository state on a new commit, move/create the intended release tag according to the project's release policy, and let the validated workflow create the release.
