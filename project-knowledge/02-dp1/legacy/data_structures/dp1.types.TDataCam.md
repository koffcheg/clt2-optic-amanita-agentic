---
id: dp1.types.TDataCam
title:
  uk: "TDataCam - метадані камери"
  en: "TDataCam - camera metadata"
tags: [dp1, struct, camera, metadata]
kind: legacy-reference-card
source_role: legacy-reference
source:
  file: "datapro1/src/datarpoTypes.h"
  lines: "38-41"
status: "draft"
---

## Definition
Ідентифікація камери та (опційно) її положення у просторі в координатах системи/установки.

**Поля:**
- `cam_index` (int): індекс/ID камери у системі.
- `Xcam, Ycam, Zcam` (double): координати камери (одиниці доменно визначені).

## Assumptions
- `cam_index` стабільний у всіх логах/файлах/каналах (DP1, DP2, збереження).
- Координатна система для X/Y/Z узгоджена з бінокулярним профілем (якщо використовується).

## Theorem / Contract
- Пара (`cam_index`, `index_frame`) достатня для унікальної прив’язки результатів до джерела.

## Interpretation
Мінімальний “ідентифікатор джерела кадра” для багатокамерних сценаріїв.

## Failure cases
- Переплутані `cam_index` між потоками → некоректна агрегація/трекинг.

## Typical misuse
- Використовувати `Xcam/Ycam/Zcam` як валідні без ініціалізації (за замовчуванням 0).

## Connections
- part_of: dp1.types.TDataRes
- used_by: dp1.save.save_res (ідентифікація при збереженні)
