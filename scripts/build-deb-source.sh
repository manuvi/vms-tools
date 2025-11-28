#!/usr/bin/env bash
set -euo pipefail

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$repo_root"

if ! command -v dpkg-parsechangelog >/dev/null; then
  echo "dpkg-parsechangelog not found; install devscripts or dpkg-dev." >&2
  exit 1
fi

source_name="$(dpkg-parsechangelog -S Source)"
full_version="$(dpkg-parsechangelog -S Version)"
upstream_version="${full_version%%-*}"
orig_tar="../${source_name}_${upstream_version}.orig.tar.gz"

# Clean stale file list and old tarball in repo root
rm -f "${repo_root}/debian/files"
rm -f "${orig_tar}"

echo "Creating orig tarball: ${orig_tar}"
tar -C "$repo_root" \
  --exclude=debian \
  --exclude=build \
  --exclude-vcs \
  --transform="s,^.,${source_name}-${upstream_version}," \
  -czf "${orig_tar}" .

echo "Building source package..."
dpkg-buildpackage -S -us -uc

echo "Done. Source package artifacts are in the parent directory."
