---
id: dp2.runtime.server
title:
  uk: "dp2::server - асинхронний TCP acceptor DP2"
  en: "dp2::server - asynchronous DP2 TCP acceptor"
tags: [dp2, runtime, server, tcp, asio]
kind: legacy-reference-card
source_role: legacy-reference
source:
  file: "datapro2/src/dp2_svr.h"
  lines: "15-31"
status: "draft"
---

## Definition
`dp2::server` інкапсулює listen/accept lifecycle для DP2: слухає TCP порт, створює `session` на кожне підключення, контролює stop через sentinel timer.

## Assumptions
- `cl_fabric_` повертає валідний `rpc_sink` для кожної нової сесії.
- `stop_work_func_` є безпечним для періодичного виклику з io_context callbacks.

## Theorem / Contract
- Після успішного accept створюється `session` і відразу стартує `start()`.
- `do_accept()` реєструє наступний accept, поки не спрацює stop/cancel.
- При stop викликається `acceptor_.cancel()` і сервер припиняє прийом.

## Interpretation
Це entry-point runtime boundary для мережевого прийому DP1 -> DP2.

## Failure cases
- Помилки accept/cancel можуть обірвати прийом нових з'єднань.
- Некоректна фабрика sink-ів ламає downstream parsing path.

## Typical misuse
- Передавати фабрику, що реюзає один `rpc_sink` на багато сесій без синхронізації.

## Open questions
- Чи потрібна explicit policy для max concurrent sessions.

## Connections
- creates: dp2.runtime.session
- uses: dp2.rpc.dp2_rpc_cl
- related_to: dp2.net.dp1_to_dp2_receive_path
