---
id: "<canonical.id>"
title:
  uk: "<Ukrainian title, optional when bilingual title is required>"
  en: "<English title>"
tags: [tag1, tag2, tag3]
kind: "<data-domain-card | stage-interface-card | stage-spec-card | pipeline-card | config-card | protocol-card | validation-card | governance-card | legacy-reference-card>"
source_role: "<canonical | legacy-reference | verification | draft>"
source:
  file: "path/to/file"
  lines: "10-42"
status: "draft"
---

## Definition

Short definition of the entity.

## Assumptions

Assumptions about lifetime, ownership, thread-safety, data types, formats, calls, or environment.

## Theorem / Contract

What is guaranteed. Include input/output conditions and constraints that must not be violated.

## Interpretation

How this entity should be read in the project architecture and what role it has.

## Failure cases

Typical failures, invalid states, portability problems, undefined behavior, format mismatches, or lifetime issues.

## Typical misuse

Common incorrect uses.

## Open questions

Facts that are not yet code-verified or require confirmation. Use `TODO: confirm with user` when the meaning is unclear.

## Connections

- used_by: <other.card.id>
- produces: <other.card.id>
- overlaps_with: <other.card.id>

## Codegen use

`source_role` controls whether a card may be used as a source for canonical code
generation.

Default rule:
- `source_role: canonical` cards are eligible as canonical code-generation
  inputs when they contain formal contracts and the required stage specification
  and configuration `C` are present.
- `source_role: legacy-reference` cards must not be used for code generation
  unless the active task card explicitly allows legacy access and lists the
  exact legacy sources.
- `source_role: verification` cards may constrain validation and acceptance, but
  they do not replace product, data, protocol, stage, or configuration cards.
- `source_role: draft` cards are not sufficient for code generation until their
  status and source role are promoted.
