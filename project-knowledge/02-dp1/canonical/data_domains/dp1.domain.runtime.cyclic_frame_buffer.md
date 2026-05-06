---
id: dp1.domain.runtime.cyclic_frame_buffer
title: "Циклічний буфер кадрів DP1"
tags: [dp1, canonical, data-domain, runtime, structure, buffer, frame]
kind: data-domain-card
source_role: canonical
source:
  file: "project-knowledge/02-dp1/canonical/data_domains/dp1.domain.runtime.cyclic_frame_buffer.md"
status: "draft"
---

## Definition

`CyclicFrameBuffer` є канонічною runtime-структурою для зберігання обмеженого
вікна кадрів або кадроподібних payloads із перезаписом за кільцевим індексом.

Ця структура описує повторно використовуваний runtime-стан. Вона не є виходом
етапу, не є payload між етапами і не замінює `FramePacket`, `ProcessingFrame` або
`TileProcessingFrame`.

## Assumptions

- Буфер належить конкретному етапу або варіанту алгоритму, який явно описує
  політику відбору кадрів.
- Усі slots мають однакову geometry, pixel format, OpenCV type і route metadata.
- `capacity` задається під час конструювання або `reset()` і не змінюється у
  гарячому шляху.
- Конкретна C++ форма може бути окремим helper або приватним станом реалізації
  етапу.
- Якщо алгоритм потребує хронологічного порядку, він має явно запросити
  хронологічний view або iterator.

## Theorem / Contract

`CyclicFrameBuffer` має мінімально містити або посилатися на:

- `capacity` — кількість slots у кільцевому вікні, `capacity >= 1`;
- `slots` — попередньо виділені буфери кадрів одного розміру і типу;
- `next_slot` — індекс slot, який буде перезаписаний наступним записом;
- `filled_count` — кількість валідно заповнених slots, не більша за `capacity`;
- optional `logical_frame_index` або `selected_frame_index` — індекс потоку або
  індекс відібраного кадру, якщо це потрібно stage spec;
- optional metadata для geometry, pixel format, input bit depth і range policy.

Правило запису:

```text
slots[next_slot] <- input_frame
next_slot <- (next_slot + 1) mod capacity
filled_count <- min(filled_count + 1, capacity)
```

Повне вікно є валідним тільки коли:

```text
filled_count == capacity
```

Усі slots мають бути виділені під час конструювання або `reset()`. Запис у
існуючий slot не має створювати новий буфер кадру в гарячому шляху.

Фізичний порядок `slots[0..capacity-1]` не є хронологічним контрактом. Алгоритм
може читати фізичні slots напряму тільки якщо його результат не залежить від
порядку кадрів у вікні.

## Поля

| Поле | Тип | Навіщо | Для яких обчислень | Пам'ять |
|---|---|---|---|---|
| `capacity` | `int` | Розмір обмеженого вікна. | Часові фільтри, моделі фону, історія кадрів. | 4 B |
| `slots` | array/vector буферів кадрів | Володіє пам'яттю кадрів вікна. | Доступ до останніх або відібраних кадрів. | `capacity * frame payload` |
| `next_slot` | `int` | Вказує наступний slot для перезапису. | O(1) додавання з перезаписом. | 4 B |
| `filled_count` | `int` | Відстежує прогрів і валідність повного вікна. | Заборона читання неповного вікна. | 4 B |
| `logical_frame_index` | optional `uint64` | Зв'язок із потоком кадрів. | Валідація, trace, політика відбору. | 8 B |
| `selected_frame_index` | optional `uint64` | Зв'язок із відібраними кадрами. | Часове прорідження, diagnostics. | 8 B |

## Пам'ять

`CyclicFrameBuffer` володіє пам'яттю зображень для своїх slots. Для full-frame route
його вартість масштабується так:

```text
capacity * width * height * bytes_per_pixel
```

Для tile-local або ROI route вартість має рахуватися від geometry відповідної
одиниці обробки, а не від повного кадру.

`clear()` або логічний reset можуть скидати `next_slot` і `filled_count` без
звільнення вже виділених slots, якщо `capacity`, geometry і pixel type не
змінюються. Зміна `capacity`, geometry або pixel type потребує явної reset /
reallocation policy.

## Interpretation

Ця структура потрібна для stateful temporal algorithms, які мають bounded
пам'ять і повторно використовують кадри з попередніх моментів часу.

`CyclicFrameBuffer` визначає тільки контракт пам'яті й lifecycle. Він не визначає:

- які кадри потрапляють у буфер;
- як обчислюється median, average, background або інший результат;
- який output повертає stage;
- чи вікно читається у фізичному або хронологічному порядку.

Ці правила мають задаватися stage spec або окремою algorithm card.

## Failure cases

- Читання повного вікна до `filled_count == capacity`.
- Припущення, що `slots[0]` є найстарішим або найновішим кадром без явного
  хронологічного mapping.
- Зміна geometry, pixel type або `capacity` без reset / reallocation policy.
- Прихована allocation у гарячому шляху через запис у непідготовлений slot.
- Shared mutable access до одного буфера з кількох workers без synchronization
  contract.
- Використання cyclic buffer як виходу етапу замість явного domain object.

## Typical misuse

- Трактувати `CyclicFrameBuffer` як replacement для `ProcessingFrame`.
- Передавати internal slots між stages як canonical payload.
- Використовувати physical slot order у temporal algorithm, де потрібний
  chronological order.
- Викликати `shrink_to_fit()` або звільняти slots у гарячому шляху.

## Open questions

- Чи потрібен один shared helper API для всіх stage implementations, чи достатньо
  canonical contract і приватних implementation states.
- Стандартна форма chronological iterator/view для алгоритмів, чутливих до
  порядку кадрів.
- Політика ownership для tile-local cyclic buffers у майбутньому parallel route.

## Connections

- belongs_to: dp1.domain.runtime
- uses: dp1.domain.pixel_format
- may_store: dp1.domain.raw.frame_packet
- may_store: dp1.domain.processing.frame
- may_store: dp1.domain.processing.tile_processing_frame
- used_by: dp1.stage_spec.radiometric_correction.inverse_median
- constrains: dp1.pipeline.stage_domain_bindings
