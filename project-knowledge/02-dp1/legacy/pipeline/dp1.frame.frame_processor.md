---
id: dp1.frame.frame_processor
title:
  uk: "frame_processor - інтерфейс обробника кадрів"
  en: "frame_processor - frame processing interface"
tags: [dp1, class, interface, frame]
kind: legacy-reference-card
source_role: legacy-reference
source:
  file: "datapro1/src/dp1_frame_proc.h"
  lines: "21-29"
status: "draft"
---

## Definition
Абстрактний інтерфейс (polymorphic), який реалізує один метод:
`proc_next_frame(frame_n_header rc_frame)`.

## Assumptions
- Виклики `proc_next_frame` йдуть послідовно з головного потоку runner-а (IPC/URI), якщо в коді не сказано інше.
- Реалізація сама вирішує, чи використовувати багатопоточність на рівні тайлів.

## Theorem / Contract
- Реалізація повинна бути exception-safe: помилка обробки кадра не повинна “валити” процес без контрольованого логування/stop.
- Реалізація не повинна блокувати надовго, якщо upstream вимагає realtime.
- Якщо preprocessing-фіча увімкнена (через конфіг), вона застосовується перед основним DP1 pipeline.
- Якщо preprocessing змінює scale кадра, координати вимірів мають бути повернуті в СК початкового кадра до експорту/передачі далі.

## Interpretation
Це основна “точка розширення” DP1: новий CV-конвеєр або preprocessing-фіча реалізуються в межах цього boundary з керуванням через конфіг.

## Failure cases
- Довгі блокування → переповнення буфера кадрів у runner.
- Невірна обробка типів Mat (16U vs 8U) → некоректні маски/метрики.

## Typical misuse
- Робити I/O (запис відео, великі PNG) прямо в `proc_next_frame` без throttling.
- Зберігати pointers з `frame_n_header` після повернення.

## Connections
- factory: dp1.frame.get_fr_processor
- input: dp1.frame.frame_n_header
- uses: dp1.config.prg_config, dp1.types.TDataproConfig/TDataproVar
- preprocessing: dp1.preproc.binning_sum
- validated_by: validation.amnt0004.dp1_binning
