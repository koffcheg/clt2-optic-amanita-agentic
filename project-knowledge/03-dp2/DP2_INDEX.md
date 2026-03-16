# DP2_INDEX

## Призначення

Це вхідна точка knowledge base для DP2 (`datapro2`).
Файл маршрутизує до структурних карток і джерел коду для подальшого розширення знань.

## Поточний вміст розділу

- `DP2_CARDS_INDEX.md` - індекс структурних карток DP2
- `cards/*.md` - атомарні картки структур DP2

## Що вже покрито

1. Core структури трекінгу: `Measurement`, `PTPoint`, `TStrobe`, `Trajectory`.
2. Конфігураційні структури: `binocular_cfg`, `dp2strobe_mth_cfg`, `dp2_cfg`, `dp2_cfg::turret_exch_cfg`.
3. Runtime boundary cards: `server`, `session`, `dp2_rpc_cl`.
4. Receive path DP1 -> DP2 (TCP stream -> framed message -> payload dispatch).
5. Перетини з DP1-типами (через `Connections` у DP2 cards).

## Що лишається додати далі

1. За потреби, винесення shared protocol facts у `04-protocols/`.
2. Розширення runtime cards у бік turret exchange lifecycle та помилкових сценаріїв reconnect.

## Рекомендований маршрут читання

1. `DP2_CARDS_INDEX.md`
2. `cards/*.md`
3. `datapro2/src/*`
4. `datapro2/config/*`
5. `04-protocols/*` для shared контрактів
