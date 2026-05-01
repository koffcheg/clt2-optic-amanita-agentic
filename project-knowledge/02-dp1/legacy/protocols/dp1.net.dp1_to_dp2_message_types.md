---
id: dp1.net.dp1_to_dp2_message_types
title:
  uk: "dp1_to_dp2_* - типи повідомлень DP1 -> DP2 (msg_type)"
  en: "dp1_to_dp2_* - DP1 -> DP2 message types (msg_type)"
tags: [dp1, datapro1, rpc, protocol, dp2]
kind: legacy-reference-card
source_role: legacy-reference
source:
  file: "datapro1/src/dp1_tr_res2dp2.cpp"
  lines: "72-101"
status: "draft"
---

## Definition
`msg_type` - перше поле payload у `CMemStore` перед даними. DP1 використовує щонайменше два типи:

- `dp1_to_dp2_camera_calibration_data`
- `dp1_to_dp2_rpc_msg_new_measure`

Фактичні значення констант визначені у `rpc_msg_defines.h` (заголовок не входить у архів DP1), але **порядок та семантика** видно з коду відправника.

## Assumptions
- DP2 читає перше значення як `msg_type` і маршрутизує парсер payload.
- `msg_type` є “native integer” і однаковий за розміром/endianness на обох сторонах.

## Theorem / Contract
- Для `dp1_to_dp2_camera_calibration_data` DP1 пише: `msg_type`, `cam_index`, `serialize_camera_calibration_data(...)`.
- Для `dp1_to_dp2_rpc_msg_new_measure` DP1 пише: `msg_type`, `serialize_dp1_res(...)`.

## Interpretation
Це мінімальний “RPC dispatcher”: один TCP канал, багато типів повідомлень, payload залежить від `msg_type`.

## Failure cases
- DP2 читає `msg_type` іншим типом/розміром -> з’їде курсор і далі все буде некоректним.
- Додати новий `msg_type` без реалізації парсера на стороні DP2 -> ігнорування/краш.

## Typical misuse
- Вважати, що “всі повідомлення однакові” і намагатись завжди парсити як `TDataRes`.
- Переплутати порядок `cam_index` і calibration payload.

## Connections
- used_by: dp1.net.tr_camera_settings, dp1.net.send_res_to_dp2
- payloads: dp1.rpc.serialize_camera_calibration_data, dp1.rpc.serialize_dp1_res
- framed_by: dp1.rpc.rpc_data_former
