# Диалог

## Пользователь

Структуры данных для внутрикадровой оброботки компьютерного зрения. Есть вот такое ТЗ, там есть етапы, мне нужно понять каждый етап, его теорию. Найди материалы и свежие статьи по этому поводу.

## Ассистент

Я сначала привяжу ответ к этапам из твоего ТЗ, затем подберу внешние материалы: отдельно базовую теорию по каждому этапу и отдельно свежие статьи/обзоры, которые можно использовать для углубления.

## Ассистент

Уже видно, что в ТЗ “структуры данных” не отдельная тема сбоку, а скелет пайплайна: FrameContext, форматы Mat/масок/измерений, тайлы, компоненты, кандидаты и профили времени должны обслуживать каждый этап, а не жить отдельно.

## Ассистент

В твоём ТЗ DP1 описан не как один алгоритм, а как configurable pipeline: этапы + конфигурация + структуры данных. Ключевая идея: сначала нужно понять, какие данные живут между этапами, а уже потом - какие алгоритмы применять. В ТЗ это прямо выражено через FrameContext, MeasurementRecord, StageTiming/StageProfile, домены Raw / Processing / Mask / Measurement / Visualization и последовательность Prep → Radiometric correction → Enhancement → Matched filtering → Candidate extraction → Segmentation → Object filtering → Measurement. fileciteturn2file0

## 1. Что именно изучать по структурам данных

Главная структура - это не просто `cv::Mat`. `cv::Mat` хранит изображение или промежуточную карту, но полноценная архитектура DP1 должна иметь несколько уровней данных: сырой кадр, ROI/тайлы, residual, response map, binary mask, label map/contours, object candidates, measurement records и profiling info.

Для базы по `cv::Mat`, ROI и памяти начни с OpenCV Mat tutorial. Важный момент: ROI в OpenCV можно создать как новый заголовок над областью исходного изображения, без обязательного копирования пикселей, например через `Mat D(A, Rect(...))`; это напрямую связано с твоим этапом Prep и требованием “минимум копий”. ([docs.opencv.org](https://docs.opencv.org/3.4/d6/d6d/tutorial_mat_the_basic_image_container.html))

Также обязательно изучи тему типов данных изображений. В твоём ТЗ правильно выделены домены `CV_16U`, `CV_32F`, `CV_8U`, потому что неконтролируемые переходы между ними могут ломать фотометрию. В документации scikit-image отдельно подчёркивается, что в pipeline стоит явно управлять dtype-конвертациями, а слепое растягивание интенсивностей может исказить изображение и сделать шум похожим на полезный сигнал. ([scikit-image.org](https://scikit-image.org/docs/0.24.x/user_guide/data_types.html))

## 2. Теория по каждому этапу DP1

| Этап из ТЗ | Что это теоретически | Какие структуры данных нужны |
|---|---|---|
| **1. Prep** | Подготовка кадра: ROI, тайлы, border, преобразование координат local/global. Это не “обработка сигнала”, а организация данных перед обработкой. | `FrameContext`, `TileDescriptor`, `RoiDescriptor`, `BorderPolicy`, `CoordinateMapper`, ссылки/представления на `cv::Mat` без лишних копий. |
| **2. Radiometric correction** | Оценка фона и построение residual: `residual = frame - background`. Теория: подавление низкочастотного фона, background estimation, median/mean/Gaussian background, band-pass. | `BackgroundModel`, `ResidualFrame`, `PhotometryDomain`, `ConversionRecord`. Тут критично работать в `CV_32F` или другом signed/float формате, чтобы не потерять отрицательные значения residual. |
| **3. Enhancement / denoising** | Повышение SNR перед детекцией: Gaussian, median, bilateral, DoG, guided/multiscale filtering. Это этап улучшения сигнала, но ещё не детектор. | `EnhancedFrame`, `NoiseStats`, `LocalContrastMap`, `SNRStats`. |
| **4. Matched / quasi-matched filtering** | Детекция сигнала через согласование с шаблоном/PSF/ядром. На выходе не изображение “для красоты”, а карта отклика детектора. | `KernelDescriptor`, `PSFModel`, `ResponseMap`, `PeakMap`. OpenCV `filter2D` и `matchTemplate` - базовые материалы по этому этапу. ([docs.opencv.org](https://docs.opencv.org/4.x/d4/d86/group__imgproc__filter.html)) |
| **5. Candidate extraction** | Превращение response/residual/enhanced map в первичные гипотезы: threshold, adaptive threshold, Otsu, absdiff, MOG2/KNN. | `CandidateMask`, `ThresholdConfig`, `CandidateStats`. Для adaptive threshold важно понимать, что порог считается локально по окрестности, а не один раз для всего кадра. ([docs.opencv.org](https://docs.opencv.org/4.x/d7/dd0/tutorial_js_thresholding.html)) |
| **6. Segmentation refinement** | Очистка маски и выделение объектов: morphology open/close, connected components, contours. Это переход от пиксельной маски к объектным регионам. | `LabelMap`, `ComponentRecord`, `ContourRecord`, `MorphologyTrace`. OpenCV поддерживает connected components с 4/8-связностью и разными алгоритмами CCL. ([docs.opencv.org](https://docs.opencv.org/4.x/d3/dc0/group__imgproc__shape.html)) |
| **7. Object filtering** | Отсев ложных объектов по площади, форме, яркости, compactness, aspect ratio, фотометрическим признакам. | `ObjectCandidate`, `RejectReason`, `FilterTrace`, `ShapeFeatures`, `PhotometryFeatures`. |
| **8. Measurement** | Формирование финальных измерений для DP2: centroid, bbox, moments, intensity mean/max, координаты local/global. | `MeasurementRecord`, `MeasurementList`, `PhotometryMeasurement`, `CoordinateSystem`. В scikit-image `regionprops` хорошо показывает набор типовых признаков: area, bbox, centroid, weighted centroid, intensity_mean, moments, Hu moments и т.д. ([scikit-image.org](https://scikit-image.org/docs/stable/api/skimage.measure.html?highlight=label)) |
| **9. Visualization** | Отдельный домен отображения. Не должен влиять на расчёты. | `VisualizationFrame`, `OverlayRecord`. |
| **10. Persistence / profiling** | Сохранение результата, DSL snapshot, schema_version, тайминги. | `FrameProcessingReport`, `StageTiming`, `StageProfile`, `PipelineConfigSnapshot`. |

## 3. Материалы, которые стоит прочитать

Для базовой теории и реализации через OpenCV:

1. **OpenCV Mat - The Basic Image Container**. Читать для понимания `cv::Mat`, ROI, копирования, shallow/deep copy, layout. Это основа для Prep, FrameContext и минимизации копий. ([docs.opencv.org](https://docs.opencv.org/3.4/d6/d6d/tutorial_mat_the_basic_image_container.html))

2. **OpenCV Image Filtering / Smoothing / filter2D**. Читать для Radiometric correction, Enhancement и Matched filtering. Особенно важно понять свёртку, ядра, Gaussian, median, bilateral, border policy и стоимость больших ядер. OpenCV указывает, что для больших ядер `filter2D` может использовать DFT-based algorithm, а для малых - direct algorithm. ([docs.opencv.org](https://docs.opencv.org/4.x/d4/d86/group__imgproc__filter.html))

3. **OpenCV Image Thresholding**. Читать для Candidate extraction: global threshold, adaptive threshold, Otsu. Особенно полезно для понимания, почему global threshold подходит не всегда, если фон/освещение неоднородные. ([docs.opencv.org](https://docs.opencv.org/4.x/d7/dd0/tutorial_js_thresholding.html))

4. **OpenCV Template Matching**. Читать для Matched / quasi-matched filtering. Там хорошо объясняется идея: шаблон скользит по изображению, формируется result matrix, потом максимум/минимум выбирается через `minMaxLoc`. ([docs.opencv.org](https://docs.opencv.org/4.x/de/da9/tutorial_template_matching.html))

5. **OpenCV Structural Analysis and Shape Descriptors**. Читать для Segmentation refinement, Object filtering, Measurement: `findContours`, `connectedComponents`, moments, shape descriptors. ([docs.opencv.org](https://docs.opencv.org/4.x/d3/dc0/group__imgproc__shape.html))

6. **OpenCV Background Subtraction Methods**. Читать не как главный источник по внутрикадровой обработке, а как материал для stateful-вариантов Candidate extraction, если в L2 допускаются MOG2/KNN. OpenCV показывает использование MOG2/KNN для генерации foreground mask. ([docs.opencv.org](https://docs.opencv.org/4.x/d1/dc5/tutorial_background_subtraction.html))

## 4. Свежие статьи и обзоры

**Small and dim target detection in infrared imagery: A review, current techniques and future directions, Neurocomputing, 2025.** Это, вероятно, самый полезный свежий обзор под твою задачу, если DP1 связан с малыми/слабыми объектами. Обзор покрывает методы от классической обработки изображений до deep learning, классифицирует методы, обсуждает datasets, проблемы низкого контраста, шумов, clutter и single-frame/multi-frame подходы. Для твоего DP1 особенно важны части про single-frame detection, background suppression, local contrast, SNR/SCR и post-processing. ([sciencedirect.com](https://www.sciencedirect.com/science/article/pii/S0925231225003121))

**Dual channel and multi-scale adaptive morphological methods for infrared small targets, Journal of Big Data, 2024.** Полезно для Radiometric correction, Enhancement, Candidate extraction и Segmentation. В статье прямо разделяются sequence-based и single-frame-based методы; single-frame подходи описываются как методы, которые используют градиент, яркость, контраст, target enhancement и background suppression. Это очень близко к логике твоего DP1 как внутрикадрового модуля. ([link.springer.com](https://link.springer.com/article/10.1186/s40537-024-00880-2))

**A contemporary survey on image reconstruction with unsupervised deep learning: from denoising to generation, Visual Intelligence, 2025.** Это не про твой pipeline напрямую, но полезно для этапа Enhancement / denoising. Обзор объясняет связь классических prior-based методов и современных нейросетевых denoiser-подходов. Для твоего проекта его лучше использовать как теоретический материал, а не как прямую рекомендацию тащить deep learning в DP1. ([link.springer.com](https://link.springer.com/article/10.1007/s44267-025-00092-z))

**Paying more attention to local contrast: improving infrared small target detection performance via prior knowledge, arXiv, 2024.** Интересная статья именно потому, что она соединяет handcrafted local contrast operators с нейросетевым подходом. Для твоей архитектуры это полезно концептуально: даже если DP1 остаётся классическим OpenCV/C++ pipeline, local contrast можно рассматривать как отдельный stage variant или как часть matched/enhancement stage. ([arxiv.org](https://arxiv.org/abs/2411.13260))

**Image Segmentation in Foundation Model Era: A Survey, arXiv, 2024.** Это обзор современной сегментации, но он больше про foundation models, SAM-подобные подходы и deep learning. Для твоего ТЗ полезен как общий обзор понятия segmentation, но не как основной источник для RT-5/RT-20 классического C++ pipeline. ([arxiv.org](https://arxiv.org/abs/2408.12957?utm_source=chatgpt.com))

**Halide: Decoupling Algorithms from Schedules for High-Performance Image Processing.** Это не свежая статья, но очень важная для архитектурного мышления. В ней хорошо сформулирована идея, что для производительного image processing недостаточно оптимизировать отдельные циклы; нужно управлять locality, tiling, blocking и schedule всего pipeline. Это напрямую связано с твоими требованиями к тайлам, профилированию, минимизации копий и RT-профилям. ([researchgate.net](https://www.researchgate.net/publication/322092679_Halide_Decoupling_algorithms_from_schedules_for_high-performance_image_processing))

## 5. В каком порядке изучать

Я бы шёл так: сначала `cv::Mat`, ROI, shallow/deep copy, типы `CV_16U / CV_32F / CV_8U`; потом filtering: Gaussian, median, bilateral, DoG, `filter2D`; потом thresholding и adaptive threshold; потом morphology, contours, connected components; потом region measurements: centroid, bbox, moments, intensity features; и только после этого читать свежие статьи по small/dim target detection, потому что там эти базовые операции уже используются как строительные блоки.

Главный вывод: для твоего ТЗ “структуры данных” - это не только C++ structs/classes. Это формальная модель прохождения данных через pipeline: `RawFrame → ProcessingFrame/Residual → ResponseMap → Mask → Components/Contours → ObjectCandidate → MeasurementRecord → FrameProcessingReport`. Именно вокруг этой цепочки нужно строить маленькие ТЗ этапов и Data Cards.

## Пользователь

Предоставляю текущее состояние базы знаний а также материалы, которые ты предоставил. В базе знаний есть директория data_domains, а также другие в 02-dp1\canonical. Тебе необходимо анализировать канонические структуры. Там есть описания этих структур. Проанализируй их и напиши что с ними не так. 

## Пользователь

Предоставляю текущее состояние базы знаний, разговор с GPT по поводу етапов, а также материалы, которые ты предоставил. В базе знаний есть директория data_domains, а также другие в 02-dp1\canonical. Тебе необходимо анализировать канонические структуры. Там есть описания этих структур. Проанализируй их и напиши что с ними не так. Какие есть ошибки? Насколько они оптимальны по памяти? Подходят ли они для работы с OpenCV? Достаточно ли полное описание для работы AI-кодера? 

## Ассистент

Я просмотрел архив `pk(1).zip`, в первую очередь `02-dp1/canonical/data_domains`, `02-dp1/canonical/data_domains/structures`, `02-dp1/canonical/pipeline` и stage-карточки. Короткий вердикт: направление в целом правильное, но текущие canonical structures пока больше похожи на хорошую архитектурную черновую модель, чем на достаточное ТЗ для AI-кодера. Главная проблема не в том, что структуры “плохие”, а в том, что они слишком часто оставляют ключевые вещи на “future implementation task”, имеют не до конца синхронизированные типы, частично конфликтуют между собой и недостаточно строго описывают OpenCV-инварианты.

По требованиям большого ТЗ код должен выводиться только из формализованных карток, stage specs и конфигурации, а карточка должна содержать definition, fields/interface, input/output, constraints, failure cases и connections. Также DP1 должен быть pipeline = stages + configuration + data structures, с явными форматами и контролем вычислительной стоимости. fileciteturn7file0 Текущие карточки это направление поддерживают, но пока не закрывают уровень точности, достаточный для безопасной генерации C++ кода.

## 1. Главная архитектурная проблема

В базе сейчас есть конфликт между двумя моделями `FrameContext`.

В большом ТЗ `FrameContext` описан как единый контейнер, который содержит raw frame, processing representations, masks, contours/components, objects, measurements, profiling и metadata. fileciteturn6file3 В текущей canonical-базе `FrameContext` наоборот объявлен как small metadata object, который не должен содержать primary image buffers, masks, candidates, segments или measurements. Это не обязательно ошибка проектирования - с точки зрения памяти текущий canonical-вариант даже лучше. Но это конфликт источников истины.

Решение: нужно явно зафиксировать одну из двух моделей. Я бы оставил текущую canonical-модель: `FrameContext` как служебный metadata/context, а все данные этапов передавать через explicit structures: `FramePacket`, `TileRawView`, `TileProcessingFrame`, `TileBinaryMask`, `Candidate`, `Segment`, `ValidatedObject`, `MeasurementRecord`. Но тогда большое ТЗ или отдельная canonical clarification card должны сказать: `FrameContext` “contains or references runtime metadata”, а не “owns all data”.

## 2. Ошибки и несогласованности в типах

Самая заметная ошибка - несогласованность `ProcessingFrame` / `TileProcessingFrame` с `PixelFormat`.

В `dp1.domain.pixel_format.md` определены `U8`, `U16`, `F32`, `MaskU8`, `S16`, `S32`. Это логично, потому что residual после вычитания фона может быть signed. Но в `ProcessingFrame` написано, что `pixel_format` может быть `U8`, `U16` или `F32`, тогда как stage `radiometric_correction` допускает output `S16`, `S32`, `F32`, `U8` или `U16`, а `ProcessingRouteConfig` вообще по умолчанию ставит `radiometric_output_format = S32`. Это прямое противоречие.

Нужно привести к одному варианту. Для radiometric residual canonical processing carrier должен разрешать `S16`, `S32` и `F32`. Если основной тракт должен быть `F32`, тогда `S16/S32` надо оставить как intermediate/raw residual route, но не как `ProcessingFrame.pixel_format`. Сейчас база говорит одновременно оба варианта.

Вторая проблема - `RangePolicy`. В `ProcessingFrame` default стоит `RangePolicy::NormalizedFloat`, а в `ProcessingRouteConfig` default стоит `RangePolicy::SignedResidual`. Для residual это критично: normalized float и signed residual имеют разную семантику порогов, фотометрии и threshold units. Нужно сделать default зависящим от `processing_domain`: для `RadiometricResidual` - `SignedResidual`, для `DetectorResponse` - отдельный `DetectorResponseRange` или явно заданный `NormalizedFloat`.

Третья проблема - часть типов используется, но не определена в canonical data structures. Например, в карточках встречаются `StageStatus`, `ProfileEvent`, `DiagnosticMessage`, `BorderPolicy`, `CandidateStatus`, `TileProfilingSummary`, `ParameterMap`. `PipelineConfig` есть, а вот эти типы либо отсутствуют, либо не имеют отдельной canonical card. Для человека это понятно, но AI-кодер при генерации C++ получит незамкнутую модель.

## 3. Проблемы с OpenCV-совместимостью

Использование `cv::Mat` в целом подходит для OpenCV. Это правильный carrier для raw, processing и mask domains. Также правильна идея ROI-view через `cv::Mat tile = frame(rect)`, потому что для tile route не нужно копировать raw frame. Это согласуется и с материалом по структурам на входе/выходе этапов, где tile должен быть view, а не deep copy. fileciteturn6file13

Но текущие карточки недостаточно строго описывают OpenCV-инварианты. Для каждой структуры с `cv::Mat` надо явно указать:

`image.empty() == false`, `image.channels() == 1`, `image.type()` соответствует `pixel_format`, `image.cols/rows` соответствуют `geometry` или `valid_area`, `image.isSubmatrix()` допустим для ROI-view, `image.isContinuous()` не гарантируется, поэтому stage-код не имеет права слепо идти по памяти как по continuous buffer без проверки. Это особенно важно для `TileRawView`, потому что ROI в OpenCV часто имеет stride исходного изображения, а не плотную tile-width строку.

В `TileRawView`, `TileProcessingFrame`, `TileBinaryMask` есть `valid_area` и `origin_px`, но не всегда строго сказано, в каких координатах хранится `valid_area`: tile-local, frame-global или local относительно `roi_with_border`. Это опасно. Для OpenCV-кода это приведёт к ошибкам crop/merge на границах тайлов.

Также нужно явно описать border policy. В `TileDesc` есть `roi`, `roi_with_border`, `valid_area`, `origin_px`, `BorderPolicy`, но `BorderPolicy` не определён. Для фильтров OpenCV border behavior критичен: `GaussianBlur`, `filter2D`, morphology и median могут давать разные результаты у границ. В исследованиях по high-performance image processing отдельно подчёркивается, что tiling, boundary/guard bands и организация промежуточного хранения напрямую влияют на производительность и корректность pipeline. fileciteturn7file2

## 4. Оценка по памяти

В целом память спроектирована правильно: raw frame хранится один раз, tile route использует `TileDesc[]`, per-worker `TileContext`, reuse буферов и `TileResult` без image payload. Это хорошая модель.

Для кадра `1440x1080` оценка примерно такая: raw `CV_16UC1` - около 3.1 MB, один full-frame `CV_32FC1` - около 6.2 MB, full-frame `MaskU8` - около 1.6 MB. Если держать несколько full-frame processing buffers для residual, enhancement и response, память быстро растёт. Поэтому tile route с двумя `F32` buffers и одним `MaskU8` per worker действительно лучше.

Для tile `256x256`: один `F32` buffer - около 256 KB, два `F32` buffers - около 512 KB, один `MaskU8` - около 64 KB. На 8 workers это примерно 4.5 MB только на image buffers, без vector capacities. Это нормально.

Но есть несколько слабых мест.

Первое: `Segment` содержит `std::vector<cv::Point> contour_px`. Это потенциально самый дорогой объект. На шумных масках число контуров и точек может резко вырасти. Для RT-5/RT-20 это риск. В production route лучше не хранить полный contour по умолчанию. Нужен режим: `bbox + area + centroid + moments` как compact representation, а полный contour только в debug/L2/L3 или если stage spec требует shape analysis.

Второе: `TileResult` хранит `candidate_results`, `segment_results`, `validated_object_results`, `measurement_results`. Если все эти векторы копируются из `TileContext`, будет лишняя память и лишние copy/move. Надо явно прописать: production route переносит только минимально нужный набор, обычно `measurement_results`, а промежуточные vectors сохраняются только при `debug_trace=true` или `profiling/audit` режиме.

Третье: `std::string source_id`, `source_ref`, `pipeline_run_id` в runtime-структурах создают heap allocations. Для metadata это допустимо, но для hot path лучше использовать numeric IDs или interned string references. Особенно `BinaryMask.source_ref` как строка - плохой вариант для AI-кодера: непонятно, что именно туда писать и как валидировать.

Четвёртое: `cv::Mat` header в карточках оценивается примерно как 96 B. Это нормально как приблизительная оценка, но нельзя делать acceptance criteria на конкретный размер `cv::Mat`, потому что он зависит от сборки OpenCV и платформы. Лучше писать “header-only overhead, implementation-dependent”.

## 5. Что не хватает в структурах для DP1-алгоритмов

Для radiometric correction не хватает отдельной структуры для background model / background estimate. В ТЗ radiometric stage должен разделять модель фона и residual. fileciteturn6file2 Сейчас есть только `ProcessingFrame`, куда можно положить residual или corrected image. Но для inverse median, adaptive background, MOG2/KNN, per-tile background логически нужны отдельные carriers: `BackgroundFrame`, `BackgroundModelRef`, `ResidualFrame` или хотя бы `ProcessingDomain::BackgroundEstimate`.

Для candidate extraction L2 указаны MOG2/KNN, но нет структуры для stateful-модели. OpenCV `BackgroundSubtractorMOG2/KNN` - это состояние, и оно должно жить не в `FrameContext`, а в отдельном stage state, явно связанном с config/profile/source. Сейчас это не формализовано.

Для local contrast / small target detection не хватает признаков качества. В статьях по infrared small target detection постоянно фигурируют background suppression, target enhancement, local contrast, SCR/SNR/BSF и влияние multi-scale morphology на качество. fileciteturn5file0 В твоих структурах есть `score`, `quality_flags`, `PhotometryStats`, но нет явных полей вроде `local_contrast`, `background_mean`, `background_std`, `snr`, `scr`, `peak_intensity`, `integrated_intensity`. Не все они обязаны быть в `MeasurementRecord`, но для research/validation route их надо где-то хранить: либо в `ObjectQualityStats`, либо в расширении `PhotometryStats`.

Для measurement не хватает schema/version metadata. Большое ТЗ требует reproducibility и пригодность для DP2 handoff. fileciteturn6file8 В `MeasurementRecord` есть `measurement_id`, `frame_id`, `camera_id`, `time_ref`, координаты и photometry, но нет `schema_version`, `source_id`, `config/profile reference`, `stage variant provenance`. Возможно это должно быть на уровне batch/report, но тогда нужна структура `MeasurementBatch` или `FrameProcessingReport`.

## 6. Проблемы с координатами

Координатная модель хорошая по идее, но пока недостаточно точная для кода.

Нужно явно определить, что означает `cv::Rect bbox_px`: OpenCV convention - `x, y, width, height`, где right/bottom граница не включается. Это должно быть записано, иначе AI-кодер может использовать inclusive rectangle.

Нужно определить convention для `centroid_px`: координаты центра пикселя считаются как `(x + 0.5, y + 0.5)` или как integer grid point? Для субпиксельных координат это важно.

Нужно определить, когда exactly выполняется tile-local → frame-global conversion. Сейчас сказано, что merge переводит координаты, но не описан алгоритм: `global = local + origin_px`, но local относительно чего - `roi`, `roi_with_border` или `valid_area`? Это должно быть формализовано.

Нужно закрыть open question по duplicate suppression на tile borders. Пока он открыт, tile route нельзя считать готовым для production. На границах overlap один объект может быть найден в двух tile, и без строгой merge policy DP2 получит дубликаты.

## 7. Достаточно ли для AI-кодера

Для AI-кодера текущая база недостаточна, если задача звучит “сгенерировать рабочий C++ код”. Она достаточна для “сгенерировать черновой каркас классов” или “проверить соответствие архитектуре”.

Почему недостаточна:

1. Не все типы определены.
2. Много “recommended C++ form”, но мало обязательных compile-ready declarations.
3. Нет namespace/include/dependency policy.
4. Нет точных function signatures для stage interfaces.
5. Нет строгих OpenCV preconditions/postconditions.
6. Нет ownership enum и lifetime policy для `cv::Mat` с внешней памятью.
7. Нет exact validation rules: что делать при wrong type, wrong geometry, empty Mat, non-continuous Mat, out-of-bounds ROI.
8. Нет точного serialization schema для `MeasurementRecord`, `FrameProcessingReport`, `PipelineConfig`.
9. Нет полного registry для flags/status/errors.
10. Есть open questions, которые блокируют production-код: ID namespace, duplicate suppression, profiling trace, exact payload schema.

Минимум, который надо добавить для AI-кодера: один `dp1.canonical.types.hpp.md` или набор карточек, где все enum/struct compile-ready; отдельную `dp1.domain.opencv_invariants.md`; отдельную `dp1.domain.ids.md`; отдельную `dp1.domain.errors_and_status.md`; отдельную `dp1.domain.profiling.md`; и для каждого stage - точную сигнатуру `process(...)` по route `full_frame` и `tiles`.

## 8. Оценка по группам структур

`FramePacket` в целом правильный. Проблемы: `source_id` строкой не оптимален для hot path, ownership `cv::Mat image` не определён, нет явного указания на external memory/ring buffer case, нет строгой проверки `image.type() == CV_16UC1/CV_8UC1`.

`TileDesc` правильный концептуально. Проблемы: не определён `BorderPolicy`, неоднозначен `valid_area`, не прописана формула local/global coordinates, не закрыта duplicate suppression.

`TileRawView` подходит OpenCV, если это ROI-view. Проблемы: надо явно запретить запись, но `cv::Mat` сам по себе mutable. Лучше использовать `cv::Mat` + contract read-only, либо wrapper `ConstMatView`, либо хотя бы `const cv::Mat* source + cv::Rect roi`.

`ProcessingFrame` и `TileProcessingFrame` полезны, но сейчас конфликтуют с signed residual. Нужно разрешить `S16/S32` или убрать их из route. Также нужны stronger invariants по `processing_domain`, `range_policy`, `value_range`.

`BinaryMask` и `TileBinaryMask` в целом нормальные. Нужно добавить правило: mask values are binary by convention, but OpenCV functions treat any non-zero as foreground. Нужно указать, должен ли pipeline нормализовать все foreground к 255.

`Candidate` нормальный компактный объект. Не хватает определения `CandidateStatus`. Нужно решить, `score` в каких единицах: detector response, probability-like score, normalized response, threshold margin.

`Segment` самый проблемный по памяти из-за `std::vector<cv::Point> contour_px`. Для RT route нужно сделать contour optional/debug или заменить на compact shape features.

`ValidatedObject` в целом хорош как промежуточный результат object filtering. Проблема: много дублирует `Segment`; это допустимо, но надо явно сказать, что это compact snapshot, а не полная копия segment. Также rejected objects надо хранить только по policy.

`MeasurementRecord` хороший старт, но недостаточен для финального DP1 → DP2 контракта. Нужны schema/version/batch, source_id, acquisition timestamp, coordinate convention, photometry domain, возможно local contrast/SNR/SCR для quality route.

`TileContext` правильный как per-worker owner. Проблема: ровно два processing buffers может быть мало для отдельных L2/L3 stage variants. Нужно описать scratch allocator или per-stage buffer request model.

`TileResult` правильный как merge boundary, но нужно прописать move-only/production-debug policy, иначе он может стать источником лишних копий.

## Итог

Текущие canonical structures хорошие как архитектурная основа. Они уже правильно разделяют Raw / Processing / Mask / Struct / Measurement domains и в целом соответствуют идее `16U → 32F → 32F → 32F → 8U(mask) → structures → MeasurementRecord`, которая зафиксирована в материалах по входам/выходам этапов. fileciteturn7file1

Но до уровня “можно давать AI-кодеру для реализации” их нужно ужесточить. Сейчас главный риск - агент напишет код, который формально похож на архитектуру, но по факту будет иметь скрытые `convertTo`, неверные координаты на tile borders, невалидные `cv::Mat` assumptions, лишние копии vectors/contours и неопределённые статусы/типы. Для следующего шага я бы не переписывал всё сразу, а сначала закрыл 5 блокирующих карточек: OpenCV invariants, missing enums/types, coordinate/merge policy, ownership/lifetime policy, MeasurementRecord/FrameProcessingReport schema.

## Пользователь

Тебе понятна идея пайплайна? Есть 8 етапов, у каждого етапа по 3 варианта, варианты выбираются через конфигурацию. Например для Prep етапа есть варианты `full_frame`, `roi`, `tiles`, `adaptive_roi` и тд

## Ассистент

Да, идея понятна.

Модель такая: DP1 - это не один жёстко зашитый алгоритм, а конфигурируемый pipeline из 8 основных этапов. Каждый этап является интерфейсом, а конкретное поведение выбирается через конфигурацию: какой вариант реализации использовать, какой уровень сложности, какие параметры и какие форматы данных допустимы. Это соответствует общей логике ТЗ: pipeline задаётся как этапы + конфигурация + структуры данных. fileciteturn5file6

Но здесь важно не смешать два разных понятия: **variant** и **level**.

**Variant** - это конкретный способ реализации этапа. Например, для `Prep` это могут быть `full_frame`, `roi`, `tiles`, `adaptive_roi`, `multi_scale_tiles`.

**Level** - это уровень сложности/стоимости: условно `L0`, `L1`, `L2`, иногда `L3`. Он описывает компромисс “скорость - качество - сложность”.

То есть правильнее мыслить не так: “у каждого этапа ровно 3 варианта”, а так: “у каждого этапа есть несколько вариантов реализации, и каждый вариант может относиться к одному или нескольким уровням сложности”.

Например для `Prep`:

```json id="wpkh5t"
"prep": {
  "enabled": true,
  "variant": "tiles",
  "level": "L1",
  "parameters": {
    "tile_width": 256,
    "tile_height": 256,
    "overlap": 16
  }
}
```

Тогда `Prep` как интерфейс остаётся один, но реализация выбирается через DSL. В одном запуске это может быть `full_frame`, в другом - `tiles`, в третьем - `adaptive_roi`.

Логика по всем 8 этапам примерно такая:

| Этап | Роль | Примеры вариантов |
|---|---|---|
| `Prep` | подготовка кадра, ROI, тайлы, координаты | `full_frame`, `roi`, `tiles`, `adaptive_roi` |
| `Radiometric` | фон, residual, вычитание фона | `gaussian_subtraction`, `median`, `inverse_median`, `adaptive_background` |
| `Enhancement` | повышение SNR, подавление шума | `gaussian`, `dog`, `bilateral`, `local_contrast` |
| `MatchedFilter` | детектор / карта отклика | `gaussian_quasi`, `filter2d_kernel`, `template`, `psf_fit` |
| `CandidateExtraction` | маска / первичные гипотезы | `global_threshold`, `adaptive_threshold`, `otsu`, `mog2`, `knn` |
| `Segmentation` | компоненты, контуры, морфология | `connected_components`, `contours`, `open_close_contours` |
| `ObjectFiltering` | отсев ложных объектов | `area_only`, `geometry_basic`, `shape_photometry` |
| `Measurement` | финальные измерения для DP2 | `centroid_bbox`, `moments`, `photometry_basic`, `subpixel` |

Главный архитектурный принцип: каждый этап должен иметь стабильный контракт входа/выхода. Например, если `Radiometric` выбран как `inverse_median`, а `Enhancement` как `dog`, то они всё равно должны стыковаться через заранее описанный формат, например `CV_32F ProcessingFrame`. Нельзя, чтобы один вариант внезапно вернул `CV_8U`, а следующий этап молча сделал `convertTo`.

То есть итоговая формула такая:

```text id="f9503k"
Stage interface + selected variant + level + parameters + input/output data contract = concrete stage implementation
```

И весь pipeline задаётся как набор таких выбранных реализаций:

```text id="tunwo6"
DP1Config =
  Prep.variant + Radiometric.variant + Enhancement.variant
  + MatchedFilter.variant + CandidateExtraction.variant
  + Segmentation.variant + ObjectFiltering.variant + Measurement.variant
```

Поэтому да, идея понятна. Но я бы формулировал её точнее так: **есть 8 канонических этапов; каждый этап имеет набор допустимых variants; конфигурация выбирает variant, level и parameters; структуры данных обеспечивают совместимость между этапами**.

## Пользователь

Предложи исправления базы знаний понятные для AI-агента

## Ассистент

Да. Я бы оформил исправления не как “улучшить документацию”, а как набор маленьких patch-задач для AI-агента. Главная цель - сделать так, чтобы агент не додумывал архитектуру, а работал по закрытым правилам: какие есть 8 этапов, какие у них variants, какие структуры входят/выходят, какие `cv::Mat` типы допустимы, кто владеет памятью, где выполняется conversion, где merge тайлов.

В большом ТЗ уже зафиксировано, что код нельзя генерировать из неформализованного текста, а только из карточек структур данных, карточек интерфейсов, малых ТЗ этапов и конфигурации. Там же сказано, что каждый этап должен иметь карточку, множество реализаций и быть независимой единицей конфигурации. fileciteturn8file7 Поэтому база знаний должна быть не “объяснением для человека”, а набором строгих contracts для агента.

## 1. Исправление №1 - разделить Stage, Variant и Level

Сейчас в базе смешиваются понятия “этап”, “вариант” и “уровень сложности”. Нужно добавить отдельную карточку:

`02-dp1/canonical/configuration/dp1.config.stage_variant_registry.md`

Содержание должно быть примерно таким:

```text id="8ns8ex"
Stage = fixed pipeline interface.
Variant = concrete implementation family selected by config.
Level = complexity/performance class for a selected variant.
Parameters = variant-specific parameters.

The AI coder MUST NOT treat L0/L1/L2 as variants.
The AI coder MUST NOT create a new variant unless it is registered here.
The AI coder MUST NOT hard-code variant selection outside PipelineConfig.
```

Далее в этой карточке нужно дать таблицу:

| Stage | Allowed variants | Typical levels |
|---|---|---|
| `prep` | `full_frame`, `roi`, `tiles`, `adaptive_roi` | L0/L1/L2 |
| `radiometric_correction` | `gaussian_subtraction`, `median`, `inverse_median`, `adaptive_background` | L0/L1/L2 |
| `enhancement` | `none`, `gaussian`, `dog`, `bilateral`, `local_contrast` | L0/L1/L2 |
| `matched_filtering` | `gaussian_quasi`, `filter2d_kernel`, `template`, `psf_fit` | L0/L1/L2 |
| `candidate_extraction` | `global_threshold`, `adaptive_threshold`, `otsu`, `mog2`, `knn` | L0/L1/L2 |
| `segmentation_refinement` | `connected_components`, `contours`, `morphology_contours` | L0/L1/L2 |
| `object_filtering` | `area_only`, `geometry_basic`, `shape_photometry` | L0/L1/L2 |
| `measurement` | `centroid_bbox`, `moments`, `photometry_basic`, `subpixel` | L0/L1/L2 |

Тут важно: `tiles` - это variant этапа `Prep`, а не отдельный pipeline. При `prep.variant = tiles` меняется execution route: каждый tile проходит этапы 2-8, затем выполняется merge.

## 2. Исправление №2 - добавить единую Stage I/O Matrix

Нужно создать карточку:

`02-dp1/canonical/pipeline/dp1.pipeline.stage_io_matrix.md`

Эта карточка должна стать главным документом для агента при стыковке этапов. В ней нужно прямо зафиксировать канонический поток:

| Stage | Input | Internal | Output |
|---|---|---|---|
| `input` | - | - | `FramePacket` with `CV_16UC1` or explicit `CV_8UC1` test route |
| `prep` | `FramePacket` | Raw domain | `FramePacketView` / `TileRawView[]` / `RoiRawView[]` |
| `radiometric_correction` | Raw view | `CV_32FC1` | `ProcessingFrame(RadiometricResidual, F32)` |
| `enhancement` | `ProcessingFrame(F32)` | `CV_32FC1` | `ProcessingFrame(EnhancedFrame, F32)` |
| `matched_filtering` | `ProcessingFrame(F32)` | `CV_32FC1` | `ProcessingFrame(DetectorResponse, F32)` |
| `candidate_extraction` | `ProcessingFrame(F32)` | `CV_32FC1` + threshold logic | `BinaryMask(MaskU8)` + optional `Candidate[]` |
| `segmentation_refinement` | `BinaryMask(MaskU8)` | `CV_8UC1` | `Segment[]` |
| `object_filtering` | `Candidate[]` / `Segment[]` | structures | `ValidatedObject[]` |
| `measurement` | `ValidatedObject[]` + raw/proc photometry source | structures | `MeasurementRecord[]` |

Это прямо согласуется с уже предоставленной таблицей: raw/photometry - `CV_16U`, processing/detection - `CV_32F`, mask - `CV_8U`, после segmentation идут structures, а Measurement выдаёт `MeasurementRecord`. fileciteturn8file0

В этой же карточке надо написать запрет:

```text id="x6dkdj"
The AI coder MUST NOT pass VisualizationDomain back into computation.
The AI coder MUST NOT use CV_8U as Processing domain unless the selected variant explicitly declares a fast_u8 route.
The AI coder MUST NOT perform implicit convertTo between stages.
```

## 3. Исправление №3 - исправить `ProcessingFrame`

Сейчас `ProcessingFrame` должен быть ужесточён. Для канонического варианта нужно сделать так:

```cpp id="zpgzwl"
enum class ProcessingDomain : std::uint8_t {
    RadiometricResidual,
    RadiometricCorrected,
    EnhancedFrame,
    DetectorResponse,
    LocalContrastResponse,
    BackgroundEstimate
};

enum class RangePolicy : std::uint8_t {
    RawPreserved,
    SignedResidual,
    NormalizedFloat,
    DetectorResponse,
    ClippedToInputRange,
    ShiftedToPositive
};

struct ProcessingFrame {
    std::uint64_t frame_id = 0;
    std::optional<std::uint64_t> source_frame_id;
    cv::Mat image;                  // default canonical: CV_32FC1
    PixelFormat pixel_format = PixelFormat::F32;
    PixelRange value_range;
    ProcessingDomain processing_domain;
    RangePolicy range_policy;
    FrameGeometry geometry;
};
```

Главное исправление: `ProcessingFrame` не должен по умолчанию разрешать всё подряд `U8 / U16 / F32`. Канонический processing route должен быть `F32`. `U8` можно разрешать только для явно объявленного fast route, а `S16/S32` - только для отдельного residual route, где stage spec явно описывает signed range. В материалах по входам/выходам этапов это уже сформулировано как “16U - raw, 32F - весь детекційний тракт, 8U - тільки маска”. fileciteturn8file2

## 4. Исправление №4 - добавить OpenCV invariants

Нужно создать карточку:

`02-dp1/canonical/data_domains/dp1.domain.opencv_invariants.md`

Это одна из самых важных карточек для AI-кодера. В неё нужно вставить такие правила:

```text id="3jhqv0"
For every cv::Mat carrier:
- mat.empty() MUST be false before processing.
- mat.channels() MUST match the domain:
  Raw/Processing/Mask = 1 channel, Visualization = 3 channels.
- mat.type() MUST match PixelFormat.
- mat.cols and mat.rows MUST match declared geometry.
- mat.isContinuous() MUST NOT be assumed.
- ROI/tile cv::Mat may be a submatrix.
- Stage code MUST use row-wise access or check isContinuous().
- cv::Rect uses OpenCV half-open convention: x <= px < x + width, y <= py < y + height.
- Visualization cv::Mat MUST NOT be used as computation input.
```

Также нужно явно прописать соответствие:

| PixelFormat | OpenCV type |
|---|---|
| `U8` | `CV_8UC1` |
| `U16` | `CV_16UC1` |
| `S16` | `CV_16SC1` |
| `S32` | `CV_32SC1` |
| `F32` | `CV_32FC1` |
| `MaskU8` | `CV_8UC1` |
| `BgrU8` | `CV_8UC3` |

Без такой карточки агент может написать код, который компилируется, но ломается на ROI, потому что `cv::Mat` после `frame(rect)` часто не является continuous.

## 5. Исправление №5 - зафиксировать ownership и lifetime

Нужно обновить:

`02-dp1/canonical/data_domains/dp1.domain.memory_ownership.md`

И добавить туда простую модель:

```text id="rifgz5"
FramePacket.image:
- owner or external reference to full raw frame.
- lifetime MUST cover all stage processing for this frame.

TileRawView.image:
- non-owning ROI view into FramePacket.image.
- MUST NOT outlive FramePacket.image.
- MUST be read-only by contract.

TileProcessingFrame.image:
- owned/reused by TileContext.
- may be overwritten by next stage only if previous value is no longer needed.

TileBinaryMask.mask:
- owned/reused by TileContext.
- foreground convention must be explicit: 0 background, 255 foreground.

TileResult:
- owns only structural results, not image buffers.
```

Для агента это критично, потому что без ownership policy он может начать клонировать каждый tile или хранить `TileRawView` дольше, чем живёт исходный `FramePacket`.

## 6. Исправление №6 - уточнить tile route и merge

Нужно создать или расширить карточку:

`02-dp1/canonical/pipeline/dp1.pipeline.tile_execution_route.md`

В ней надо описать именно это:

```text id="6rwkmp"
When prep.variant = tiles:
1. Prep builds TileDesc[].
2. Each TileDesc has roi, roi_with_border, valid_area, origin_px.
3. TileRawView.image = FramePacket.image(tile_desc.roi_with_border).
4. Stages 2-8 run inside each tile.
5. Candidate/Segment/Object/Measurement results are cropped by valid_area.
6. Tile-local coordinates are converted to frame-global coordinates.
7. TileResult[] are merged.
8. Border duplicates are suppressed.
9. Final output is frame-level MeasurementRecord[].
```

Нужно обязательно зафиксировать:

```text id="7henv6"
local_point_in_tile_with_border -> global_point:
global.x = tile_desc.roi_with_border.x + local.x
global.y = tile_desc.roi_with_border.y + local.y

valid_area is expressed in tile-local coordinates relative to roi_with_border.
```

В материалах уже есть правильная идея: tile должен обрабатываться как `tile_with_border → processing → crop_valid_area → objects`, потому что фильтры и морфология используют соседние пиксели; после всех тайлов нужен merge, удаление дубликатов на границах и перевод координат в глобальные координаты кадра. fileciteturn8file6

## 7. Исправление №7 - сделать `FrameContext` не контейнером данных, а runtime context

Сейчас возможен конфликт: в большом ТЗ `FrameContext` звучит как контейнер всего, а в canonical-модели лучше сделать его lightweight runtime context. Я бы зафиксировал так:

```text id="7b01xg"
FrameContext is NOT the owner of primary image payloads.
FrameContext is NOT a hidden output container.
FrameContext stores:
- frame identity;
- selected PipelineConfig snapshot;
- timing/profiling;
- diagnostics;
- stage counters;
- references to current route metadata.

FrameContext MUST NOT store:
- masks;
- candidates;
- segments;
- validated objects;
- measurements;
- hidden cv::Mat outputs.
```

Это нужно внести в:

`02-dp1/canonical/data_domains/structures/runtime/dp1.domain.runtime.frame_context.md`

И добавить явный запрет в `dp1.pipeline.stage_contract.md`:

```text id="ezai9x"
A stage MUST return output explicitly.
A stage MUST NOT write its primary result only into FrameContext.
```

Так агент будет писать чистые stage interfaces, а не большой глобальный mutable object.

## 8. Исправление №8 - добавить missing canonical types

Нужно создать файл:

`02-dp1/canonical/data_domains/dp1.domain.common_types.md`

В нём определить все типы, которые сейчас используются, но не везде формализованы:

```cpp id="077a8f"
enum class StageStatus : std::uint8_t {
    Ok,
    Skipped,
    EmptyInput,
    InvalidInputFormat,
    InvalidGeometry,
    ConfigError,
    RuntimeError
};

enum class BorderPolicy : std::uint8_t {
    None,
    Replicate,
    Reflect101,
    ConstantZero
};

enum class CandidateStatus : std::uint8_t {
    Provisional,
    RejectedByArea,
    RejectedByThreshold,
    RejectedByGeometry,
    AcceptedForSegmentation
};

enum class MaskForegroundConvention : std::uint8_t {
    ZeroBackground255Foreground,
    ZeroBackgroundOneForeground
};

struct FrameGeometry {
    int width = 0;
    int height = 0;
    CoordinateOrigin origin = CoordinateOrigin::TopLeft;
};

struct StageTiming {
    std::string stage_name;
    std::string variant;
    std::string level;
    double elapsed_ms = 0.0;
};

struct StageProfile {
    std::string stage_name;
    std::string variant;
    std::string level;
    std::size_t input_bytes = 0;
    std::size_t output_bytes = 0;
    std::size_t conversion_bytes = 0;
};
```

Большое ТЗ требует стандартизировать `FrameContext`, `MeasurementRecord`, `StageProfile`, `StageTiming`, фиксировать время каждого этапа, суммарное время кадра и время преобразований форматов. fileciteturn8file3 Поэтому эти типы должны быть не “когда-нибудь”, а частью canonical contract.

## 9. Исправление №9 - сделать MeasurementRecord полноценным DP1 → DP2 contract

Нужно обновить:

`02-dp1/canonical/data_domains/structures/measurement/dp1.domain.measurement.record.md`

Добавить поля уровня контракта:

```cpp id="3jj7oh"
struct MeasurementRecord {
    std::uint32_t schema_version = 1;

    std::uint64_t measurement_id = 0;
    std::uint64_t frame_id = 0;
    int camera_id = -1;

    std::uint64_t source_object_id = 0;
    std::uint64_t source_segment_id = 0;

    TimeRef time_ref;

    cv::Point2f position_px;
    cv::Rect bbox_px;

    float area_px = 0.0F;
    float width_px = 0.0F;
    float height_px = 0.0F;

    PhotometryStats photometry;
    MeasurementQualityFlags quality_flags;

    ProcessingDomain photometry_domain;
    PixelFormat photometry_source_format;
};
```

И добавить отдельную batch-структуру:

```cpp id="nm6wo3"
struct FrameMeasurementBatch {
    std::uint32_t schema_version = 1;
    std::uint64_t frame_id = 0;
    int camera_id = -1;
    TimeRef time_ref;
    std::string pipeline_config_hash;
    std::vector<MeasurementRecord> records;
    StageProfile frame_profile;
};
```

Причина: отдельный `MeasurementRecord` сам по себе не всегда несёт всю информацию о конфигурации, версии схемы и воспроизводимости. А DP2 будет работать не с одним объектом, а с набором измерений кадра.

## 10. Исправление №10 - ограничить тяжёлые структуры в hot path

Нужно обновить `Segment`:

`02-dp1/canonical/data_domains/structures/struct/dp1.domain.struct.segment.md`

Сейчас полный `std::vector<cv::Point> contour_px` может быть очень дорогим. Нужно явно разделить production и debug payload:

```cpp id="79nduv"
struct Segment {
    std::uint64_t segment_id = 0;
    std::uint64_t frame_id = 0;
    int tile_id = -1;

    std::uint64_t candidate_id = 0;

    cv::Rect bbox_px;
    cv::Point2f centroid_px;
    float area_px = 0.0F;

    std::optional<std::vector<cv::Point>> contour_px; // debug/L2/L3 only
    ShapeStats shape;
};
```

И правило:

```text id="j9iobm"
Production RT route MUST NOT require full contour storage unless selected variant explicitly needs it.
L0/L1 should prefer compact geometry: bbox, area, centroid.
Full contour is allowed for L2/L3, debug trace, or shape-heavy filtering.
```

Это важно для памяти и скорости. Иначе при шумной маске агент создаст много векторов с контурами, и RT-профиль развалится.

## 11. Исправление №11 - добавить validation rules для AI-кодера

Нужно создать:

`02-dp1/canonical/validation/dp1.validation.stage_contract_checks.md`

Минимальный набор checks:

```text id="kt35y5"
For every stage implementation:
- Check enabled/variant/level exist in PipelineConfig.
- Check input domain matches Stage I/O Matrix.
- Check cv::Mat type matches PixelFormat.
- Check geometry is non-empty.
- Check no implicit conversion happened.
- Check output domain matches stage contract.
- Record StageTiming.
- Record conversion timing separately.
- Do not read from Visualization domain.
- Do not write hidden output to FrameContext.
```

Для `tiles`:

```text id="ycopd0"
- Check roi_with_border is inside frame bounds.
- Check valid_area is inside tile image bounds.
- Check tile_id is unique inside frame.
- Check tile result coordinates are global before merge.
- Check duplicate suppression policy is applied.
```

## Готовый промпт для AI-агента

Можно дать агенту вот такой текст:

```text id="c5h785"
You are working in project-knowledge only. Do not modify source code. Do not modify legacy cards except adding references if explicitly required.

Goal:
Make DP1 canonical knowledge base suitable for AI-code generation of a configurable 8-stage pipeline.

Context:
DP1 has 8 fixed canonical stages:
1. prep
2. radiometric_correction
3. enhancement
4. matched_filtering
5. candidate_extraction
6. segmentation_refinement
7. object_filtering
8. measurement

Each stage is an interface. A concrete pipeline run is selected by PipelineConfig:
stage.enabled + stage.variant + stage.level + stage.parameters.

Do not confuse variant and level:
- variant = implementation family, e.g. prep.tiles, prep.roi, radiometric.inverse_median.
- level = complexity/performance class, e.g. L0/L1/L2.

Required changes:
1. Add a stage variant registry card under 02-dp1/canonical/configuration.
2. Add a canonical stage I/O matrix under 02-dp1/canonical/pipeline.
3. Fix ProcessingFrame, TileProcessingFrame, PixelFormat and RangePolicy consistency:
   - default canonical processing route is CV_32FC1 / PixelFormat::F32.
   - CV_8U is mask or explicit fast route only.
   - Raw photometry uses CV_16UC1 or explicit source route.
4. Add OpenCV invariants card:
   - no empty Mat;
   - channels/type must match domain;
   - ROI Mat may be non-continuous;
   - isContinuous must not be assumed;
   - cv::Rect is half-open;
   - visualization must not feed computation.
5. Fix FrameContext:
   - FrameContext is runtime metadata/profiling/diagnostics context.
   - It must not be hidden owner of masks, candidates, segments, objects or measurements.
6. Add tile execution route:
   - TileDesc[] created by prep.tiles.
   - TileRawView is a non-owning ROI view.
   - stages 2-8 run per tile.
   - results are cropped by valid_area.
   - coordinates are converted to global frame coordinates.
   - TileResult[] are merged and border duplicates are suppressed.
7. Add missing common canonical enums/types:
   StageStatus, BorderPolicy, CandidateStatus, MaskForegroundConvention, FrameGeometry, StageTiming, StageProfile.
8. Strengthen MeasurementRecord and add FrameMeasurementBatch with schema_version and config hash.
9. Add validation checklist for stage contract conformance.

Acceptance criteria:
- Every new/updated card has Definition, Fields/Interface, Input/Output, Constraints, Failure cases, Connections.
- No card says that implementation details are left to future task if they are needed for code generation.
- All stage outputs are explicit structures, not hidden FrameContext fields.
- All cv::Mat carriers declare PixelFormat, geometry, ownership/lifetime and OpenCV type.
- The canonical flow is explicitly documented as:
  Raw16 -> Processing F32 -> Processing F32 -> DetectorResponse F32 -> MaskU8 -> Struct -> MeasurementRecord.
- The knowledge base clearly allows config-driven variant selection without hard-coded algorithm choice.

After work, report:
1. Files created.
2. Files modified.
3. New canonical rules added.
4. Open questions that remain blocking for C++ implementation.
```

Главная мысль для базы знаний: агенту нельзя давать “понимай сам по контексту”. Нужно дать замкнутую модель: `StageRegistry + StageIOMatrix + DataStructures + OpenCVInvariants + Ownership + TileMerge + Validation`. После этого AI-кодер уже сможет писать каркас C++ без постоянного додумывания архитектуры.

## Пользователь

Весь этот чат как диалог перенеси в один .md файл. Содержание не меняй
