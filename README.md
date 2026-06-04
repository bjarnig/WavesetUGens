# WavesetUGens

SuperCollider server plugins (UGens) that port the waveset processes of the
[Composers Desktop Project](https://www.composersdesktop.com/) (CDP) to
buffer-reading UGens, in the spirit of `GrainBuf`.

A waveset is a span of signal between zero crossings. The source sound lives in
a Buffer rather than streaming, which lets the waveset processes emit sound more
fluently.

## UGens

- **`WavesetRepeat`** — replays each waveset `repeats` times; time-stretches and distorts.
- **`WavesetReverse`** — plays each waveset reversed in time.
- **`WavesetOmit`** — plays `keep` wavesets, silences `skip` (timing preserved).
- **`WavesetDelete`** — plays `keep` wavesets, drops `drop` (compresses).
- **`WavesetMultiply`** — pitches each waveset up by `factor`, duration preserved.

## Install (prebuilt)

Download the archive for your OS from
[Releases](https://github.com/bjarnig/WavesetUGens/releases), extract the
`WavesetUGens/` folder into your SuperCollider Extensions folder, then recompile
the class library (Cmd/Ctrl-Shift-L) and reboot the server. Binaries target
SuperCollider 3.12–3.14.x (plugin API v3). On macOS, clear the download
quarantine: `xattr -dr com.apple.quarantine <extracted WavesetUGens folder>`.

## Build from source

```sh
./build.sh [path-to-supercollider-source]
```

Builds and installs into your user Extensions folder
(`.../SuperCollider/Extensions/WavesetUGens/`). Then in SuperCollider: recompile
the class library (Cmd-Shift-L) and boot the server.

```supercollider
b = Buffer.read(s, Platform.resourceDir +/+ "sounds/a11wlk01.wav"); // mono
{ WavesetRepeat.ar(b, repeats: 6) * 0.5 ! 2 }.play;
```

Requires CMake ≥ 3.12 and a C++17 compiler. Builds against a SuperCollider
source checkout matching the scsynth you run (see `build.sh`).

## Credits & license

Ports of CDP waveset processes by Trevor Wishart and the Composers Desktop
Project — an homage, not an official CDP product. Licensed under
**GPL-3.0** (see [`LICENSE`](LICENSE)).
