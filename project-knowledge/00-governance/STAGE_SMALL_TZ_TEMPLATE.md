# STAGE_SMALL_TZ_TEMPLATE

## Purpose

Reusable small-TZ template for canonical DP1/DP2 stage specifications.

Use this template when a stage-interface card exists, but implementation-grade
constraints are not yet formalized.

## Metadata

```yaml
id: <stage_spec_id>
title:
  uk: "<short title uk>"
  en: "<short title en>"
tags: [<dp1|dp2>, canonical, stage-spec, small-tz]
kind: stage-spec-card
source_role: canonical
source:
  file: "<path-to-this-card>"
  lines: "1-N"
status: "draft"
```

## Scope

- Stage interface: `<IStageInterface>`
- Pipeline slot: `<pipeline.stage_name>`
- Complexity level: `<L0|L1|L2>`
- Variant: `<variant_name>`

## Inputs

- Upstream domain card(s):
  - `<card-id-1>`
  - `<card-id-2>`
- Runtime input format/type constraints.

## Outputs

- Downstream domain card(s):
  - `<card-id-3>`
- Output format/type constraints.

## Preconditions

- Required config fragment keys.
- Required context/runtime state.
- Explicit ownership of mutable state, if any.

## Deterministic flow

1. `<step-1>`
2. `<step-2>`
3. `<step-3>`

For each step, state exact input/output contract and allowed side effects.

## Configuration contract

Canonical `C` fragment:

```json
{
  "<pipeline_stage_key>": {
    "enabled": "<bool>",
    "variant": "<enum>",
    "level": "<enum>",
    "parameters": {
      "<key>": "<value-type>"
    }
  }
}
```

Rules:
- Unknown keys: `<forbidden|ignored-with-warning>`.
- Missing required keys: `<error mode>`.
- Defaulting policy: `<explicit defaults or forbidden>`.

## Invariants

- Functional invariants that must hold for every invocation.
- Data-shape and type invariants at stage boundaries.
- Monotonicity/stability constraints, if relevant.

## Failure cases

- `<failure-case-1>` -> expected handling.
- `<failure-case-2>` -> expected handling.

## Non-goals

- Explicitly list what this stage must not do.

## Validation hooks

- Metrics/counters/profiling points.
- Conformance checks mapped to `05-validation` knowledge.

## Source-of-truth and canonicalization safety

- This stage spec is a canonical target-architecture source.
- Runtime/build/config/protocol behavior must still be verified against code
  and authoritative environment/config/protocol files before execution claims.
- If canonical target and observed runtime behavior diverge, report mismatch and
  propose minimal synchronization scope; do not silently rewrite either side.

## Connections

- Stage interface card: `<dp1.stage.* | dp2.stage.*>`
- Pipeline model card: `<*.pipeline.*>`
- Relevant protocol/config cards: `<protocols.* / *.config.*>`
