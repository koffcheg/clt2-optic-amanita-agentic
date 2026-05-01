---
id: dp1.net.dp1_tr_res2dp2_connection
title:
  uk: "send_res_to_dp2 / init_connect_to_dp2 - канал передачі DP1 -> DP2 (TCP)"
  en: "send_res_to_dp2 / init_connect_to_dp2 - DP1 -> DP2 transport channel (TCP)"
tags: [dp1, datapro1, tcp, dp2, transport]
kind: legacy-reference-card
source_role: legacy-reference
source:
  file: "datapro1/src/dp1_tr_res2dp2.cpp"
  lines: "31-112"
status: "draft"
---

## Definition
Модуль, що відповідає за:
- з’єднання з DP2 по TCP (`boost::asio`),
- реконект з інтервалом,
- відправку двох типів повідомлень: calibration та measurements.

## Assumptions
- Взаємодія з DP2 відбувається через один сокет, який може бути `nullptr` якщо не підключено.
- `is_program_stop()` використовується для заборони реконекту при зупинці програми.
- Після успішного конекту DP1 одразу відправляє calibration.

## Theorem / Contract
- `init_connect_to_dp2(host, port, reconn_sec, cam_index, cam_settings)`:
  - зберігає параметри,
  - створює `io_context`,
  - робить `connect_to_dp2()`.
- `connect_to_dp2()`:
  - підключається,
  - ставить `tcp::no_delay(true)`,
  - робить `data_former.reset()`,
  - викликає `tr_camera_settings()`.
- `send_res_to_dp2(data)`:
  - якщо нема сокета - пробує реконект,
  - формує payload (`msg_type + serialize_dp1_res`) і шле через `rpc_data_former`.

## Interpretation
Це “мережевий вихід” етапу DP1, який робить DP1 частиною конвеєра (DP1 -> DP2).

## Failure cases
- Немає DP2 або порт недоступний -> `dp2socket == nullptr` і `send_res_to_dp2` тихо повертається.
- Помилка запису -> сокет закривається, буде реконект пізніше.
- `cam_settings` - raw pointer на зовнішні дані; якщо вони знищені раніше - UB при `tr_camera_settings`.

## Typical misuse
- Передавати тимчасовий `TDataCalibrationCamera` у `init_connect_to_dp2` (потрібен стабільний lifetime).
- Викликати `send_res_to_dp2` з кількох потоків без синхронізації (використовується static `CMemStore`).

## Connections
- depends_on: dp1.rpc.CMemStore, dp1.rpc.rpc_data_former
- msg_types: dp1.net.dp1_to_dp2_message_types
- payload: dp1.rpc.serialize_dp1_res, dp1.rpc.serialize_camera_calibration_data
