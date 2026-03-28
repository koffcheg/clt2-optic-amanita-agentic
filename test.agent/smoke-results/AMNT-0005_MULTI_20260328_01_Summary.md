# Підсумок multi-run тесту

- test_id: AMNT-0005_MULTI_20260328_01
- test_date: 2026-03-28 19:08:02
- dataset: frames_with_gt/SimpleVideo10Balls (multi-run)
- test_description: Auto-generated aggregated summary for multi-run scenario.
- stages_count: 2

## Загальний підсумок multi-run
- dp1_total_objects_all_stages: 1530
- dp2_total_objects_all_stages: 1002
- comparator_report_files_total: 4
- comparator_frames_compared_total: 360
- comparator_mean_overlap_pct_weighted: 96.04
- comparator_mean_rms_deviation_area_weighted: 2.84
- comparator_mean_false_positives_pct_weighted: 1.73
- comparator_mean_false_negatives_pct_weighted: 20.12

## Stage S01_BASELINE
- dp1_total_objects_all_frames: 765
- dp1_execution_time_sec: 127.72
- dp2_total_objects_all_frames: 501
- dp2_execution_time_sec: 131.197
- comparator_execution_time_sec: 3.83
- comparator_report_files: 2
- comparator_frames_compared_total: 180
- comparator_mean_overlap_pct: 96.04
- comparator_mean_rms_deviation_area: 2.84
- comparator_mean_false_positives_pct: 1.73
- comparator_mean_false_negatives_pct: 20.12
- comparator_reports:
  - 0_modeled_vs_dp1.json: frames=90, mean_overlap_pct=93.4, mean_rms=1.86
  - 1_modeled_vs_dp2.json: frames=90, mean_overlap_pct=98.68, mean_rms=3.81

## Stage S02_CANDIDATE
- dp1_total_objects_all_frames: 765
- dp1_execution_time_sec: 127.72
- dp2_total_objects_all_frames: 501
- dp2_execution_time_sec: 131.197
- comparator_execution_time_sec: 3.83
- comparator_report_files: 2
- comparator_frames_compared_total: 180
- comparator_mean_overlap_pct: 96.04
- comparator_mean_rms_deviation_area: 2.84
- comparator_mean_false_positives_pct: 1.73
- comparator_mean_false_negatives_pct: 20.12
- comparator_reports:
  - 0_modeled_vs_dp1.json: frames=90, mean_overlap_pct=93.4, mean_rms=1.86
  - 1_modeled_vs_dp2.json: frames=90, mean_overlap_pct=98.68, mean_rms=3.81

## Result summary
- PASS: multi-run stages aggregated into one summary report.
