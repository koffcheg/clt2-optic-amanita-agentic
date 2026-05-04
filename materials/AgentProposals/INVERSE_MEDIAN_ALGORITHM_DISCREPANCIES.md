# Inverse Median Algorithm Discrepancies

Date: 2026-05-03

Scope: comparison between raw technical requirements under `materials/ТЗ/`,
canonical Project Knowledge for `inverse_median`, and the current
`datapro1_v2` implementation.

No Project Knowledge files are changed by this note.

## Source Layers

Raw requirements:
- `materials/ТЗ/ТЗ_Міжкадровий_попіксельний_медіанний_фільтр_оновлене.md`
- `materials/ТЗ/ТЗ_Міжкадровийv2_попіксельний_медіанний_фільтр_лише.docx`
- `materials/ТЗ/Уточнення Міжкадровий попіксельний медіанний фільтр.docx`
- `materials/ТЗ/Бітність результату міжкадового медіанного фільтру.docx`

Canonical sources:
- `project-knowledge/02-dp1/canonical/stage_specs/dp1.stage_spec.radiometric_correction.inverse_median.md`
- `project-knowledge/02-dp1/canonical/stages/dp1.stage.radiometric_correction.md`
- `project-knowledge/02-dp1/canonical/configuration/dp1.config.pipeline_configuration_c.md`
- `project-knowledge/05-validation/cards/validation.dp1.radiometric_correction.inverse_median.md`

Current code:
- `datapro1_v2/include/dp1v2/stages/inverse_median.hpp`
- `datapro1_v2/src/stages/inverse_median.cpp`

## Summary

The current implementation follows the canonical specification more closely than
the older raw requirements. The main discrepancies are:

1. The `FixedK5` sorting network written in the raw requirements is incorrect.
2. Older raw requirements include `ArbitraryK8bit`, `Centered`, and
   `SelectedOnly`, but canonical specification excludes them from scope.
3. Runtime `stride` changes are requested in raw clarification, but canonical
   specification leaves this as an open question and the current code does not
   implement a runtime update policy.
4. Raw requirements ask to select the median kernel once during initialization;
   the current code has separate kernels but still branches on `window_size_`
   inside the median-frame pixel loop.
5. `ShiftToPositive` and `ScaleToInputRange` conversion semantics still have
   underspecified details around unsigned output clipping and negative values.

## Discrepancy 1: Raw `FixedK5` Sorting Network Is Incorrect

### Raw Requirement

The updated raw requirement defines `FixedK5` as a fixed compare-swap network:

```cpp
cswap(a, b);
cswap(d, e);
cswap(c, e);
cswap(c, d);
cswap(a, d);
cswap(b, e);
cswap(b, c);
cswap(a, c);
cswap(b, d);
return c;
```

### Problem

This network does not always return the median.

Counterexample:

```text
input:    a=0, b=3, c=1, d=2, e=4
sorted:   [0, 1, 2, 3, 4]
median:   2
raw TЗ:   3
```

Another counterexample with repeated values:

```text
input:    a=0, b=1, c=0, d=0, e=1
sorted:   [0, 0, 0, 1, 1]
median:   0
raw TЗ:   1
```

### Canonical Position

Canonical specification requires exact temporal median for `FixedK5`; it does
not require copying the raw compare-swap sequence.

### Current Code

The current code uses a corrected fixed compare-swap sequence:

```cpp
cswap(a, b);
cswap(d, e);
cswap(c, e);
cswap(c, d);
cswap(b, e);
cswap(a, d);
cswap(a, c);
cswap(b, d);
cswap(b, c);
return c;
```

This differs from raw requirements but satisfies the canonical requirement for
an exact `FixedK5` median.

## Discrepancy 2: `ArbitraryK8bit` Appears In Older Raw Requirements Only

### Raw Requirement

`ТЗ_Міжкадровийv2_попіксельний_медіанний_фільтр_лише.docx` mentions:

- `FixedK3`
- `FixedK5`
- `ArbitraryK8bit` for `K = 15..50`, only for `uint8`

### Canonical Specification

Canonical specification explicitly places `ArbitraryK8bit` outside the current
scope.

### Current Code

Current code implements only:

- `FixedK3`
- `FixedK5`

### Interpretation

This is a raw-to-canonical scope reduction, not a code defect.

## Discrepancy 3: `Centered` And `SelectedOnly` Are Raw/Legacy Options Only

### Raw Requirement

Older raw requirements describe:

- `Centered`
- `Causal`
- `SelectedOnly`
- `HoldLastMedian`

Clarification says the intended mode is:

- only `Causal`
- only `HoldLastMedian`

### Canonical Specification

Canonical specification explicitly excludes:

- `Centered`
- `SelectedOnly`

### Current Code

Current code implements:

- causal processing;
- no future frames;
- median update only for selected frames;
- residual output for every frame after warm-up using the last valid median.

### Interpretation

Code aligns with canonical specification and with the clarified raw intent, but
not with the broader older raw option set.

## Discrepancy 4: Runtime `stride` Update Is Not Defined Canonically

### Raw Requirement

Raw clarification states that `stride` may change during camera runtime via
configuration/adaptation.

### Canonical Specification

Canonical specification lists runtime `stride` change policy as an open
question:

- whether buffer reset is required;
- whether continuing with current state is allowed;
- exact runtime disable/re-enable policy.

### Current Code

Current code:

- validates `stride >= 1` in constructor;
- stores config as immutable runtime state;
- does not provide a runtime `setStride()` or reconfiguration method;
- supports state reset through `reset()`.

### Interpretation

This is an unresolved product/API policy, not a proven algorithmic bug.
Implementation should not invent runtime reconfiguration semantics until the
canonical specification is updated.

## Discrepancy 5: Kernel Dispatch Is Not Selected Once Outside The Pixel Loop

### Raw Requirement

Raw requirements say:

- use separate kernels for `FixedK3` and `FixedK5`;
- select the kernel once during construction or initialization.

### Current Code

Current code has separate pixel kernels:

- `fixedK3Median()`
- `fixedK5Median()`

But `recomputeMedianFrameTyped()` still checks `window_size_` inside the
per-pixel loop:

```cpp
if (window_size_ == 3) {
    median_row[x] = fixedK3Median(...);
} else {
    median_row[x] = fixedK5Median(...);
}
```

### Canonical Specification

Canonical specification requires fixed operations per pixel and stable runtime
independent of pixel statistics. It does not explicitly require dispatch outside
the pixel loop.

### Interpretation

The code is algorithmically correct, but performance structure is not as strict
as the raw implementation guidance. A later optimization could split median
frame recomputation into separate `FixedK3` and `FixedK5` frame kernels.

## Discrepancy 6: Output Conversion Semantics Are Not Fully Formalized

### Raw And Canonical Agreement

All layers agree that internal residual is signed:

- `uint8 -> int16`
- `uint16 -> int32`

All layers also agree that converted user-facing output may use:

- `RawSigned`
- `ClipToInputRange`
- `ShiftToPositive`
- `ScaleToInputRange`

### Ambiguity

`ShiftToPositive` is defined as:

```text
Residual_shift = Residual - min(Residual)
```

`ScaleToInputRange` is defined as:

```text
alpha = range_input / range_residual
Residual_scaled = alpha * Residual
```

However, the canonical specification does not fully define:

- whether shifted values above `Imax` must saturate;
- how negative scaled values are represented in `uint8` / `uint16`;
- whether `ScaleToInputRange` should preserve the formula exactly or map
  `[min, max]` into `[0, Imax]`;
- behavior when `range_residual == 0`.

### Current Code

Current code:

- clamps converted output to the input unsigned range;
- uses `Residual - min(Residual)` for `ShiftToPositive`;
- uses `alpha * Residual` for `ScaleToInputRange`;
- writes zero frame when `range_residual == 0`.

### Interpretation

The current implementation is practical, but conversion semantics need a more
precise canonical policy before integration with user-facing output or file/IPC
contracts.

## Areas With No Meaningful Discrepancy

The following points are aligned between raw clarification, canonical
specification, and current code:

- temporal per-pixel median only;
- no spatial median window;
- no OpenCV `medianBlur` as implementation of МКМФ;
- input is single-channel grayscale;
- supported input pixel types are `uint8` and `uint16`;
- `Med_t` keeps input pixel type;
- residual formula is `I_t - Med_t`;
- residual uses signed widened type;
- `stride >= 1`;
- selected frames follow `t % stride == 0` with frame counter starting at `0`;
- no valid residual before selected-frame buffer is full;
- residual is emitted for every input frame after warm-up;
- non-selected frames use last computed median;
- buffers are allocated in construction/reset path, not intentionally inside
  `processFrame`;
- `RawSigned` is the internal default;
- `ClipToInputRange` is the recommended user/compatibility mode.

## Recommended Follow-Up

1. Promote or correct the `FixedK5` compare-swap network in the canonical
   specification if exact kernel sequence should be documented.
2. Decide runtime `stride` update policy:
   - reset buffer;
   - continue with current state;
   - unsupported without restart.
3. Decide runtime disable/re-enable state policy.
4. Formalize conversion semantics for:
   - saturation after `ShiftToPositive`;
   - negative values under `ScaleToInputRange`;
   - zero residual range.
5. Optionally split `FixedK3` and `FixedK5` frame-level recomputation paths to
   remove mode dispatch from the pixel loop.
