---
id: dp1.domain.frame_context
title:
  uk: "Контекст кадру canonical DP1"
  en: "Canonical DP1 frame context"
tags: [dp1, canonical, data-domain, frame-context]
kind: data-domain-card
source_role: canonical
source:
  file: "project-knowledge/02-dp1/canonical/data_domains/dp1.domain.frame_context.md"
  lines: "1-N"
status: "draft"
---

## Definition

`FrameContext` є canonical runtime/context object, який супроводжує обробку одного кадру або frame-derived packet між етапами DP1.

Він відповідає на питання: у якому pipeline/runtime контексті обробляється кадр.

## Assumptions

- `FrameContext` передається між stages разом із input/output data objects, але не замінює їх.
- `FrameContext` не є pipeline configuration `C`.
- Фактична C++ форма context має бути підтверджена окремою implementation task.

## Theorem / Contract

`FrameContext` має мінімально містити або посилатися на:

- `frame_id` — ідентифікатор кадру, узгоджений із `FramePacket`.
- `pipeline_run_id` — ідентифікатор pipeline run або processing session.
- `config_ref` або config snapshot reference — посилання на active configuration `C`.
- `pixel_route` — selected route such as `U8`, `U16`, або `F32`.
- `stage_status` — bounded map/list статусів stages для поточного кадру.
- `profiling_trace` — накопичувач timing/profiling events.
- `warnings` / `errors` — structured diagnostics для поточного кадру.
- optional `source_metadata_ref` — посилання на camera/source metadata.

`FrameContext` не має містити primary image buffers, masks, candidates, segments або measurements як hidden payload.

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

- Exact representation of profiling trace.
- Чи потрібен immutable config snapshot або достатньо config reference.
- Error taxonomy для stage-level diagnostics.

## Connections

- accompanies: dp1.domain.frame_packet
- constrains: dp1.pipeline.stage_contract
- references: dp1.config.pipeline_configuration_c
- uses: dp1.domain.pixel_format
