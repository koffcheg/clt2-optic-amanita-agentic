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

Ця секція не визначає payload schema або measurement semantics. Вона визначає тільки runtime connection policy.

## Assumptions

- DP1 -> DP2 payload contract визначається окремо в protocol cards.
- Перша codegen-ітерація не реалізує DP1 -> DP2 exchange.
- Ця card потрібна для стабілізації canonical runtime configuration structure до початку code generation.

## Theorem / Contract

`ApplicationConfig.dp2` конфігурує runtime policy для downstream handoff.

Базова форма JSON authoring:

```json
{
  "dp2": {
    "enabled": false,
    "mode": "disabled|local|network"
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
    validation: "Для першого етапу дозволені тільки `disabled` і `local`."
```

## Constraints

- `ApplicationConfig.dp2` не визначає payload schema.
- `ApplicationConfig.dp2` не визначає transport protocol details.
- `ApplicationConfig.dp2` не змінює `MeasurementRecord` semantics.
- Реальна network transport implementation потребує окремих task cards.

## Failure cases

- DP2 payload schema визначається в runtime config.
- Runtime connection policy змішується з measurement semantics.
- IPC або transport implementation додаються без окремої task.

## Connections

- belongs_to: dp1.config.application
- references: protocols.dp1_dp2.measurement_handoff
