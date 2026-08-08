# Upstream synchronization

This repository is a standalone integration snapshot of two upstream projects:

| Component | Upstream | Base commit |
|---|---|---|
| CrossPoint Reader | https://github.com/crosspoint-reader/crosspoint-reader | `4e619035` |
| FreeInk SDK | https://github.com/Free-Ink/freeink-sdk | `a485dc4` |
| Murphy research | https://github.com/crosspoint-reader/Murphy | `9bb430c` |

FreeInk SDK is intentionally vendored at `freeink-sdk/` so a single clone is
buildable while the upstream pull requests are pending. When synchronizing:

1. Merge or replay new CrossPoint changes at the repository root.
2. Merge or replay FreeInk SDK changes inside `freeink-sdk/`.
3. Rebuild both `murphy_m4` and CrossPoint's default target.
4. Repeat the documented on-device M4 validation before publishing a release.

The long-term goal is to upstream the hardware layer to FreeInk SDK and the
application integration to CrossPoint, allowing this repository to become a
thin release/integration layer instead of maintaining permanent source divergence.
