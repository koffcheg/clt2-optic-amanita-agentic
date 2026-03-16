---
id: dp2.rpc.dp2_rpc_cl
title:
  uk: "dp2_rpc_cl - RPC sink обробки повідомлень DP1"
  en: "dp2_rpc_cl - RPC sink for DP1 message handling"
tags: [dp2, rpc, sink, dp1, receive]
source:
  file: "datapro2/src/dp2_rpc_cl.h"
  lines: "15-21"
status: "draft"
---

## Definition
`dp2_rpc_cl` реалізує `rpc_sink` і виконує payload-level dispatch повідомлень DP1 за `msg_type` (`new_measure` / `camera_calibration`) у `on_rd_msg_complite`.

## Assumptions
- Вхідний `data,len` уже є повним framed RPC payload (після обробки lower-layer у `rpc_sink`).
- Першим полем payload завжди є `uint16_t msg_type`.

## Theorem / Contract
- Для `dp1_to_dp2_rpc_msg_new_measure` викликається `on_new_dp1_meas`.
- Для `dp1_to_dp2_camera_calibration_data` викликається `on_camera_cfg`.
- Невідомий `msg_type` логуються як помилка без аварійного завершення процесу.

## Interpretation
Це message dispatcher boundary між wire-level RPC framing і доменною логікою DP2 (tracking/calibration state).

## Failure cases
- Невалідний payload layout зміщує курсор `CMemStore` і псує парсинг.
- Відсутність попередньої camera calibration може впливати на інтерпретацію вимірів.

## Typical misuse
- Додавати новий `msg_type` у DP1 без синхронного оновлення dispatch у DP2.

## Open questions
- Чи потрібна explicit policy на невідому версію payload/schema.

## Connections
- implements: dp2.runtime.session
- part_of: dp2.net.dp1_to_dp2_receive_path
- overlaps_with: dp1.net.dp1_to_dp2_message_types
- overlaps_with: dp1.rpc.serialize_dp1_res
