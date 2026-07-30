# tools/

Release pipeline utilities.

| File | What it is |
| --- | --- |
| [`upx.exe`](upx.exe) | [UPX](https://github.com/upx/upx), compresses `RealViewOn.exe`. **x64** build. |
| [`7zr.exe`](7zr.exe) | [7-Zip](https://github.com/ip7z/7zip) standalone console, builds `RealViewOn.7z`. **x86** build. |
| [`tools.lock.json`](tools.lock.json) | Pinned versions and SHA256 of the two binaries above. |
| [`Update-VendoredTools.ps1`](Update-VendoredTools.ps1) | Compares the binaries against upstream and updates them. |
| [`ReleaseUPXZIP.bat`](ReleaseUPXZIP.bat) | PostBuildEvent: copies, UPX-compresses and 7-Zip-packages the build. |
| [`GetWorka.ps1`](GetWorka.ps1) | Helper script for collecting workaround data. |

## Automated updates

[`.github/workflows/update-tools.yml`](../.github/workflows/update-tools.yml) runs
Mondays at 06:00 UTC. For every tool in `tools.lock.json` it:

1. Lists the upstream releases and discards drafts, prereleases and any tag or
   name that looks like `alpha` / `beta` / `rc` / `preview`. GitHub's
   `prerelease` flag is not enough on its own: neither `upx/upx` nor `ip7z/7zip`
   uses it, so a "26.03 beta" would arrive as a normal release.
2. Picks the newest stable one by **comparing version numbers**, not dates: a
   patch for an older line published later must not beat a newer minor.
3. If there is something new, downloads the asset and **verifies everything
   before replacing anything**:
   - the SHA256 matches the digest GitHub publishes for that asset;
   - the architecture read from the PE header matches the pinned value;
   - a real functional test passes, using the same flags as the release.
4. Replaces the binary, updates `version` and `sha256` in the lock, opens a PR.

If any check fails the repo's binary is **left untouched** and the job goes red.

### Checksum verification

The GitHub releases API returns a per-asset `digest` field (`sha256:<hex>`), and
the bot verifies the download against it before extracting or executing anything.

Two details worth knowing:

- For `7zr.exe` the digest covers the executable itself. For UPX it covers the
  **`.zip`**, so verification happens before extraction and the `upx.exe` inside
  gets its own SHA256 recorded in the lock afterwards.
- GitHub only computes digests for assets uploaded after that feature shipped.
  Current releases have one; older ones (`upx` v4.2.4, `7zip` 24.09 and earlier)
  return `null`. When the digest is missing the bot **fails by default** rather
  than quietly skipping the check. Use `-AllowMissingDigest` to accept an
  unverified asset; the PR body and the job summary then say so explicitly.

What this does and does not prove: matching the digest shows the bytes are exactly
what GitHub stores for that asset in that release of that repository, which rules
out corruption or tampering in transit. It is not a build provenance signature, so
it does not independently prove who produced the binary. The trust anchor is the
official upstream repository.

### The functional tests

A binary that downloads cleanly but cannot compress is useless, so checking the
version string is not enough:

- **UPX** — compresses a copy of `tools/7zr.exe` (a real, small PE already in the
  repo) with `--ultra-brute`, confirms it shrank, validates the result with
  `upx -t`, and **runs the compressed binary** to confirm it still works. That is
  exactly what the pipeline does to `RealViewOn.exe`.
- **7zr** — creates a `.7z` with `a -t7z -mx=9 -md=1m -ms=on` (the flags from the
  "Prepare release assets" step), validates it with `7zr t`, extracts it and
  compares the content's SHA256 against the original.

### Running it by hand

```powershell
# Only report whether updates exist. Does not touch the working tree.
pwsh tools/Update-VendoredTools.ps1 -CheckOnly

# Update one tool (downloads, verifies, replaces the binary and the lock).
pwsh tools/Update-VendoredTools.ps1 -Id upx

# All of them.
pwsh tools/Update-VendoredTools.ps1
```

Without a token the GitHub API allows 60 requests per hour per IP. If you hit it:

```powershell
$env:GITHUB_TOKEN = 'ghp_...'   # a token with no scopes is enough, it just raises the limit
pwsh tools/Update-VendoredTools.ps1 -CheckOnly
```

### Adding another tool

Add an entry to `tools.lock.json`; the workflow matrix is built by reading that
file, so the YAML needs no changes. Fields:

| Field | Purpose |
| --- | --- |
| `id` | Short identifier. It is the `-Id` value and the job name. |
| `path` | Path to the binary, relative to the repo root. |
| `repo` | `owner/name` of the upstream GitHub repository. |
| `version` | Currently pinned version. Maintained by the bot. |
| `arch` | `x64`, `x86` or `ARM64`. Validated against the PE header. |
| `asset` | Release asset name. `{version}` is substituted. |
| `archiveMember` | Path inside the zip, or `null` if the asset is the `.exe` itself. |
| `sha256` | Hash of the pinned binary. Maintained by the bot. |
| `usedBy` | Free text; shown in the PR body. |

To give the bot a functional test, add a `case` to `Invoke-FunctionalTest`.
Without one the tool still updates, but the PR states that no test ran.

> The lock is fully reserialised when updated. If you edit it by hand, keep the
> existing format (2-space indent, same key order) so the PR diff shows only
> `version` and `sha256`.

## Keeping the lock and the binaries consistent

Before comparing against upstream, the script checks that the on-disk binary's
SHA256 matches the lock. If somebody replaced an `.exe` without updating the lock
it fails and says so, instead of reasoning from data that is not true.

## What the bot does not do

- **It does not merge.** It only opens the PR.
- **It does not nag.** If a PR for that version already exists, open or closed, it
  will not create another: a closed one was the maintainer's decision.
- **It does not trigger the other workflows.** PRs created with `GITHUB_TOKEN` do
  not trigger workflows, by GitHub's design. On top of that `tools/**` is not in
  the `paths` filters of `tests.yml` or `msbuild.yml`, so these PRs arrive without
  CI. To have a UPX change proven by actually building and compressing, add
  `tools/**` to the `pull_request` paths in `msbuild.yml` and create the PR with a
  PAT instead of `GITHUB_TOKEN`.
- **It does not verify build provenance.** See the checksum note above.
