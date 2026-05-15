---
id: dp1.config.application.source
title:
  uk: "Canonical-конфігурація джерела кадрів DP1"
  en: "Canonical DP1 frame source configuration"
tags: [dp1, canonical, config, application, source, runtime]
kind: config-card
source_role: canonical
source:
  file: "project-knowledge/02-dp1/canonical/configuration/dp1.config.application.source.md"
status: "draft"
---

## Definition

`ApplicationConfig.source` визначає runtime-джерело кадрів для одного DP1 instance.

Ця секція відповідає на питання: звідки application layer отримує кадр перед створенням canonical `FramePacket`.

## Assumptions

- Один DP1 instance має один active frame source.
- `ApplicationConfig.source` не визначає алгоритмічний `input_route`; pixel format, bit depth і range policy залишаються в `PipelineConfig.input_route`.
- Перший codegen slice використовує тільки `file` source mode.
- `camerapro` і `camerapro_sim` є майбутніми source modes і не входять у першу codegen-ітерацію.

## Theorem / Contract

`ApplicationConfig.source` конфігурує source adapter, який створює або передає в DP1 canonical `FramePacket`.

Базова форма JSON authoring:

```json
{
  "source": {
    "mode": "file",
    "file": {
      "path": "<input-file-or-folder>",
      "recursive": false,
      "repeat": false
    }
  }
}
```

Дозволені `mode` значення:

- `file` — файловий input adapter для першої codegen-ітерації і demo replay.
- `camerapro_sim` — майбутній adapter для CameraProSim через transport boundary.
- `camerapro` — майбутній adapter для реального CameraPro source.

Typed canonical config model:

```cpp
enum class FrameSourceMode {
    File,
    CameraProSim,
    CameraPro
};

struct FileSourceConfig {
    std::string path;
    bool recursive = false;
    bool repeat = false;
};

struct SourceConfig {
    FrameSourceMode mode = FrameSourceMode::File;
    FileSourceConfig file;
};
```

## Fields / Interface

```yaml
fields:
  - name: "`mode`"
    type: "`FrameSourceMode`"
    required: true
    purpose: "Вибір runtime source adapter."
    affects: "Який adapter створює або приймає input frame перед `FramePacket`."
    does_not_affect: "Не вибирає pixel format, bit depth, stage variants або algorithm behavior."
    validation: "Для першої codegen-ітерації дозволений тільки `file`."

  - name: "`file.path`"
    type: "`std::string`"
    required_when: "`mode == file`"
    purpose: "Файл або папка з кадрами для deterministic input replay."
    validation: "Має бути non-empty authoring value; filesystem existence перевіряється runtime або validation run."

  - name: "`file.recursive`"
    type: "`bool`"
    required_when: "`mode == file`"
    default: false
    purpose: "Дозвіл обходу підпапок для folder replay."

  - name: "`file.repeat`"
    type: "`bool`"
    required_when: "`mode == file`"
    default: false
    purpose: "Дозвіл циклічного replay input frames."
```

## Input / Output

Input:

- application configuration authoring file;
- filesystem path для `file` source;
- `PipelineConfig.input_route` як окрема source-of-truth для pixel/range metadata.

Output:

- typed `SourceConfig`;
- selected source adapter;
- input frame payload для створення `FramePacket`.

## Constraints

- `ApplicationConfig.source` не має дублювати `PipelineConfig.input_route`.
- `ApplicationConfig.source` не має визначати `stage.variant`, `stage.level` або stage parameters.
- `file` source не має імітувати CameraPro transport semantics.
- `camerapro_sim` і `camerapro` потребують окремих task cards перед implementation.
- Source adapter не має створювати masks, candidates, segments або measurements.

## Interpretation

Для першої codegen-ітерації `ApplicationConfig.source.mode = "file"` задає тільки спосіб отримати кадр з файлу або папки. Canonical `FramePacket` все одно має відповідати `dp1.domain.raw.frame_packet`, а input pixel/range metadata мають узгоджуватись із `PipelineConfig.input_route`.

## Failure cases

- Source config додається в `PipelineConfig C` замість `ApplicationConfig`.
- `file` source самостійно вирішує bit depth/range policy без `PipelineConfig.input_route`.
- Source adapter починає виконувати binning, candidate extraction або measurement.
- CameraProSim або IPC реалізуються в першій file-source ітерації.

## Typical misuse

- Вважати `source.mode = file` алгоритмічним pipeline variant.
- Додавати CameraPro transport fields у `file` section.
- Використовувати `ApplicationConfig.source` як місце для threshold або processing parameters.

## Connections

- belongs_to: dp1.config.application
- separates_from: dp1.config.pipeline_configuration_c
- uses: dp1.domain.raw.frame_packet
- constrained_by: dp1.domain.pixel_format
- constrained_by: dp1.domain.memory_ownership
