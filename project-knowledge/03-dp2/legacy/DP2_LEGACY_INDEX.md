# DP2_LEGACY_INDEX

## Purpose

Legacy reference index for existing DP2 knowledge cards moved from the former `03-dp2/cards/` directory.

Legacy cards describe current `datapro2` behavior. They are not target architecture for canonical DP2.

## Access rule

During canonical DP2 development, this section is forbidden unless the active task card has `legacy_access: allowed` and lists the exact legacy sources.

## Classification

### Data structures

- `data_structures/dp2.types.Measurement.md`
- `data_structures/dp2.types.PTPoint.md`
- `data_structures/dp2.types.TStrobe.md`
- `data_structures/dp2.types.Trajectory.md`

### Configuration

- `configuration/dp2.config.binocular_cfg.md`
- `configuration/dp2.config.dp2_cfg.md`
- `configuration/dp2.config.dp2_cfg.turret_exch_cfg.md`
- `configuration/dp2.config.dp2strobe_mth_cfg.md`

### Runtime

- `runtime/dp2.runtime.server.md`
- `runtime/dp2.runtime.session.md`

### Protocols

- `protocols/dp2.net.dp1_to_dp2_receive_path.md`
- `protocols/dp2.rpc.dp2_rpc_cl.md`

## Migration notes

These cards may support migration analysis only. Canonical DP2 rules must be restated in `canonical/` before they can guide implementation.
