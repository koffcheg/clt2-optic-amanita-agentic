---
id: dp2.runtime.session
title:
  uk: "dp2::session - runtime сесія TCP читання"
  en: "dp2::session - TCP read runtime session"
tags: [dp2, runtime, session, tcp, asio]
source:
  file: "datapro2/src/dp2_ses.h"
  lines: "8-33"
status: "draft"
---

## Definition
`dp2::session` обробляє життєвий цикл одного TCP-з'єднання: async read raw bytes, передача байтів у `rpc_sink`, heartbeat/stop контроль через sentinel timer.

## Assumptions
- Вхідні дані можуть приходити фрагментовано; framing/reassembly виконує `rpc_sink` (`on_next_raw_read`).
- `need_br_conn` з `rpc_sink` сигналізує про необхідність обриву поточного з'єднання.

## Theorem / Contract
- `start()` запускає одночасно: read-loop і timer-loop.
- `on_read()` передає прийняті байти в `rpc_client_->on_next_raw_read(...)`.
- При stop або помилці сесія скасовує сокет/таймер і завершується.

## Interpretation
Це runtime boundary між TCP stream transport і RPC message-level parsing.

## Failure cases
- Read error або EOF завершує сесію.
- Неконсистентний state у sink може вимагати форсований break connection.

## Typical misuse
- Вважати один `async_read_some` еквівалентом одного логічного RPC повідомлення.

## Open questions
- Чи потрібне обмеження per-session buffer growth у sink при malformed input.

## Connections
- created_by: dp2.runtime.server
- uses: dp2.rpc.dp2_rpc_cl
- part_of: dp2.net.dp1_to_dp2_receive_path
