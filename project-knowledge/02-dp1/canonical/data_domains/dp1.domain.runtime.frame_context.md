---
id: dp1.domain.runtime.frame_context
title: "Runtime-контекст кадру DP1"
tags: [dp1, canonical, data-domain, runtime, structure, frame-context]
kind: data-domain-card
source_role: canonical
source:
  file: "project-knowledge/02-dp1/canonical/data_domains/dp1.domain.runtime.frame_context.md"
status: "draft"
---

## Definition

`FrameContext` є canonical runtime/context structure, яка супроводжує обробку одного кадру або похідного від кадру пакета між етапами DP1.

Вона відповідає на питання: у якому pipeline/runtime context обробляється кадр.

## Assumptions

- `FrameContext` передається між stages разом із input/output data objects, але не замінює їх.
- `FrameContext` не є pipeline configuration `C`.
- Конкретна C++ форма context має визначатися окремою implementation task.

## Theorem / Contract

`FrameContext` має мінімально містити або посилатися на:

- `frame_id` — ідентифікатор кадру, узгоджений із `FramePacket`.
- `pipeline_run_id` — ідентифікатор pipeline run або processing session.
- `config_ref` або config snapshot reference — посилання на active configuration `C`.
- `pixel_route` — selected route, наприклад `U8`, `U16` або `F32`.
- `stage_status` — bounded map/list статусів stages для поточного кадру.
- `profiling_trace` — накопичувач timing/profiling events.
- `warnings` / `errors` — structured diagnostics для поточного кадру.
- optional `source_metadata_ref` — посилання на camera/source metadata.

`FrameContext` не має містити primary image buffers, masks, candidates, segments або measurements як hidden payload.

Рекомендована C++ форма:

```cpp
struct FrameContext {
    std::uint64_t frame_id = 0;
    std::string pipeline_run_id;
    const PipelineConfig *config_ref = nullptr;
    int worker_count = 0;
    PixelFormat input_format = PixelFormat::U16;
    InputBitDepth input_bit_depth = InputBitDepth::Bit16;
    std::vector<StageStatus> stage_statuses;
    std::vector<ProfileEvent> profiling_trace;
    std::vector<DiagnosticMessage> diagnostics;
};
```

## Поля

| Поле | Тип | Навіщо | Для яких обчислень | Пам'ять |
|---|---|---|---|---|
| `frame_id` | `std::uint64_t` | Зв'язок із `FramePacket`. | Validation, trace. | 8 B |
| `pipeline_run_id` | `std::string` | Ідентифікатор запуску pipeline. | Reproducibility, logs. | ~24 B + payload |
| `config_ref` | `const PipelineConfig*` | Active configuration `C`. | Stage parameters, route selection. | 8 B |
| `worker_count` | `int` | Кількість tile workers для кадру. | Profiling, memory accounting. | 4 B |
| `input_format` | `PixelFormat` | Route-level input carrier. | Перевірка, що stage route узгоджений із frame. | 4 B |
| `input_bit_depth` | `InputBitDepth` | Route-level input bit depth. | Threshold/range validation. | 4 B |
| `stage_statuses` | `std::vector<StageStatus>` | Bounded status per stage. | Error handling, stage audit. | ~24 B + capacity |
| `profiling_trace` | `std::vector<ProfileEvent>` | Frame-level timing events. | Performance analysis. | ~24 B + capacity |
| `diagnostics` | `std::vector<DiagnosticMessage>` | Structured warnings/errors. | Debug without image payloads. | ~24 B + capacity |

## Пам'ять

`FrameContext` є small metadata object. Він не володіє image buffers і не має
масштабуватися з розміром кадру. Його vector fields мають бути bounded policy
через config або runtime limits.

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

## Failure cases

- Image buffers або stage outputs ховаються в context.
- Context використовується як заміна configuration `C`.
- Stage silently mutates route/config semantics через context.
- Diagnostics записуються як unstructured strings без можливості audit.

## Typical misuse

- Використовувати `FrameContext` як global mutable state.
- Зберігати candidates або measurements у context замість explicit stage output.

## Open questions

- Exact representation для profiling trace.
- Чи потрібен immutable config snapshot або достатньо config reference.
- Error taxonomy для stage-level diagnostics.

## Connections

- belongs_to: dp1.domain.runtime
- accompanies: dp1.domain.raw.frame_packet
- constrains: dp1.pipeline.stage_contract
- references: dp1.config.pipeline_configuration_c
- uses: dp1.domain.pixel_format
- constrained_by: dp1.domain.identity
- constrained_by: dp1.domain.time
