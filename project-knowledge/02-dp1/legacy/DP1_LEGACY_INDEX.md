# DP1_LEGACY_INDEX

## Purpose

Legacy reference index for existing DP1 knowledge cards moved from the former `02-dp1/cards/` directory.

Legacy cards describe current or historical `datapro1` behavior. They are not target architecture for canonical DP1.

## Access rule

During canonical DP1 development, this section is forbidden unless the active task card has `legacy_access: allowed` and lists the exact legacy sources.

## Classification

### Data structures and file artifacts

- `data_structures/dp1.types.TDataCalibrationCamera.md`
- `data_structures/dp1.types.TDataCalibrationFrame.md`
- `data_structures/dp1.types.TDataCam.md`
- `data_structures/dp1.types.TDataFrame.md`
- `data_structures/dp1.types.TDataRes.md`
- `data_structures/dp1.types.TDataproConfig.md`
- `data_structures/dp1.types.TDataproVar.md`
- `data_structures/dp1.types.TDrawMeasurement.md`
- `data_structures/dp1.types.TFolder.md`
- `data_structures/dp1.types.TOptionsMeasurement.md`
- `data_structures/dp1.frame.frame_n_header.md`
- `data_structures/dp1.io.dp1_output_filenames.md`
- `data_structures/dp1.io.save_res_blob.md`
- `data_structures/dp1.io.save_res_json.md`

### Runtime

- `runtime/dp1.ipc.ipc_data_rc.md`
- `runtime/dp1.ipc.ipc_data_rc_impl.md`
- `runtime/dp1.runtime.dp1_main_orchestration.md`
- `runtime/dp1.runtime.dp1_th_proc_par.md`
- `runtime/dp1.runtime.rc_ipc_raw_frame.md`
- `runtime/dp1.runtime.rc_uri_raw_frame.md`
- `runtime/dp1.runtime.run_ipc_src.md`
- `runtime/dp1.runtime.run_uri_src.md`

### Pipeline

- `pipeline/dp1.frame.frame_processor.md`
- `pipeline/dp1.preproc.binning_sum.md`
- `pipeline/dp1.runtime.legacy_pipeline.md`
- `pipeline/dp1.tiles.i_calc_tile_limit.md`
- `pipeline/dp1.tiles.snail_path.md`

### Protocols

- `protocols/dp1.net.dp1_to_dp2_message_types.md`
- `protocols/dp1.net.dp1_tr_res2dp2_connection.md`
- `protocols/dp1.rpc.CMemStore.md`
- `protocols/dp1.rpc.rpc_data_former.md`
- `protocols/dp1.rpc.serialize_Mat.md`
- `protocols/dp1.rpc.serialize_camera_calibration_data.md`
- `protocols/dp1.rpc.serialize_dp1_res.md`
- `protocols/dp1.rpc.serialize_frame_calibration_data.md`

### Configuration

- `configuration/dp1.config.calc_tile_lim_cfg_t.md`
- `configuration/dp1.config.prg_config.md`

## Migration notes

These cards may inform migration analysis, but any canonical rule must be restated in `canonical/` before it can drive implementation.
