# PROTOCOLS_INDEX

## Purpose

This file is the canonical index for shared contracts between project modules: message types, wire formats, serialization rules, file exchange boundaries, and IPC/TCP handoff.

## What Belongs Here

- Shared message types between DP1 and DP2.
- Serialization contracts.
- Framing rules.
- Binary file exchange contracts when they are shared or cross-module relevant.
- Portability, compatibility, and versioning constraints.

## What Does Not Belong Here

- Local DP1 or DP2 algorithm logic.
- UI details.
- Visualization details.
- Build-system details.

## Canonical Protocol Cards

- `cards/protocols.dp1_dp2.measurement_handoff.md` - canonical DP1 -> DP2 boundary based on the DP1 Measurement domain.

## Legacy Reference Policy

Legacy DP1/DP2 transport cards describe existing runtime behavior only. They may be used for migration analysis when a task card explicitly allows legacy access, but they are not target protocol architecture.

## Initial Verification Sources

Use these only when the task requires verification against legacy/current code:
- `datapro1/src/dp1_tr_res2dp2.cpp`
- `datapro1/src/dp1_rpc_data_mrsh.cpp`
- DP2 receive-path code under `datapro2/src/*`
- relevant legacy DP1 cards under `02-dp1/legacy/protocols/`
- relevant legacy DP2 cards under `03-dp2/legacy/protocols/`
