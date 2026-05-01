# DP2_INDEX

## Purpose

This is the entry point for DP2 knowledge. It routes agents between canonical DP2 placeholders and legacy-reference knowledge for old `datapro2`.

## Current Section Content

- `canonical/DP2_CANONICAL_INDEX.md` - canonical placeholder entry point for future DP2.
- `legacy/DP2_LEGACY_INDEX.md` - legacy-reference index for old DP2.
- `DP2_CARDS_INDEX.md` - compatibility index for the former `cards/` layout.

## Covered Knowledge

Canonical DP2:
1. Product/source-of-truth placeholders.
2. Input contract placeholder linked to canonical DP1 Measurement handoff.
3. Pipeline, data-domain, configuration, and validation placeholders.

Legacy DP2:
1. Core tracking structures: `Measurement`, `PTPoint`, `TStrobe`, `Trajectory`.
2. Configuration structures: `binocular_cfg`, `dp2strobe_mth_cfg`, `dp2_cfg`, `dp2_cfg::turret_exch_cfg`.
3. Runtime boundary cards: `server`, `session`, `dp2_rpc_cl`.
4. DP1 -> DP2 receive path.
5. Cross-module overlaps with DP1 types.

## Next Work

1. Formalize canonical DP2 product responsibilities.
2. Formalize DP2 pipeline, data domains, configuration, and validation route.
3. Synchronize DP2 input with canonical DP1 -> DP2 Measurement handoff.

## Reading Route

1. For canonical development: `canonical/DP2_CANONICAL_INDEX.md`.
2. For legacy analysis: `legacy/DP2_LEGACY_INDEX.md`.
3. For compatibility navigation: `DP2_CARDS_INDEX.md`.
4. For shared canonical contracts: `04-protocols/*`.
