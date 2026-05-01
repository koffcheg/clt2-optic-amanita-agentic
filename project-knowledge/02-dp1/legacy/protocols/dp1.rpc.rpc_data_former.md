---
id: dp1.rpc.rpc_data_former
title:
  uk: "rpc_data_former - фреймінг TCP повідомлень (black-box API)"
  en: "rpc_data_former - TCP message framing (black-box API)"
tags: [dp1, datapro1, rpc, protocol, tcp, dependency]
kind: legacy-reference-card
source_role: legacy-reference
source:
  file: "datapro1/src/dp1_tr_res2dp2.cpp"
  lines: "17-70"
status: "draft"
---

## Definition
`rpc_data_former` - зовнішня залежність (header `m_rpc_d_former.h` не входить у архів DP1), яка формує “raw TCP message bytes” з payload.

DP1 використовує:
- `data_former.reset()`
- `data_former.form_next_msg(payload_ptr, payload_size) -> raw_msg`

## Assumptions
- `form_next_msg` додає необхідні заголовки/довжину/CRC (що саме - визначено у зовнішньому модулі).
- Повернений `raw_msg` містить **повне** повідомлення, яке можна одним `boost::asio::write` відправити у сокет.
- `reset()` переводить стан у “початковий” (важливо при реконекті, щоб не було “половинчастого” фрейму).

## Theorem / Contract
- На кожен payload з `CMemStore` викликається `form_next_msg(ms.data(), ms.size())`.
- Результат пишеться в TCP як суцільний буфер. При помилці запису сокет закривається, а data_former буде ресетнуто при наступному успішному конекті.

## Interpretation
Це шар між “структурованими байтами payload” і “TCP стрімом”. Він відповідає за те, щоб DP2 міг відновлювати межі повідомлень у потоці.

## Failure cases
- Якщо DP2 очікує інший framing, то він не зможе знайти межі повідомлень.
- Якщо `reset()` не викликається при реконекті - DP2 може отримати некоректний stateful framing (наприклад, continuation).

## Typical misuse
- Обходити `rpc_data_former` і писати payload напряму у TCP (DP2 тоді не зможе парсити).
- Використовувати один `rpc_data_former` з кількома сокетами/потоками без синхронізації.

## Connections
- used_by: dp1.net.send_res_to_dp2 (tr_msg_to_dp2)
- wraps: dp1.rpc.CMemStore payload
- depends_on: dp1.net.dp1_to_dp2_message_types (payload починається з msg_type)
