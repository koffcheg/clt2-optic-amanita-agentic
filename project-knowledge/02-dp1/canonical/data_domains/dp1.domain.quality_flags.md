---
id: dp1.domain.quality_flags
title: "Canonical-прапори якості та відсіву DP1"
tags: [dp1, canonical, data-domain, quality, flags]
kind: data-domain-card
source_role: canonical
source:
  file: "project-knowledge/02-dp1/canonical/data_domains/dp1.domain.quality_flags.md"
status: "draft"
---

## Definition

Ця картка визначає canonical policy для bounded quality і reject flags у DP1
Struct і Measurement domains.

## Assumptions

- Hot path має використовувати compact numeric flags, а не strings.
- Human-readable mapping може існувати в diagnostics або documentation layer.
- Exact flag registry може розширюватися через controlled Project Knowledge
  updates.

## Theorem / Contract

Canonical runtime representation:

```cpp
using QualityFlags = std::uint32_t;
using RejectFlags = std::uint32_t;
```

Initial canonical bit registry:

```yaml
flags:
  - bit: "`0`"
    name: "`BorderTouched`"
    role: "Object/segment торкається tile або frame border."
  - bit: "`1`"
    name: "`TooSmall`"
    role: "Area менше config minimum."
  - bit: "`2`"
    name: "`TooLarge`"
    role: "Area більше config maximum."
  - bit: "`3`"
    name: "`LowContrast`"
    role: "Photometry/response contrast lower than threshold."
  - bit: "`4`"
    name: "`Saturated`"
    role: "Source pixels або photometry мають saturation."
  - bit: "`5`"
    name: "`ShapeInvalid`"
    role: "Shape/geometry criteria failed."
  - bit: "`6`"
    name: "`PartialFrame`"
    role: "Result formed from partial/incomplete frame area."
  - bit: "`7`"
    name: "`DuplicateSuppressed`"
    role: "Merge detected duplicate/border overlap."
```

`quality_flags` описують bounded properties result. `reject_flags` описують
причини rejection у `ValidatedObject`.

## Interpretation

Bit flags мінімізують memory, добре серіалізуються і дозволяють parallel workers
записувати independent object records без shared mutable text buffers.

## Failure cases

- Reject reason зберігається тільки як free-form diagnostic string.
- Один bit має різне значення в різних stages.
- Quality flags потрапляють у DP2 без registry/version awareness.

## Typical misuse

- Використовувати flags як прихований algorithm state.
- Змішувати quality і rejection semantics в одному полі без contract.

## Open questions

- Versioning policy для flag registry.
- Чи потрібен окремий severity field.

## Connections

- constrains: dp1.domain.struct.candidate
- constrains: dp1.domain.struct.segment
- constrains: dp1.domain.struct.validated_object
- constrains: dp1.domain.measurement.record
- informs: protocols.dp1_dp2.measurement_handoff
