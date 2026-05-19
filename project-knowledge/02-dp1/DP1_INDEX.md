# DP1_INDEX

## Purpose

This is the entry point for DP1 knowledge. It routes agents between canonical DP1 target knowledge and legacy-reference knowledge for old `datapro1`.

## Current Section Content

- `canonical/DP1_CANONICAL_INDEX.md` - canonical entry point for new DP1.
- `legacy/DP1_LEGACY_INDEX.md` - legacy-reference index for old DP1.
- `DP1_CARDS_INDEX.md` - compatibility index for the former `cards/` layout.

## Covered Knowledge

Canonical DP1:
1. Product definition and source-of-truth route.
2. Formal pipeline model `Π`.
3. Data domains: Raw, Processing, Mask, Measurement, Visualization, conversion rules.
4. Stage0 input normalization plus eight main stage-interface cards derived from the Big TZ interface requirements.
5. Configuration model `C` and complexity levels.
6. Canonical conformance validation.

Legacy DP1:
1. DP1 data structures, including `TData*`, `TOptionsMeasurement`, `TDrawMeasurement`, `TFolder`.
2. Frame processing boundary.
3. Runtime buffers and ingest/threading structures.
4. Configuration layer.
5. Preprocessing and tile-related behavior.
6. RPC serialization primitives and DP1 -> DP2 legacy transport.
7. File output contracts.
8. Runtime lifecycle/orchestration contracts.

## Next Work

1. Prepare Stage0.1 implementation from the canonical Input Normalization and CanonicalFrame baseline.
2. Write full small stage specifications for canonical DP1, including Stage0.2 binning before implementation.
3. Define the complete configuration schema `C`.
4. Define canonical Measurement payload schema.

## Reading Route

1. For canonical development: `canonical/DP1_CANONICAL_INDEX.md`.
2. For legacy analysis: `legacy/DP1_LEGACY_INDEX.md`.
3. For compatibility navigation: `DP1_CARDS_INDEX.md`.
