---
id: validation.dp1.file_frame_source_frame_packet
title: "Validation route for FileFrameSource to FramePacket"
tags: [dp1, validation, canonical, input-route]
kind: validation-card
source_role: canonical
source:
  file: "project-knowledge/05-validation/cards/validation.dp1.file_frame_source_frame_packet.md"
status: "draft"
---

# Purpose

Validation route for the first bounded DP1 code generation slice:

```text
FileFrameSource -> FramePacket
```

This validation checks that the generated patch follows canonical DP1 input contracts and does not expand into unrelated runtime or pipeline areas.

# Required canonical sources

- `project-knowledge/02-dp1/canonical/DP1_CANONICAL_INDEX.md`
- `project-knowledge/02-dp1/canonical/data_domains/structures/raw/dp1.domain.raw.frame_packet.md`
- `project-knowledge/02-dp1/canonical/configuration/dp1.config.pipeline_configuration_c.md`
- `project-knowledge/06-tasks/cards/AMNT-0022.md`

# Validation goals

The generated patch must:

- create or populate a canonical `FramePacket` from a file-backed frame source;
- preserve input metadata needed by the canonical contract;
- remain inside the approved scope;
- avoid hidden runtime coupling;
- provide reviewable evidence.

# Minimal validation checklist

## Scope checks

- The patch does not modify CameraPro / CameraProSim.
- The patch does not introduce POSIX IPC.
- The patch does not add TileScheduler/runtime execution logic.
- The patch does not implement unrelated DP1 stages.
- The patch does not introduce broad repository refactoring.
- The patch does not modify build scripts unless explicitly approved.

## Contract checks

- `FramePacket` is used as the canonical output boundary.
- `frame_id` is populated or explicitly stubbed.
- `camera_id` / `source_id` policy is explicit.
- `image` carrier is defined.
- `pixel_format` and `bit_depth` handling are explicit.
- `geometry` metadata is preserved.
- `FramePacket` is not used as a mutable container for masks/candidates/measurements.

## Evidence checks

The generated patch must provide:

- list of changed files;
- summary of implemented behavior;
- explicit non-goals;
- validation notes or validation command;
- known limitations;
- next proposed bounded task.

# Non-goals

This validation route does not check:

- CameraPro integration;
- TileScheduler correctness;
- MeasurementRecord generation;
- temporal filtering;
- radiometric correctness;
- performance profiling;
- real-time execution.

# Escalation rule

If the generated patch:

- expands into unrelated runtime areas;
- changes repository-wide architecture;
- introduces hidden state;
- rewrites canonical structures;
- touches forbidden files;

then the iteration must be stopped and reduced in scope before continuing.
