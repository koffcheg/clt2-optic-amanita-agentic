---
id: dp1.io.dp1_output_filenames
title:
  uk: "Іменування вихідних файлів DP1 (.blob/.json)"
  en: "DP1 output file naming (.blob/.json)"
tags: [dp1, datapro1, io, naming]
kind: legacy-reference-card
source_role: legacy-reference
source:
  file: "datapro1/src/dataproSaveToFile.cpp"
  lines: "8-29, 98-115"
status: "draft"
---

## Definition
Правила побудови імен файлів результатів DP1, які кодують `cam_index`, час (UTC) і/або `index_frame`.

## Assumptions
- Використовується `std::gmtime(&t_current)` -> час у назві є **UTC**, не локальний.
- `cam_index` завжди форматується як 3 цифри з leading zeros.
- `index_frame` форматується як 6 цифр з leading zeros.

## Theorem / Contract
Є два варіанти імені:

1) Rolling filename (містить розширення в середині формування):
`C{cam:03}_D{yy}{mm}{dd}_T{HH}{MM}{SS}.blob`

2) Single-frame base filename (без дати у фінальному рядку, дата закоментована в коді):
`C{cam:03}_F{frame:06}`

Функція `creating_file_name(t_current, cam, frame, name_file)` заповнює `name_file` другим форматом.

## Interpretation
Назва файлу - ключ для трасованості (яка камера, який кадр). Важливо, що rolling варіант містить timestamp, а single-frame - frame index.

## Failure cases
- Якщо `index_frame` виходить за межі 6 цифр - формат “ламається” (буде ширше поле).
- Якщо `cam_index` > 999 - теж.
- Використання UTC може плутати людей при ручному аналізі (але добре для системної синхронізації).

## Typical misuse
- Змішувати rolling і single-frame у одному каталозі без маркерів -> важко автоматично парсити.
- Додавати локальний час у назву і потім порівнювати з UTC timestamps у метаданих.

## Connections
- used_by: dp1.io.save_res_blob, dp1.io.save_res_json
- uses: dp1.types.TFolder (output dir = folder_name_dp1.data_bin)
