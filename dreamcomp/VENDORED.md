# Vendored copy

This directory is a squashed copy of dreamcomp (GPL-2.0, `LICENSE` and `THIRD_PARTY_NOTICES.md`
here) for the Hydro Thunder port.

| What | Commit |
|---|---|
| dreamcomp | `2f7b7c333e1600ba70c506b86ae1789edf8d5447` |
| `engine/third_party/libchdr` (submodule upstream, BSD-3-Clause) | `8e7b8bd32bc676b7e5c6b42fe7d2daca986c4a0d`, files copied without `.git` |

Verified 2026-11-16: the tree equals the dreamcomp checkout at that commit (`diff -rq`, ignoring
`.git`), and libchdr equals a fresh clone of rtissera/libchdr at its pinned commit.

Every change made here since: `docs/engine-changes.md` (rows dated 2026-11-16 and later).
