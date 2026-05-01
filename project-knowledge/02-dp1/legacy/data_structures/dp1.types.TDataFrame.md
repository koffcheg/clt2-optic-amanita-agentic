---
id: dp1.types.TDataFrame
title:
  uk: "TDataFrame - метадані кадра"
  en: "TDataFrame - frame metadata"
tags: [dp1, datapro1, struct, frame, metadata]
kind: legacy-reference-card
source_role: legacy-reference
source:
  file: "datapro1/src/datarpoTypes.h"
  lines: "7-22"
status: "draft"
---

## Definition
C-структура метаданих одного кадра, яку DP1/DP2 використовує як “паспорт кадра”: індекси, час експозиції, геометрія кадра та (за наявності) дані турелі.

**Поля (семантика):**
- `index_frame` (int): порядковий номер кадра у потоці.
- `dp1_spent_time` (uint32 ms): час обробки кадра модулем DP1.
- `exposureStart` (DateTime): час початку експозиції (system_clock).
- `width`, `height` (int): розмір кадра у пікселях.
- `exposureLength` (s): тривалість експозиції.
- `focalLength` (m): фокусна відстань.
- `pixelWidth`, `pixelHeight` (m): розмір пікселя.
- `El`, `Az` (float): кутові координати/орієнтація (елев./азимут) з турелі/системи наведення.
- `V_el`, `V_az` (float): кутові швидкості.
- `turretInfoValid` (bool): чи валідні поля турелі.

## Assumptions
- `width/height` узгоджені з фактичним `cv::Mat` кадром, який обробляється.
- Одиниці вимірювання **не змінюються** по пайплайну (секунди/метри/градуси або радіани мають бути узгоджені глобально).
- Якщо `turretInfoValid == false`, то значення `El/Az/V_el/V_az` **не можна** використовувати у розрахунках.

## Theorem / Contract
- `dp1_spent_time` вимірюється в **мілісекундах** і відноситься до обробки **цього** кадра.
- Кадр ідентифікується комбінацією (`cam_index` з TDataCam, `index_frame`).

## Interpretation
Це “шапка” кадра для логування, збереження результатів, синхронізації з зовнішніми модулями (DP2, трекінг, архівація).

## Failure cases
- `width/height` не відповідають `cv::Mat` → помилки ROI, тайлінгу, збереження.
- Невідома конвенція для `El/Az` (deg vs rad, знак, нульова вісь) → геометричні помилки в наступних модулях.
- `exposureStart` у локальному часі замість UTC → некоректна синхронізація.

## Typical misuse
- Використовувати `El/Az` без перевірки `turretInfoValid`.
- Перезаписувати `index_frame` “для зручності”, втрачаючи трасованість.

## Connections
- part_of: dp1.types.TDataRes
- complements: dp1.frame.frame_n_header (метадані можуть приходити з cam_pro::FrameHeader)
- produced_by: dp1.runtime.rc_ipc_raw_frame / dp1.runtime.rc_uri_raw_frame (джерела кадрів)
