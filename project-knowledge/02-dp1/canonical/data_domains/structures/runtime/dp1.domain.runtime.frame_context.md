---
id: dp1.domain.runtime.frame_context
title: "Runtime-контекст кадру DP1"
tags: [dp1, canonical, data-domain, runtime, structure, frame-context]
kind: data-domain-card
source_role: canonical
source:
  file: "project-knowledge/02-dp1/canonical/data_domains/structures/runtime/dp1.domain.runtime.frame_context.md"
status: "draft"
---

## Definition

`FrameContext` є canonical runtime/context structure, яка супроводжує обробку одного кадру або похідного від кадру пакета між етапами DP1.

Вона відповідає на питання: у якому pipeline/runtime context обробляється кадр.

## Assumptions

- `FrameContext` передається між stages разом із input/output data objects, але не замінює їх.
- `FrameContext` не є pipeline configuration `C`.
- Конкретна C++ форма context має визначатися окремою implementation task.
- `FrameContext` супроводжує один frame/source tuple і має бути узгоджений із
  `FramePacket.camera_id` / `FramePacket.source_id`.

## Theorem / Contract

`FrameContext` має мінімально містити або посилатися на:

- `frame_id` — ідентифікатор кадру, узгоджений із `FramePacket`.
- `camera_id` або `source_ref` — ідентичність active camera/source поточного DP1
  instance, узгоджена з `FramePacket`.
- `pipeline_run_id` — ідентифікатор pipeline run або processing session.
- `config_ref` або config snapshot reference — посилання на active configuration `C`.
- `pixel_route` — selected route, наприклад `U8`, `U16` або `F32`.
- `stage_status` — bounded map/list статусів stages для поточного кадру.
- `profiling` — frame-scoped profiling data з `dp1.domain.profiling`.
- `warnings` / `errors` — structured diagnostics для поточного кадру.
- optional `source_metadata_ref` — посилання на camera/source metadata.

`FrameContext` не має містити primary image buffers, masks, candidates, segments або measurements як hidden payload.

Рекомендована C++ форма:

```cpp
struct FrameContext {
    std::uint64_t frame_id = 0;
    int camera_id = -1;
    std::string pipeline_run_id;
    const PipelineConfig *config_ref = nullptr;
    int worker_count = 0;
    PixelFormat input_format = PixelFormat::U16;
    InputBitDepth input_bit_depth = InputBitDepth::Bit16;
    std::vector<StageStatus> stage_statuses;
    FrameProfiling profiling;
    std::vector<DiagnosticMessage> diagnostics;
};
```

## Поля

```yaml
fields:
  - name: "`frame_id`"
    type: "`std::uint64_t`"
    purpose: "Зв'язок із `FramePacket`."
    used_for: "Validation, trace."
    memory: "8 B"
  - name: "`camera_id`"
    type: "`int`"
    purpose: "Active source camera поточного DP1 instance."
    used_for: "Identity validation, DP2 handoff trace."
    memory: "4 B"
  - name: "`pipeline_run_id`"
    type: "`std::string`"
    purpose: "Ідентифікатор запуску pipeline."
    used_for: "Reproducibility, logs."
    memory: "~24 B + payload"
  - name: "`config_ref`"
    type: "`const PipelineConfig*`"
    purpose: "Active configuration `C`."
    used_for: "Stage parameters, route selection."
    memory: "8 B"
  - name: "`worker_count`"
    type: "`int`"
    purpose: "Кількість tile workers для кадру."
    used_for: "Profiling, memory accounting."
    memory: "4 B"
  - name: "`input_format`"
    type: "`PixelFormat`"
    purpose: "Route-level input carrier."
    used_for: "Перевірка, що stage route узгоджений із frame."
    memory: "4 B"
  - name: "`input_bit_depth`"
    type: "`InputBitDepth`"
    purpose: "Route-level input bit depth."
    used_for: "Threshold/range validation."
    memory: "4 B"
  - name: "`stage_statuses`"
    type: "`std::vector<StageStatus>`"
    purpose: "Bounded status per stage."
    used_for: "Error handling, stage audit."
    memory: "~24 B + capacity"
  - name: "`profiling`"
    type: "`FrameProfiling` із `dp1.domain.profiling`"
    purpose: "Frame-level timing, bounded trace, cardinality і memory metrics."
    used_for: "Performance analysis, runtime summaries, validation evidence."
    memory: "bounded by `dp1.config.application.profiling`"
  - name: "`diagnostics`"
    type: "`std::vector<DiagnosticMessage>`"
    purpose: "Structured warnings/errors."
    used_for: "Debug without image payloads."
    memory: "~24 B + capacity"
```

## Пам'ять

`FrameContext` є small metadata object. Він не володіє image buffers і не має
масштабуватися з розміром кадру. Його diagnostics і profiling fields мають бути
bounded через `dp1.config.application.profiling`,
`dp1.config.application.logging` або runtime limits.

## Етапи

`FrameContext` передається всім stages як context, але output stages мають бути
явними domain structures:

```text
FramePacket + FrameContext
  -> TileDesc[]
  -> TileRawView + TileContext per worker
  -> TileResult[]
```

## Interpretation

`FrameContext` дозволяє передавати службову інформацію між stages без забруднення domain objects. Він потрібен для tracing, profiling, diagnostics, route selection і відтворюваності.

`FrameContext` може містити тимчасовий per-frame runtime state, але не має ставати контейнером для algorithm outputs.
Він також не має ставати загальним mutable context для кількох камер; multi-camera
aggregation належить downstream boundary або orchestration layer.

## Failure cases

- Image buffers або stage outputs ховаються в context.
- Context змішує diagnostics/status для frames різних камер як один DP1 input.
- Context використовується як заміна configuration `C`.
- Stage silently mutates route/config semantics через context.
- Diagnostics записуються як unstructured strings без можливості audit.
- Profiling trace росте без configured bound.
- Profiling field використовується для прихованого перенесення image buffers або
  algorithm outputs.

## Typical misuse

- Використовувати `FrameContext` як global mutable state.
- Зберігати candidates або measurements у context замість explicit stage output.

## Open questions

- Чи потрібен immutable config snapshot або достатньо config reference.
- Error taxonomy для stage-level diagnostics.

## Connections

- belongs_to: dp1.domain.runtime
- accompanies: dp1.domain.raw.frame_packet
- constrains: dp1.pipeline.stage_contract
- references: dp1.config.pipeline_configuration_c
- uses: dp1.domain.pixel_format
- uses: dp1.domain.profiling
- configured_by: dp1.config.application.profiling
- constrained_by: dp1.domain.identity
- constrained_by: dp1.domain.time
