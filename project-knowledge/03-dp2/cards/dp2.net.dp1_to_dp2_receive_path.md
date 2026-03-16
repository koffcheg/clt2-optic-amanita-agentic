---
id: dp2.net.dp1_to_dp2_receive_path
title:
  uk: "Receive path DP1 -> DP2 (TCP stream -> framed msg -> payload dispatch)"
  en: "DP1 to DP2 receive path (TCP stream to framed message to payload dispatch)"
tags: [dp2, net, rpc, protocol, dp1]
source:
  file: "datapro2/src/dp2_ses.cpp"
  lines: "26-63"
status: "draft"
---

## Definition
Контракт receive-path у DP2: TCP stream читається `session`, передається в `rpc_sink::on_next_raw_read`, після reassembly викликається `dp2_rpc_cl::on_rd_msg_complite`, де payload dispatch виконується за `msg_type`.

## Assumptions
- Фреймінг/збірка повідомлень у stream виконується зовнішнім `rpc_sink`-шаром.
- Payload layout узгоджений із DP1 serialization (`serialize_dp1_res`, camera calibration payload).

## Theorem / Contract
- Рівень `session` не знає про message boundaries payload і працює з raw bytes.
- Рівень `dp2_rpc_cl` працює з повним повідомленням і читає `msg_type` як перше поле.
- На signal `need_br_conn` транспортний рівень сесії розриває з'єднання.

## Interpretation
Це формальний міст між transport-рівнем (ASIO stream) та доменним receive-рівнем DP2.

## Failure cases
- Framing mismatch між DP1/DP2 руйнує receive path навіть при валідному TCP.
- ABI/layout mismatch у payload структурах дає неправильну десеріалізацію.

## Typical misuse
- Парсити raw stream як готовий payload без framing stage.
- Змінювати порядок полів payload у DP1 без синхронізації DP2 dispatch/deserialization.

## Open questions
- Чи потрібне явне versioning поле payload для керованої еволюції контракту.

## Connections
- uses: dp2.runtime.server
- uses: dp2.runtime.session
- uses: dp2.rpc.dp2_rpc_cl
- overlaps_with: dp1.net.dp1_to_dp2_message_types
- overlaps_with: dp1.rpc.serialize_dp1_res
- overlaps_with: dp1.rpc.serialize_camera_calibration_data
