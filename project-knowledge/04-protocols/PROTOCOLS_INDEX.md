# PROTOCOLS_INDEX

## Призначення

Цей файл має стати canonical index для shared contracts між модулями проєкту: message types, wire format, serialization rules, file exchange boundaries, IPC/TCP handoff.

## Що сюди відносити
- shared message types між DP1 і DP2
- serialization contracts
- framing rules
- binary file exchange contracts, якщо вони shared або мають міжмодульне значення
- обмеження portability, compatibility і versioning

## Що не слід тримати лише тут
- чисто локальну алгоритмічну логіку DP1 або DP2
- деталі UI, visualization або build system

## Початкові джерела
- `datapro1/src/dp1_tr_res2dp2.cpp`
- `datapro1/src/dp1_rpc_data_mrsh.cpp`
- receive path у `datapro2/src/*`
- релевантні DP1-картки по `dp1.rpc.*` і `dp1.net.*`
