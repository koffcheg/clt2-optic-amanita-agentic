---
id: dp1.config.application.dp2
title:
  uk: "Canonical-конфігурація підключення DP1 -> DP2"
  en: "Canonical DP1 to DP2 connection configuration"
tags: [dp1, canonical, config, application, dp2, runtime]
kind: config-card
source_role: canonical
source:
  file: "project-knowledge/02-dp1/canonical/configuration/dp1.config.application.dp2.md"
status: "draft"
---

## Definition

`ApplicationConfig.dp2` визначає runtime-поведінку підключення DP1 application до downstream DP2 boundary.

Ця секція не визначає payload schema або measurement semantics. Вона визначає runtime connection policy і legacy-compatible connection fields для поточного DP2 boundary.

## Assumptions

- DP1 -> DP2 payload contract визначається окремо в protocol cards.
- DP2 наразі є legacy boundary, тому runtime config має підтримувати legacy-compatible connection shape.
- Перша codegen-ітерація не реалізує DP1 -> DP2 exchange.
- Ця card потрібна для стабілізації canonical runtime configuration structure до початку code generation.

## Theorem / Contract

`ApplicationConfig.dp2` конфігурує runtime policy для downstream handoff.

Базова форма JSON authoring:

```json
{
  "dp2": {
    "enabled": false,
    "mode": "disabled|local|network",
    "host": "127.0.0.1",
    "port": 11511,
    "reconnect_interval_s": 3
  }
}
```

Legacy-compatible authoring input may also be mapped from the historical `dp2conn` section:

```json
{
  "dp2conn": {
    "host": "127.0.0.1",
    "port": 11511,
    "reconnect_interval_s": 3
  }
}
```

Typed canonical config model:

```cpp
enum class DP2ConnectionMode {
    Disabled,
    Local,
    Network
};

struct DP2ConnectionConfig {
    bool enabled = false;
    DP2ConnectionMode mode = DP2ConnectionMode::Disabled;
    std::string host = "127.0.0.1";
    int port = 11511;
    int reconnect_interval_s = 3;
};
```

## Fields / Interface

```yaml
fields:
  - name: "`enabled`"
    type: "`bool`"
    required: true
    default: false
    purpose: "Дозвіл runtime handoff до DP2 boundary."
    does_not_affect: "Не змінює pipeline algorithms або measurement semantics."

  - name: "`mode`"
    type: "`DP2ConnectionMode`"
    required: true
    default: "`Disabled`"
    purpose: "Вибір runtime connection policy."
    validation: "Для першого етапу дозволені тільки `disabled`, `local` і legacy-compatible network settings без transport rewrite."

  - name: "`host`"
    type: "`std::string`"
    required_when: "`enabled == true` або legacy `dp2conn` import"
    default: "`127.0.0.1`"
    purpose: "Host поточного legacy DP2 endpoint."
    validation: "Має бути non-empty string."

  - name: "`port`"
    type: "`int`"
    required_when: "`enabled == true` або legacy `dp2conn` import"
    default: 11511
    purpose: "TCP port поточного legacy DP2 endpoint."
    validation: "Має бути в діапазоні 1..65535."

  - name: "`reconnect_interval_s`"
    type: "`int`"
    required_when: "`enabled == true` або legacy `dp2conn` import"
    default: 3
    purpose: "Інтервал повторного підключення до legacy DP2 endpoint у секундах."
    validation: "Має бути додатним integer value."
```

## Legacy compatibility

Historical DP1 configs used `dp2conn` as the connection section:

```json
{
  "dp2conn": {
    "host": "127.0.0.1",
    "port": 11511,
    "reconnect_interval_s": 3
  }
}
```

Canonical code may map this legacy authoring shape into `ApplicationConfig.dp2`, but canonical runtime code should use the typed `DP2ConnectionConfig` model after parsing/validation.

## Constraints

- `ApplicationConfig.dp2` не визначає payload schema.
- `ApplicationConfig.dp2` не змінює `MeasurementRecord` semantics.
- `host`, `port` і `reconnect_interval_s` існують для legacy-compatible connection policy, а не для перепроєктування DP2 protocol.
- Реальна network transport implementation або protocol rewrite потребує окремих task cards.

## Failure cases

- DP2 payload schema визначається в runtime config.
- Runtime connection policy змішується з measurement semantics.
- Legacy `dp2conn` shape використовується як новий canonical object замість mapping до `ApplicationConfig.dp2`.
- IPC або transport implementation додаються без окремої task.

## Connections

- belongs_to: dp1.config.application
- references: protocols.dp1_dp2.measurement_handoff
