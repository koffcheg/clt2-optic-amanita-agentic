# DP1_CANONICAL_INDEX

## Purpose

Canonical entry point for DP1 as an engineering product.

This section defines target DP1 architecture independently from legacy `datapro1` code. Legacy materials may be used only when a task card explicitly allows them.

## Source of truth route

1. `product/dp1.canonical.product_definition.md`
2. `product/dp1.canonical.source_of_truth.md`
3. `pipeline/dp1.pipeline.formal_model.md`
4. `pipeline/dp1.pipeline.stage_contract.md`
5. `data_domains/*.md`
6. `stages/*.md`
7. `configuration/dp1.config.pipeline_configuration_c.md`
8. `configuration/dp1.config.complexity_levels.md`
9. `validation/dp1.validation.canonical_conformance.md`
10. `../../04-protocols/cards/protocols.dp1_dp2.measurement_handoff.md`

## Code generation rule

```text
Code = f(Cards, Stage_Spec, C)
```

Code generation from informal text, legacy code, or legacy-reference cards is forbidden.

Stage specifications are not written in this section yet. Stage-interface cards
define boundaries, Big-TZ-derived interface constraints, complexity variants,
OpenCV mapping, configuration fragments, data formats, and critical invariants.
They still are not sufficient for code generation without future small stage
specifications.

The current canonical stage-interface card set covers only the eight main DP1
detection/measurement stages. Infrastructure stages such as visualization and
persistence are intentionally not described in this pass.

## OpenCV boundary

OpenCV provides low-level primitives and storage types. It does not define DP1 architecture. DP1 architecture is defined by data domains, stage contracts, pipeline composition, configuration `C`, and validation rules.
