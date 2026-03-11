# DP1 LLM-first cards (draft)

Це стартовий набір “атомарних карток” для DP1 (datapro1), оформлених під LLM-first конспект:
Definition, Assumptions, Theorem/Contract, Interpretation, Failure cases, Typical misuse, Connections.

Правило: **1 картка = 1 структура або 1 інтерфейс**.

Файли розкладені так, щоб їх було легко індексувати RAG-ом, версіонувати в Git і поступово доповнювати.

## Як розширювати
1) Берете наступну структуру/клас/enum з коду.
2) Створюєте новий `.md` з таким же шаблоном.
3) В `Connections` посилаєтесь на ID інших карток.
4) Якщо структура містить `cv::Mat` або вказівники — обов’язково додаєте ownership/lifetime припущення та типові зловживання.

## Нотація
- `cv::Mat` тип кадра в DP1 часто **CV_16UC1** (16-bit, 1 канал), але це треба завжди підтверджувати джерелом кадра.
- “Кадр” = матриця + метадані (TDataFrame / cam_pro::FrameHeader).


## Batch-2: Transport + Serialization + File formats
Додані картки для “wire/protocol” та форматів виводу (DP1 -> DP2 і файли):

- dp1.rpc.CMemStore
- dp1.rpc.rpc_data_former
- dp1.net.dp1_to_dp2_message_types
- dp1.net.dp1_tr_res2dp2_connection
- dp1.rpc.serialize_Mat
- dp1.rpc.serialize_camera_calibration_data
- dp1.rpc.serialize_frame_calibration_data
- dp1.rpc.serialize_dp1_res
- dp1.io.save_res_blob
- dp1.io.save_res_json
- dp1.io.dp1_output_filenames
