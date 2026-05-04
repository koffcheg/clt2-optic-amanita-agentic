# UNIT_TESTING_GUIDE

## Purpose

This guide defines target unit-testing rules for the project.

It is a technical guide, not a governance policy. Agent permissions, approval rules, and source-of-truth order are defined in `project-knowledge/00-governance/TESTING_POLICY.md`.

## Unit Test Definition

A unit test verifies one small behavior in isolation from the full system.

Suitable units include:
- pure functions;
- small classes;
- data conversions;
- parameter validation;
- index and coordinate calculations;
- small algorithmic operations;
- parsing logic that can be exercised without external runtime state.

A unit test must be:
- deterministic;
- independent from other tests;
- fast enough for frequent local execution;
- explicit about expected results;
- structured so the checked behavior is clear from the test name and assertions.

Use the Arrange, Act, Assert structure:
- Arrange: prepare input data and controlled dependencies.
- Act: call the function or method under test.
- Assert: verify concrete output, state, or invariant.

## Visual Unit Tests

`visual-unit` is a subtype of `unit` for computer-vision logic.

A visual unit test uses a small controlled visual input and a precise expected result, property, or invariant. The input is usually a synthetic in-memory object such as:
- a small `cv::Mat`;
- a mask;
- a bounding box;
- a short synthetic frame sequence;
- a simple geometric scene encoded as pixels.

Prefer synthetic in-memory inputs over file fixtures for unit and visual-unit tests.

Examples of visual-unit checks:
- matrix size, type, channel count, and value range;
- exact mask equality for fully controlled synthetic masks;
- number of non-zero pixels;
- bounding-box coordinates;
- object center or area;
- IoU for region comparison;
- norm or mean absolute difference for approximate image comparison;
- invariant checks such as coordinates staying inside frame bounds.

Visual-unit coverage should include meaningful visual scenarios, not only line coverage:
- empty frame;
- one object;
- multiple objects;
- object near frame boundary;
- object near tile or segment boundary;
- partial visibility;
- noise;
- low contrast;
- different frame sizes;
- supported matrix types;
- invalid parameters.

## Target Framework

GoogleTest is the target C++ unit-test framework.

CTest is the target CMake-level test runner when tests are integrated into the build.

Do not add or change test infrastructure without approval.

## Production Code Access

Production code should be tested through library targets or stable public/internal interfaces.

Do not include production `.cpp` files directly in test files to reach implementation details. If code is not testable without direct `.cpp` inclusion, propose a small extraction into a testable function, class, or library target.

## Testable Unit First

Before proposing integration, system-e2e, or run-based validation for algorithmic or transformation logic, identify whether there is a smaller testable unit.

Prefer a unit or visual-unit test plan when behavior can be checked through:
- a pure function;
- a small class;
- a parser or validator;
- a converter or mapper;
- a deterministic algorithmic operation.

Use broader validation when:
- the behavior is inherently about component interaction;
- the risk is in serialization, runtime wiring, or process boundaries;
- the small unit already has coverage but end-to-end confidence is still needed;
- the user explicitly asks for run-based validation.

If logic is too tightly coupled to runtime code, propose minimal extraction into a testable function, class, or library target. Do not replace a missing unit boundary with a full pipeline run unless the task explicitly requires system validation.

Examples:
- coordinate remapping after binning should first be checked with synthetic points or bounding boxes; Amanita + Comparator can then provide regression evidence;
- temporal median residual logic should first be checked on a small synthetic frame sequence; a full pipeline run should not be the only proof of correctness;
- config parsing should first be checked through a parser or validator unit; binary execution is an integration or system-e2e concern.

## Assertions

Choose assertions by contract:
- exact equality for integers and fully controlled small matrices;
- tolerance for floating-point values;
- absolute or relative error for numeric results;
- norm or mean absolute difference for approximate image comparison;
- IoU for region and bounding-box overlap;
- explicit invariant checks when exact output is not part of the contract.

Do not test OpenCV itself. Test the project behavior and the guarantees the project code gives when it uses OpenCV.

## Naming

Use behavior-oriented names:

```text
FunctionOrClass_WhenCondition_ExpectedResult
```

Examples:

```text
ComputeTileIndex_WhenSecondRowAndThirdColumn_ReturnsExpectedLinearIndex
ConvertTo8Bit_WhenInputIs16Bit_PreservesSizeAndScalesValues
ExtractComponents_WhenMaskIsEmpty_ReturnsNoObjects
MapDp1Result_WhenObjectIsConfirmed_PreservesFrameIndexAndCenterPoint
```

The name should document the scenario and expected behavior, not only repeat the function name.

## Test Data

Default unit-test data is synthetic and created in the test.

File fixtures are not the default for unit tests. Use them only when:
- the checked behavior is still a small unit;
- the fixture is minimal;
- the expected result is explicit;
- the fixture has been approved.

Large datasets, generated snapshots, golden files, and benchmark inputs require separate approval.

## Stubs, Fakes, And Mocks

Use controlled substitutes only to isolate the unit under test:
- stub: returns prepared data;
- fake: simplified working implementation, usually in-memory;
- mock: verifies interaction such as call count or arguments.

Avoid interaction-only tests that do not verify useful project behavior.

Creating new stubs, fakes, mocks, or fixture infrastructure requires approval under `TESTING_POLICY.md`.

## Boundary With Other Test Types

A check is not a unit test when it depends on:
- a real camera;
- network or IPC runtime;
- Docker or full deployment environment;
- full DP1/DP2 pipeline execution;
- large dataset layout;
- external services;
- long-running benchmark conditions.

Classify those checks as `integration`, `accuracy-regression`, `performance`, `system-e2e`, or `manual-run-based validation` according to `TESTING_POLICY.md`.

## References

- GoogleTest Primer: https://google.github.io/googletest/primer.html
- GoogleTest repository and documentation: https://github.com/google/googletest
- OpenCV QA in OpenCV: https://github.com/opencv/opencv/wiki/QA_in_OpenCV
- CMake, OpenCV and Unit Tests: https://www.incredibuild.com/blog/cmake-opencv-and-unit-tests
- ViUniT: Visual Unit Tests for More Robust Visual Programming, arXiv:2412.08859
