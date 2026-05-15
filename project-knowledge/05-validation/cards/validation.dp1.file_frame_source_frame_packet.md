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

Validation route для першого bounded DP1 code generation slice:

```text
FileFrameSource -> FramePacket
```

Ця validation перевіряє, що generated patch дотримується canonical DP1 input contracts і не розширюється в unrelated runtime або pipeline areas.

# Required canonical sources

- `project-knowledge/02-dp1/canonical/DP1_CANONICAL_INDEX.md`
- `project-knowledge/02-dp1/canonical/data_domains/structures/raw/dp1.domain.raw.frame_packet.md`
- `project-knowledge/02-dp1/canonical/configuration/dp1.config.pipeline_configuration_c.md`
- `project-knowledge/06-tasks/cards/AMNT-0022.md`

# Validation goals

Generated patch має:

- створювати або заповнювати canonical `FramePacket` з file-backed frame source;
- зберігати input metadata, потрібні canonical contract;
- залишатись у межах approved scope;
- уникати hidden runtime coupling;
- надавати reviewable evidence.

# Minimal validation checklist

## Scope checks

- Patch не модифікує CameraPro / CameraProSim.
- Patch не додає POSIX IPC.
- Patch не додає TileScheduler/runtime execution logic.
- Patch не реалізує unrelated DP1 stages.
- Patch не виконує broad repository refactoring.
- Patch не модифікує build scripts без окремого approval.

## Contract checks

- `FramePacket` використовується як canonical output boundary.
- `frame_id` заповнений або явно stubbed.
- `camera_id` / `source_id` policy явно визначена.
- `image` carrier визначений.
- `pixel_format` і `bit_depth` handling явно визначені.
- `geometry` metadata зберігаються.
- `FramePacket` не використовується як mutable container для masks/candidates/measurements.

## Evidence checks

Generated patch має надати:

- список changed files;
- summary реалізованої поведінки;
- explicit non-goals;
- validation notes або validation command;
- known limitations;
- next proposed bounded task.

# Non-goals

Цей validation route не перевіряє:

- CameraPro integration;
- TileScheduler correctness;
- MeasurementRecord generation;
- temporal filtering;
- radiometric correctness;
- performance profiling;
- real-time execution.

# Escalation rule

Якщо generated patch:

- розширюється в unrelated runtime areas;
- змінює repository-wide architecture;
- додає hidden state;
- переписує canonical structures;
- чіпає forbidden files;

тоді ітерація має бути зупинена і звужена перед продовженням.
