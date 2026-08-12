# Ableton-first integration architecture

Status: active design input for the UI-first phase. Production DSP remains blocked by UI approval.

## Primary host

- Ableton Live 12 Suite
- Windows 10/11 x64
- Ehvota processors remain VST3
- Max for Live is used only for DAW integration

## Goals

The host integration must let Ehvota understand project structure without coupling audio processing to Max for Live. A future Codex MCP server may consume the same bridge, but the MCP server itself should live in a separate repository.

The integration must eventually distinguish three information layers:

1. **Ableton state** — tracks, returns, master, devices, selected objects, tempo, transport, clips/scenes where safely exposed.
2. **Ehvota audio state** — RefTap/RefEQ/RefComp/RefHub measurements, captures, mappings, references, proposals, processor state.
3. **Codex reasoning/control** — read-only inspection first, then audition/proposal actions, then explicitly approved writes.

## Proposed architecture

```text
                     CODEX
                       |
                      MCP
                       |
                 Ehvota DAW MCP
                       |
                 localhost IPC
                       |
              +--------+--------+
              |                 |
              v                 v
       Ehvota Bridge       Ehvota local bridge
       Max for Live              |
              |                  | RefLink external boundary
              | Live Object      |
              | Model            v
              v             Ehvota VST3 instances
         Ableton Live          RefHub / RefTap /
                              RefEQ / RefComp / ...
```

The exact IPC transport is intentionally undecided until a minimal proof compares localhost TCP/WebSocket/OSC or another robust local mechanism. Do not choose a transport based on convenience alone.

## External automation boundary

The plugin suite must be bridge-capable without exposing audio-thread memory.

Rules:

- External readers receive immutable snapshots or message frames.
- The audio thread never blocks waiting for the bridge.
- External writes are represented as validated commands or proposals.
- All processing changes retain undo/redo semantics.
- RefHub remains the authoritative user-facing approval surface for Ehvota-generated or externally requested processing proposals.
- No external process may silently write arbitrary processor parameters.
- The bridge must remain local-only by default and must not require internet access.
- The bridge must not expose reference audio files or captures to the network.

## Permission levels for future MCP integration

### Level 0 — Inspect

Read-only access to structured state and measurements.

Examples:

- project/set identity where available
- tempo and transport
- track/return/master inventory
- device inventory
- selected track/device
- Ehvota instance inventory
- RefHub reference and mapping state
- capture results
- audio metrics

### Level 1 — Audition

Non-destructive transient actions such as:

- play/stop where supported
- select track/device
- start an Ehvota capture
- switch RefHub comparison audition state
- trigger blind comparison workflow

### Level 2 — Propose

Create but do not apply Ehvota changes.

Examples:

- EQ proposal
- compression proposal
- gain-stage proposal
- mapping recommendation

### Level 3 — Apply approved changes

Only after explicit user approval or an approved proposal ID.

Examples:

- apply proposal
- reject proposal
- undo last approved change

Do not implement unrestricted arbitrary third-party device parameter writes in the first MCP version.

## RefHub implications

The Mapping workspace should become Ableton-aware while remaining semantically explicit.

A mapping may show:

```text
Reference drums.wav  ->  Ableton track: DRUM BUS
Reference bass.wav   ->  Ableton track: BASS BUS
Reference vocals.wav ->  Ableton track: VOCAL BUS
Reference music.wav  ->  Ableton track: MUSIC BUS
Full reference       ->  Ableton Master
```

Ableton can help identify track/device structure, but the user still controls semantic mapping. Never assume a track named `BUS 1` is drums.

The UI must support:

- discovered Ableton track/device context;
- Ehvota source role;
- connection health;
- missing/disconnected source states;
- one-to-one and composite mappings;
- a clear distinction between DAW state and measured audio state.

## RefTap remains required

The Ableton integration does not replace RefTap.

RefTap exists to provide detailed real-time audio analysis including, as implemented over time:

- loudness
- true peak
- spectrum
- dynamics
- crest factor
- transient behaviour
- Mid/Side
- correlation
- activity
- capture data

Use Ableton integration for **what exists and how it is arranged**. Use Ehvota measurements for **what the audio is doing**.

## RefTime implications

RefTime continues to read host tempo/meter through VST3 host context for its actual audio timing. The Ableton bridge may expose the same project timing for diagnostics and future Codex inspection, but it must not become a hard runtime dependency for RefTime.

This preserves normal VST3 operation if the Max for Live bridge is absent.

## Failure and offline behaviour

Ehvota VST3 plugins must remain usable when:

- Ehvota Bridge is not installed;
- the Max for Live device is absent from the Set;
- the MCP server is not running;
- Codex is unavailable;
- local IPC fails.

Expected degraded behaviour:

- core processing continues;
- RefHub local analysis continues;
- local RefLink between Ehvota plugins continues where supported;
- Ableton-structure enrichment is marked unavailable;
- no repeated modal warnings during normal mixing;
- reconnect is safe and non-destructive.

## UI-first requirements added by the Ableton switch

Before production DSP begins, the UI Preview should add deterministic states for:

- Ableton bridge connected;
- Ableton bridge missing;
- Ableton bridge stale/disconnected;
- track discovered but not semantically mapped;
- mapped track renamed;
- mapped track removed;
- duplicate Ehvota source roles;
- Ehvota instance found on a discovered Ableton track;
- MCP unavailable while Ableton bridge remains healthy;
- external proposal pending approval.

The existing UI approval gate remains mandatory.

## Proofs required before implementation expands

1. Read-only Live Object Model proof: enumerate tracks/returns/master and basic device inventory.
2. Local IPC proof: pass immutable project snapshots to a companion process without affecting audio performance.
3. RefLink external-boundary proof: expose dummy measurement snapshots without touching audio-thread memory.
4. Reconnect proof: close/reopen bridge components without crashing Live or Ehvota.
5. State identity proof: track renames and reordered tracks do not silently remap references to the wrong source.
6. Security proof: bridge binds locally and rejects malformed/unsupported commands.

Only after those proofs should an MCP write path be considered.