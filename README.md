# WavesetUGens

SuperCollider server plugins (UGens) that port the **waveset** processes of the
[Composers Desktop Project](https://www.composersdesktop.com/) (CDP) — originated by
Trevor Wishart — to buffer-reading UGens, in the spirit of `GrainBuf`.

A *waveset* is a span of signal between zero crossings (more precisely, `numCycles` full
wavecycles). Because the source sound lives in a `Buffer` (random access, in RAM) rather
than streaming through, these UGens can emit **more** samples than they read — which is
what lets waveset processes stretch, roughen, and re-pitch a sound.

## UGens

| UGen | Description |
|------|-------------|
| `WavesetRepeat` | Replays each detected waveset `repeats` times before advancing — time-stretches and distorts. |

_More CDP waveset processes (omit, reverse, shuffle, interpolate, …) to follow._

## Requirements

- A SuperCollider source checkout matching the scsynth you run (see **API version** below).
- CMake ≥ 3.12 and a C++17 compiler.

## Build & install

```sh
./build.sh [path-to-supercollider-source]
```

This configures, builds, and installs into your user Extensions folder:

- macOS: `~/Library/Application Support/SuperCollider/Extensions/WavesetUGens/`
- Linux: `~/.local/share/SuperCollider/Extensions/WavesetUGens/`

Then in SuperCollider: **Recompile Class Library** (Cmd-Shift-L) and boot the server.

```supercollider
b = Buffer.read(s, Platform.resourceDir +/+ "sounds/a11wlk01.wav");  // mono source
{ WavesetRepeat.ar(b, repeats: 6) * 0.5 ! 2 }.play;
```

## ⚠️ API version (plugin ABI)

A compiled `.scx` is tied to SuperCollider's plugin ABI version (`sc_api_version`), and
**must match the scsynth that loads it** or you'll get
`API version mismatch … This plugin uses an unknown version of the interface`.

| SuperCollider | `sc_api_version` |
|---|---|
| 3.12.x – 3.14.x (releases) | **3** |
| `develop` (3.15-dev, reblocking/upsampling) | **6** |

Build against a source tree whose version matches your server. `build.sh` defaults
`SC_PATH` to a 3.14.1 checkout (API v3) for compatibility with the current releases; pass
a different path to target another version, e.g. `./build.sh /path/to/develop`.

## Build system

Uses SuperCollider's canonical plugin CMake helpers, vendored under `cmake_modules/`
(`SuperColliderServerPlugin.cmake`, `SuperColliderCompilerConfig.cmake`) from the SC
source tree's `tools/cmake_gen/` (BSD-licensed). Builds scsynth-only by default; flip
`-DSUPERNOVA=ON` if you build against a tree with the `nova-tt` submodule.

## Credits & license

The waveset algorithms are ports of CDP processes by **Trevor Wishart** and the Composers
Desktop Project; this repository reimplements them for SuperCollider and is an homage, not
an official CDP product. See [composersdesktop.com](https://www.composersdesktop.com/).

Licensed under **GPL-3.0** (see [`LICENSE`](LICENSE)).
