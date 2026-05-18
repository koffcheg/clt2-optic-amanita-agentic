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

Вона відповідає на два питання:

- у якому pipeline/runtime context обробляється кадр;
- які авторитетні artifacts кадру вже існують для цього кадру, ким вони
  створені, від яких artifacts походять і як довго на них можна посилатися.

`FrameContext` не є прихованим власником усіх даних кадру. Його обов'язкова
семантика - runtime context разом із шаром реєстру/provenance для artifacts.

## Assumptions

- `FrameContext` передається між stages разом із input/output data objects, але не замінює їх.
- `FrameContext` не є pipeline configuration `C`.
- Конкретна C++ форма context має визначатися окремою implementation task.
- `FrameContext` супроводжує один frame/source tuple і має бути узгоджений із
  `FramePacket.camera_id` / `FramePacket.source_id`.
- Поточний стиль `StageOutcome<T>` / explicit-output залишається чинним: stage
  outputs і далі повертаються як явні domain structures.
- Кожен успішно створений авторитетний stage product має також бути
  відображений у `FrameContext.artifacts`.
- Записи реєстру у першій реалізації можуть бути metadata-only. Вони можуть
  описувати artifact і його provenance без typed pointer.
- Записи реєстру не мають подовжувати час життя borrowed memory.

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
- `artifacts` — `FrameArtifactRegistry`, тобто обмежений шар реєстру/provenance
  для авторитетних frame artifacts.

`FrameContext` не має містити primary image buffers, masks, candidates, segments
або measurements як hidden payload. Натомість він має відображати їх появу як
авторитетні artifacts через записи реєстру.

Обов'язкові правила узгодження:

- явні виходи етапів залишаються обов'язковими;
- відображення в context є обов'язковим для авторитетних artifacts кадру;
- володіння важкими буферами з боку `FrameContext` не вимагається за
  замовчуванням.

Словник artifacts на рівні knowledge:

```cpp
enum class ArtifactKind : std::uint8_t {
    RawFrame,
    ProcessingFrame,
    BinaryMask,
    CandidateSet,
    SegmentSet,
    ValidatedObjectSet,
    MeasurementSet,
    Visualization,
    Diagnostics,
    Profiling
};

enum class ArtifactDomain : std::uint8_t {
    Raw,
    Processing,
    Mask,
    Struct,
    Measurement,
    Visualization,
    Runtime
};

enum class ArtifactOwnership : std::uint8_t {
    OwnedByFramePacket,
    OwnedByStageOutput,
    BorrowedReadOnly,
    ExternalTransport,
    MetadataOnly,
    ExpiredReference
};

enum class ArtifactLifetime : std::uint8_t {
    InputBoundary,
    StageOutputScope,
    FrameBoundary,
    Persisted,
    MetadataOnly
};

enum class ArtifactStatus : std::uint8_t {
    Available,
    MetadataOnly,
    Expired,
    Failed
};

struct FrameArtifactRef {
    std::string artifact_id;
    ArtifactKind kind;
    ArtifactDomain domain;
    ArtifactOwnership ownership;
    ArtifactLifetime lifetime;
    ArtifactStatus status;
    std::string producer_stage;
    std::string semantic_name;
    std::optional<std::string> parent_artifact_id;
};

struct FrameArtifactRegistry {
    std::vector<FrameArtifactRef> artifacts;
};
```

Визначення:

- `FrameArtifactRegistry` є frame-scoped реєстром для авторитетних artifacts
  і provenance records.
- `FrameArtifactRef` є knowledge-level reference/record для одного artifact. Він
  не зобов'язаний зберігати payload цього artifact.
- `ArtifactKind` визначає роль artifact у pipeline.
- `ArtifactDomain` визначає canonical DP1 data domain для artifact.
- `ArtifactOwnership` фіксує, хто володіє payload, або чи є запис реєстру
  metadata-only.
- `ArtifactLifetime` фіксує максимальний valid lifetime для referenced або described data.
- `ArtifactStatus` фіксує, чи payload доступний для прямого використання, чи запис є лише metadata/provenance.
- `producer_stage` є stable stage key або runtime boundary, який створив або
  зареєстрував artifact.
- `semantic_name` є stable human-readable name, наприклад `raw_frame`,
  `radiometric_residual`, `candidate_mask` або `measurement_records`.
- `parent_artifact_id` пов'язує artifact із source artifact, від якого він
  походить, якщо цей зв'язок відомий.

`ArtifactOwnership::MetadataOnly` і `ArtifactStatus::MetadataOnly` означають, що
registry record описує artifact і provenance, але не містить live typed payload
reference. `ArtifactOwnership::OwnedByStageOutput` із
`ArtifactLifetime::StageOutputScope` не подовжує lifetime локального stage output
object. Його не можна тлумачити як дозвіл для `FrameContext` володіти всіма
heavy `cv::Mat` buffers.

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
    FrameProfiling profiling; // мінімально P2 StageTiming у поточному C++ slice
    FrameArtifactRegistry artifacts;
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
  - name: "`artifacts`"
    type: "`FrameArtifactRegistry`"
    purpose: "Відображення авторитетних frame artifacts через реєстр/provenance."
    used_for: "Аудит кадру, відтворюваність, traceability stage outputs, узгодження з ТЗ 10.3."
    memory: "обмежений vector/list metadata records; не має масштабуватися з кількістю pixels"
  - name: "`diagnostics`"
    type: "`std::vector<DiagnosticMessage>`"
    purpose: "Structured warnings/errors."
    used_for: "Debug without image payloads."
    memory: "~24 B + capacity"
```

Поля artifact:

```yaml
artifact_fields:
  - name: "`artifact_id`"
    purpose: "Унікальна frame-local або run-stable identity для artifact."
  - name: "`kind`"
    purpose: "Роль у pipeline: raw frame, processing frame, mask, candidates, segments, objects, measurements, diagnostics, profiling."
  - name: "`domain`"
    purpose: "Canonical data domain, який обмежує artifact."
  - name: "`ownership`"
    purpose: "Payload ownership/reflection policy; запобігає прихованому подовженню lifetime."
  - name: "`lifetime`"
    purpose: "Максимальний valid lifetime для referenced або described data."
  - name: "`producer_stage`"
    purpose: "Stable producer stage/runtime boundary для provenance."
  - name: "`semantic_name`"
    purpose: "Stable name для audit і human inspection."
  - name: "`status`"
    purpose: "Чи є payload доступним, metadata-only, expired або failed."
  - name: "`parent_artifact_id`"
    purpose: "Optional provenance link до source artifact."
```

## Пам'ять

`FrameContext` є bounded runtime metadata + registry object. Він не володіє
image buffers за замовчуванням і не має масштабуватися з кількістю pixels у
кадрі. Його diagnostics, profiling і artifact registry fields мають бути
bounded через `dp1.config.application.profiling`,
`dp1.config.application.logging` або runtime limits.

Записи реєстру можуть посилатися на artifacts або описувати artifacts, payload
яких належить `FramePacket`, stage output object, tile/runtime context або
persistence boundary. Запис реєстру не подовжує borrowed memory lifetime. Якщо
`ownership = BorrowedView`, consumers мають дотримуватися referenced input/stage
boundary lifetime з `dp1.domain.memory_ownership`.

Семантика профілювання регулюється `PROFILING_POLICY.md` і
`dp1.domain.profiling`. Поточний C++ baseline реалізує мінімальний P2
`StageTiming` у `FrameContext.profiling.stage_timings`; operation-level timing,
cardinality metrics, aggregation і reports лишаються окремим future scope.
`FrameContext.profiling` і повʼязані з профілюванням записи artifacts не мають
вводити окрему модель профілювання.

Порядок джерел для правил профілювання:

1. `project-knowledge/00-governance/PROFILING_POLICY.md`;
2. `dp1.domain.profiling`;
3. `dp1.config.application.profiling`.

`FrameContext.artifacts` не є заміною для `FrameContext.profiling`,
`FrameProfiling`, `StageTiming`, `OperationTiming`, `CardinalityMetrics` або
інших структур із `dp1.domain.profiling`. Реєстр artifacts може містити
метадані походження, наприклад stable `producer_stage` або ідентичність запису
часу/профілювання, коли profiling реалізовано, але не має переносити
алгоритмічні виходи між stages і не має створювати окремий канал збереження
даних профілювання.

## Етапи

`FrameContext` передається всім stages як context, але output stages мають бути
явними domain structures. Після успішного створення авторитетного output stage
або runtime boundary має зареєструвати відповідний artifact record у
`FrameContext.artifacts`.

```text
FramePacket + FrameContext
  -> TileDesc[]
  -> TileRawView + TileContext per worker
  -> TileResult[]
```

Приклад маршруту artifacts:

```text
FramePacket.image
  -> зареєструвати artifact raw_frame
  -> explicit output ProcessingFrame
  -> зареєструвати artifact radiometric_residual
  -> explicit output BinaryMask / Candidate[]
  -> зареєструвати artifacts candidate_mask / candidates
  -> explicit output MeasurementRecord[]
  -> зареєструвати artifact measurement_records
```

## Interpretation

`FrameContext` дозволяє передавати службову інформацію між stages без забруднення domain objects. Він потрібен для tracing, profiling, diagnostics, route selection, artifact provenance і відтворюваності.

`FrameContext` може містити тимчасовий per-frame runtime state і registry
records для авторитетних algorithm outputs, але не має ставати прихованим
контейнером heavy payloads.
Він також не має ставати загальним mutable context для кількох камер; multi-camera
aggregation належить downstream boundary або orchestration layer.

## Failure cases

- Image buffers або stage outputs ховаються в context.
- Авторитетний stage output повертається явно, але не відображається у
  `FrameContext.artifacts`.
- Запис реєстру посилається на borrowed memory після завершення її valid lifetime.
- Запис реєстру не має producer/provenance information.
- Context змішує diagnostics/status для frames різних камер як один DP1 input.
- Context використовується як заміна configuration `C`.
- Stage silently mutates route/config semantics через context.
- Diagnostics записуються як unstructured strings без можливості audit.
- Profiling trace росте без configured bound.
- Profiling field використовується для прихованого перенесення image buffers або
  algorithm outputs.
- Усередині artifact registry створюється нова profiling model замість
  використання `PROFILING_POLICY.md`, `dp1.domain.profiling` і
  `dp1.config.application.profiling`.

## Typical misuse

- Використовувати `FrameContext` як global mutable state.
- Зберігати candidates або measurements у context замість explicit stage output.
- Трактування metadata-only artifact records так, ніби вони володіють payload lifetime.
- Трактування `OwnedByStageOutput + StageOutputScope + MetadataOnly` як live payload після завершення local stage output lifetime.
- Реєстрація кожного temporary local buffer замість авторитетних frame
  products.

## Open questions

- Чи потрібен immutable config snapshot або достатньо config reference.
- Error taxonomy для stage-level diagnostics.
- Точний C++ storage і lookup API для `FrameArtifactRegistry`.
- Чи artifact ids мають бути frame-local strings, interned ids або numeric ids.

## Connections

- belongs_to: dp1.domain.runtime
- accompanies: dp1.domain.raw.frame_packet
- constrains: dp1.pipeline.stage_contract
- references: dp1.config.pipeline_configuration_c
- uses: dp1.domain.pixel_format
- uses: dp1.domain.profiling
- uses: PROFILING_POLICY.md
- configured_by: dp1.config.application.profiling
- constrained_by: dp1.domain.identity
- constrained_by: dp1.domain.time
- constrained_by: dp1.domain.memory_ownership
