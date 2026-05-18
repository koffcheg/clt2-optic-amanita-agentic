Змін не робив. Нижче той самий аналіз українською.

**Короткий висновок**

Так, тут є системне розходження. `ПОВНЕ ВЕЛИКЕ ТЗV20.docx` задає `FrameContext` як єдиний контейнер стану кадру, де мають бути відображені raw frame, processing representations, masks, contours/components, objects, measurements, profiling і config metadata.

Поточна canonical база знань переозначила `FrameContext` майже протилежно: як малий runtime/context metadata object, який супроводжує input/output, але не є контейнером stage outputs.

Це не дрібна різниця в назвах. Це дві різні архітектурні моделі stage-I/O:

- ТЗ: `process(input, context, config) -> output`, де `FrameContext` є спільним контейнером даних кадру, і будь-яка модифікація/поява даних має бути відображена в context.
- Canonical: `process(input, context, config) -> output`, де `context` є лише runtime/config/profiling/diagnostics metadata, а primary output має бути explicit structure поза `FrameContext`.
- Код `datapro1_v2`: ближчий до canonical, ніж до ТЗ. Stage-інтерфейси повертають `StageOutcome<SpecificOutput>`, а `FrameContext` містить тільки metadata/status/diagnostics.

**Головний конфлікт**

У ТЗ, розділ 10.3, `FrameContext` має містити:

- raw frame;
- обчислювальні представлення;
- маски;
- контури / компоненти;
- об’єкти;
- вимірювання;
- `StageTiming`;
- метадані конфігурації.

І прямо заборонено:

- приховане дублювання даних поза context;
- модифікацію даних без відображення у context.

Canonical прямо забороняє таку модель:

- `FrameContext` передається між stages разом із input/output data objects, але не замінює їх.
- `FrameContext` не має містити primary image buffers, masks, candidates, segments або measurements як hidden payload.
- Stage output має бути explicit domain structure.
- Context означає runtime/config context, а не output container.
- Використання `FrameContext` як загального mutable output container позначене як misuse.

Фактичний код також не відповідає ТЗ 10.3:

- `FrameContext` у C++ містить `frame_id`, `camera_id`, `source_id`, `config_ref`, geometry, acquisition time, `stage_statuses`, `diagnostics`.
- У ньому немає raw frame, processing frames, masks, candidates, segments, objects, measurements, `StageTiming` або `FrameProfiling`.
- Radiometric stage повертає `RadiometricFullFrameOutput{ProcessingFrame}`, але не записує/не реєструє цей результат у `FrameContext`.
- Поточний pipeline виконує тільки radiometric stage і потім будує порожній result через `build_empty_result(frame_context)`.

**Таблиця: FrameContext**

| ТЗ-контракт | Canonical-контракт | Фактичний код |
|---|---|---|
| `FrameContext` — єдиний контейнер даних кадру. | `FrameContext` — runtime/context metadata object. | `FrameContext` — metadata struct: ids, source, config ref, geometry, time, statuses, diagnostics. |
| Містить raw frame. | Не володіє primary image buffers; raw іде як `FramePacket`. | Raw frame зберігається у `FramePacket`, не в `FrameContext`. |
| Містить processing representations. | Processing outputs мають бути explicit `ProcessingFrame` / `TileProcessingFrame`. | `RadiometricFullFrameOutput` містить `ProcessingFrame`; context не оновлюється. |
| Містить masks. | Masks explicit: `BinaryMask` / `TileBinaryMask`; не hidden payload. | Інтерфейс candidate extraction повертає mask окремо; у context її немає. |
| Містить contours/components. | Components/segments explicit: `Segment[]`. | Інтерфейс segmentation повертає `Segment[]`; у context їх немає. |
| Містить objects. | Objects explicit: `ValidatedObject[]`. | Інтерфейс object filtering повертає `ValidatedObject[]`; у context їх немає. |
| Містить measurements. | Measurements explicit: `MeasurementRecord[]`. | Інтерфейс measurement повертає measurements; runtime stage ще не викликається. |
| Містить profiling / `StageTiming`. | Canonical очікує profiling як metadata, але не payload. | У C++ `FrameContext` немає `profiling`; є тільки statuses/diagnostics. |
| Забороняє hidden duplication outside context. | Забороняє протилежне: hidden outputs inside context. | Stage outputs живуть поза context у `StageOutcome<T>`. |

**Таблиця: Stage-I/O**

| Stage | ТЗ-контракт | Canonical-контракт | Фактичний код |
|---|---|---|---|
| Prep | Організує ROI/tiles/border/coordinates; результат має бути частиною або відображенням стану кадру в context. | `FramePacket -> FramePacket view / TileDesc[] / TileRawView[]`; context тільки runtime/config. | Є тільки інтерфейс. Повертає explicit `PrepFullFrameOutput` / `PrepTilesOutput`. |
| Radiometric | Формує background/residual; processing representation має бути відображений у `FrameContext`. | Output explicit `ProcessingFrame`; raw input не мутується, primary output не пишеться в context. | Реалізовано. Повертає `ProcessingFrame`, але `context` ігнорується. |
| Enhancement | Підвищує SNR; output `CV_8U/CV_32F` має бути відображений у context. | Output explicit `ProcessingFrame`; context не output container. | Є тільки інтерфейс. Повертає `EnhancementFullFrameOutput`. |
| Matched filtering | Формує detector response map; це processing representation у моделі ТЗ. | Output explicit `ProcessingFrame(DetectorResponse)`. | Є тільки інтерфейс. Response повертається поза context. |
| Candidate extraction | Формує mask і hypotheses; masks входять до `FrameContext` за ТЗ. | Output explicit `BinaryMask` + `Candidate[]`. | Є тільки інтерфейс. Mask/candidates повертаються поза context. |
| Segmentation | Формує contours/components. | Output explicit `Segment[]`; context не зберігає segments. | Є тільки інтерфейс. Повертає `Segment[]`. |
| Object filtering | Формує список objects. | Output explicit `ValidatedObject[]`. | Є тільки інтерфейс. Objects повертаються поза context. |
| Measurement | Формує `MeasurementRecord`. | Output explicit `MeasurementRecord[]`; final product/handoff. | Є тільки інтерфейс. Runtime поки не викликає measurement і публікує empty result. |

**Де саме ТЗ очікує “мутацію контексту”**

ТЗ не пише це як C++-сигнатуру `context.setX(...)`, але це прямо випливає з вимог:

1. Розділ 10.2 задає `process(input, context, config) -> output`.
2. Розділ 10.3 називає `FrameContext` єдиним контейнером даних.
3. Там же перелічує всі проміжні й фінальні продукти pipeline як вміст `FrameContext`.
4. Там же забороняє модифікацію даних без відображення у context.
5. Розділ 10.7 дозволяє state тільки в context/config.
6. Розділ 8.3 вимагає інтегрувати profiling у `FrameContext / StageTiming`.
7. Розділ 13.4 вимагає стандартизувати `FrameContext`, `MeasurementRecord`, `StageProfile`, `StageTiming`.

Отже, модель ТЗ така: кожен stage може мати output, але всі authoritative stage products мають бути відображені у `FrameContext`: residual, enhanced frame, detector response, mask, candidates, segments/components, validated objects, measurements, timings, conversion/copy profiling.

Canonical зламав саме цей центр: output chain існує, context існує, але context не є frame data state.

**Класифікація розходжень**

- `ТЗ -> canonical`: canonical gap / semantic deviation. Canonical база знань побудована не як пряме відображення ТЗ 10.3, а як альтернативна explicit-output архітектура.
- `canonical -> code`: часткова відповідність. Код іде за explicit-output підходом, але ще не реалізує повний stage chain і canonical profiling.
- `ТЗ -> code`: runtime deviation. Код не має єдиного `FrameContext`-контейнера даних, не відображає radiometric output у context, не має `StageTiming` у context, не виконує всі stages 1-8 як повний pipeline.

**Мінімальне примирення**

Потрібне архітектурне рішення:

1. Якщо ТЗ 10.3 є головним target, треба переробляти canonical `FrameContext`, stage contract, stage I/O matrix і stage headers під context-owned або context-indexed frame state.
2. Якщо canonical explicit-output модель вважається правильною, треба явно зафіксувати, що ТЗ 10.3 було переінтерпретовано, і оновити requirements trace.
3. Компромісний варіант: `FrameContext` не володіє важкими buffers, але містить typed registry/references на всі authoritative stage products плюс `StageTiming`. Тоді виконується вимога ТЗ “відображення у context”, але зберігається canonical заборона на приховані payloads. Зараз такого registry шару немає ні в canonical, ні в коді.
