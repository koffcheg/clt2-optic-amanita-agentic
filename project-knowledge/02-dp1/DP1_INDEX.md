# DP1_INDEX

## Призначення

Це вхідна точка knowledge base для DP1 (`datapro1`).
Файл маршрутизує до структурних карток та основних джерел коду DP1.

## Поточний вміст розділу

- `DP1_CARDS_INDEX.md` - індекс DP1-карток
- `cards/*.md` - атомарні картки структур та інтерфейсів DP1
- `dp1_v2/` - робоча документація по новій версії DP1 (AMNT-0006)
	- `dp1_v2/DP1_V2_PLAN_MONO8_MONO16.md`
	- `dp1_v2/DP1_V2_PERF_MEMORY_BOTTLENECKS.md`
	- `dp1_v2/PHASE_TRACKER.md`
	- `dp1_v2/CHANGE_LOG.md`

## Що вже покрито

1. Основні типи даних DP1 (`TData*`, `TOptionsMeasurement`, `TDrawMeasurement`, `TFolder`).
2. Frame processing boundary (`frame_n_header`, `frame_processor`).
3. Runtime буфери/локальні структури ingest і multithreading.
4. Конфігураційний шар DP1.
5. Preprocessing stage (sum-binning feature toggle).
6. RPC serialization primitives і DP1 -> DP2 transport.
7. File output contracts (`.blob`, `.json`, naming).
8. Runtime lifecycle/orchestration contracts (`ipc_data_rc_impl`, `run_ipc_src`, `run_uri_src`, `dp1_main`).
9. Manual validation сценарій для AMNT-0004 у `project-knowledge/05-validation/`.

## Що лишається додати далі

1. За потреби винести shared wire/file contracts у `04-protocols/`.
2. Синхронізувати додаткові зв'язки DP1 <-> DP2 при розширенні DP2 knowledge.

## Рекомендований маршрут читання

1. `DP1_CARDS_INDEX.md`
2. `cards/*.md`
3. `dp1_v2/DP1_V2_PLAN_MONO8_MONO16.md` (для задач DP1_v2)
4. `dp1_v2/DP1_V2_PERF_MEMORY_BOTTLENECKS.md` (для performance-driven ітерацій)
5. `datapro1/src/*`
6. `datapro1/config/*`
