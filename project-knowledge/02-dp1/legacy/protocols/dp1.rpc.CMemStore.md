---
id: dp1.rpc.CMemStore
title:
  uk: "CMemStore - буфер байтів з курсором (black-box API)"
  en: "CMemStore - byte buffer with cursor (black-box API)"
tags: [dp1, datapro1, rpc, buffer, dependency]
kind: legacy-reference-card
source_role: legacy-reference
source:
  file: "datapro1/src/dp1_tr_res2dp2.cpp; datapro1/src/dp1_rpc_data_mrsh.cpp"
  lines: "dp1_tr_res2dp2.cpp:58-99; dp1_rpc_data_mrsh.cpp:8-156"
status: "draft"
---

## Definition
`CMemStore` - зовнішня залежність (header `mem_store.h` не входить у архів DP1), яка використовується як:
- буфер накопичення payload,
- курсор для послідовного читання/запису,
- джерело `data()`/`size()` для фреймінгу повідомлення.

## Assumptions
- Методи `write_native<T>(...)` і `read_native<T>(...)` пишуть/читають **native layout** (endianness/packing як на платформі).
- `setPosition(0)` скидає позицію курсора запису/читання.
- `getCurrPtr()` повертає pointer на поточну позицію читання.
- `getCurrFreeLimit()` повертає кількість байтів, які ще можна читати.

## Theorem / Contract
DP1 використовує такий підмножинний API:
- `ms.setPosition(0)`
- `ms.write_native(...)`, `ms.read_native(...)`
- `ms.write(void* ptr, size_t nbytes)`
- `ms.read(void* dst, size_t nbytes, bool advance=...)`
- `ms.getCurrPtr()`, `ms.getCurrFreeLimit()`
- `ms.data()`, `ms.size()`

## Interpretation
`CMemStore` - “контейнер для wire-format”. Усі контракти серіалізації DP1 -> DP2 спираються на те, що `CMemStore` стабільний та не змінює байти.

## Failure cases
- Якщо `write_native(struct)` робить не raw-copy, а щось інше -> wire-format ламається.
- Якщо `read(nullptr, n, false)` поводиться не так (наприклад, не просуває курсор) -> `deserialize_Mat` зламається.
- Недостатній розмір буфера -> неповні повідомлення або out-of-bounds.

## Typical misuse
- Змішувати читання/запис в одному `CMemStore` без явного `setPosition`.
- Вважати, що `write_native` робить “portable serialization”. Це **не portable**, це native ABI.

## Connections
- used_by: dp1.rpc.serialize_* (Mat, calibration, res)
- framed_by: dp1.rpc.rpc_data_former
- sent_over: dp1.net.send_res_to_dp2
