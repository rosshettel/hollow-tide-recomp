# CI packaging

GitHub Actions builds portable Windows and macOS zips. Disc images and
`1ST_READ.BIN` are never committed and never uploaded as release assets.

## Triggers

| Event | Effect |
|---|---|
| `push` to `main` | Build both platforms; create/update the moving **pre-release** tagged `beta` (assets replaced each time) |
| `release` `published` | Build both platforms; attach zips to **that** release only (does not touch `beta`) |

Android is a disabled stub until M9.

## Artifact names

- `hydro-thunder-windows-<shortsha>.zip`
- `hydro-thunder-macos-arm64-<shortsha>.zip`

Layout matches the USB pack Ross already tested (`dist/windows/`: exe, `SDL3.dll`,
`frontend/`, `hydrothunder.toml`, launcher `.bat` / `.command`, README). No CHD,
no `extracted/`, no generated `gen/`.

## Required secret: boot binary URL

The translator needs `game/extracted/fs/1ST_READ.BIN` at build time. That file is
~1.9 MB (over the 64 KB Actions secret limit), so CI downloads it from a URL you
control:

| Secret | Required | Purpose |
|---|---|---|
| `HT_BOOT_URL` | yes | HTTPS URL that serves the USA `1ST_READ.BIN` (SHA-1 `d8d54a28eac55cb4b63d9693951f9582a1b2505a`) |
| `HT_BOOT_AUTH` | no | Optional `Authorization` header value (e.g. `Bearer ghp_…` for a private release asset) |

Private sibling repo `rosshettel/hollow-tide-boot` holds the file as release tag `ci-boot`
(not in git history). Prefer the **API** asset URL plus a fine-scoped PAT that can read that
repo only; rotate `HT_BOOT_AUTH` if it was seeded from a short-lived OAuth token.

```sh
# API asset URL (Accept: application/octet-stream is added by tools/ci_restore_boot.sh)
gh secret set HT_BOOT_URL -R rosshettel/hollow-tide-recomp \
  -b 'https://api.github.com/repos/rosshettel/hollow-tide-boot/releases/assets/<id>'
gh secret set HT_BOOT_AUTH -R rosshettel/hollow-tide-recomp \
  -b 'Bearer ghp_…'
```

`tools/ci_restore_boot.sh` verifies the SHA-1 before the build. The file stays on
the runner workspace and is never packed into the zip (audit step greps the
archive listing).

## Jobs

- **windows-x64** (`windows-latest`): VS Build Tools + `clang-cl` + Ninja + Vulkan
  SDK (`glslc`) + SDL 3.4.20. Same clang-cl style as dreamcomp’s primary Windows
  path. Packages with `tools/package_windows.sh`.
- **macos-arm64** (`macos-14`): Homebrew `sdl3`, `molten-vk`, `shaderc`,
  `vulkan-loader`. Packages with `tools/package_macos.sh` (ships dylibs + MoltenVK
  ICD + `Play Hydro Thunder.command`).
- **android**: `if: false` until M9.

## Local packaging (no Actions)

With a finished build and extracted boot file already present:

```sh
./tools/package_windows.sh build-windows   # or the mingw build dir
./tools/package_macos.sh build
```

The mingw cross path (`./tools/build_windows_mingw.sh`) still stages
`dist/windows/` and `dist/hydro-thunder-windows-x64.zip` itself; CI uses the
native clang-cl job instead so runner packages match dreamcomp’s Windows toolchain.

## Workflow file

[`.github/workflows/build.yml`](../.github/workflows/build.yml)
