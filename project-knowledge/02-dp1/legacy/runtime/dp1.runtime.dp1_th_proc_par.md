---
id: dp1.runtime.dp1_th_proc_par
title:
  uk: "dp1_th_proc_par - параметри багатопотокової обробки тайлів"
  en: "dp1_th_proc_par - multithread tile processing params"
tags: [dp1, struct, runtime, multithreading]
kind: legacy-reference-card
source_role: legacy-reference
source:
  file: "datapro1/src/datapro1.cpp"
  lines: "169-222 (локальна структура в .cpp)"
status: "draft"
---

## Definition
Локальна структура, яка агрегує всі посилання/ресурси, потрібні робочим потокам DP1 для паралельної обробки тайлів одного кадра.

Включає:
- бар’єр старту і флаги (`start_proc_cv`, `start_proc_mut`, `start_proc_flag`)
- чергу тайлів (`tiles_to_proc`)
- лічильник готових потоків (`num_thread_ready`)
- посилання на дані кадра/конфіги/буфери (`tmp_frame`, `data_param`, `var`, `segment`, `filters`, `KernelGauss`, ...)

Вкладена структура:
- `tile_id { int i; int j; }` - індекси тайла в сітці.

## Assumptions
- Усі вказівники посилаються на об’єкти, що живуть довше за “итерацію” обробки кадра.
- Черга `tiles_to_proc` заповнена до старту потоків на ітерацію.
- Є зовнішня домовленість про thread-safety для `TDataproVar` (бо `vec_frag/vec_bgmask` модифікуються).

## Theorem / Contract
- `dispatch_calc(th_index)` бере тайли з черги і викликає `datapro1_calc_func(i,j, ...)` поки черга не порожня.
- `queue_mut` робить pop з черги потокобезпечним, але не захищає інші спільні структури.

## Interpretation
Це “контейнер залежностей” для паралельного виконання однієї й тієї ж функції на різних тайлах.

## Failure cases
- Data race на `var.vec_*` або `data_draw/vec_meas` якщо `datapro1_calc_func` пише в спільні структури без ізоляції по індексу тайла.
- Dangling pointers при повторному вході або зміні кадра під час обробки.

## Typical misuse
- Додавати нові поля без явної політики ownership/thread-safety.
- Робити `static` mutex-и з прихованою глобальною синхронізацією, що псує масштабування.

## Connections
- uses: dp1.types.TDataproConfig, dp1.types.TDataproVar, dp1.config.prg_config.cfg_segment/cfg_filters
- calls: dp1.methods.datapro1_calc_func (в datapro1.cpp)
