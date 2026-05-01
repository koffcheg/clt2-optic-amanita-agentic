---
id: dp1.rpc.serialize_Mat
title:
  uk: "serialize_Mat / deserialize_Mat - двійковий формат cv::Mat для RPC"
  en: "serialize_Mat / deserialize_Mat - cv::Mat binary wire format for RPC"
tags: [dp1, datapro1, rpc, serialization, opencv]
kind: legacy-reference-card
source_role: legacy-reference
source:
  file: "datapro1/src/dp1_rpc_data_mrsh.cpp"
  lines: "8-37"
status: "draft"
---

## Definition
Пара функцій, що визначає **wire-format** для `cv::Mat` при передачі DP1 -> DP2 через `CMemStore`:
1) заголовок (cols, rows, elemSize, type),
2) байти пікселів (рядками або одним блоком).

## Assumptions
- `img.type()` та `img.elemSize()` є достатніми, щоб відновити матрицю у DP2.
- `CMemStore` зберігає байти **без модифікації** і підтримує послідовний курсор читання/запису.
- Для `deserialize_Mat` припускається, що в `CMemStore` лежить суцільний блок даних матриці.

## Theorem / Contract
- **Serialize:** пише (int cols, int rows, int elemSizeInBytes, int elemType), а потім `cols*rows*elemSize*channels` байтів. Якщо `img.isContinuous()`, запис відбувається як один рядок.  
- **Deserialize:** читає заголовок, створює `cv::Mat(rows, cols, elemType, ms.getCurrPtr())`, **просуває курсор** на `cols*rows*elemSizeInBytes`, і робить `copyTo(img)`.

## Interpretation
Це мінімальний контракт “як байти кадра/матриці їдуть по дроту”. Він повинен бути **ідентичним** у DP1 і DP2, інакше з’являться ABI/endianness/size mismatch проблеми.

## Failure cases
- Невідповідність `elemType`/`elemSize` між версіями OpenCV/компіляторами -> биті дані або crash.
- `deserialize_Mat`: якщо `CMemStore` не має достатньо байтів, `ms.getCurrPtr()` стане невалідним.
- `img.channels()` у Serialize врахований у `bytes_to_write`, але `elemSizeInBytes` вже включає канали (у OpenCV). Якщо змінити реалізацію формули - легко зламати сумісність.

## Typical misuse
- Додавати ще поля в заголовок без версіонування повідомлення.
- Плутати “розмір елемента” і “розмір пікселя/каналів” та змінювати формулу `bytes_to_write`.

## Connections
- used_by: dp1.rpc.serialize_camera_calibration_data, dp1.rpc.serialize_frame_calibration_data
- used_by: dp1.rpc.serialize_dp1_res (через серіалізацію калібрування)
- transport: dp1.rpc.CMemStore
