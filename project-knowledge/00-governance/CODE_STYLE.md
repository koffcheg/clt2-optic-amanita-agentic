# CODE_STYLE.md

## Purpose

This file defines code-writing rules for this repository.

It is a governance document for human and AI-assisted code changes. It does not
define task routing, test execution, project architecture, dependency versions,
or canonical DP1/DP2 product contracts.

For related policy:
- task-card and legacy-access rules: `AGENTS.md` and `TASK_CARD_TEMPLATE.md`;
- testing and validation limits: `TESTING_POLICY.md`;
- language standards, libraries, OS, Docker, and build facts: `PROJECT_ECOSYSTEM.md`;
- canonical DP1/DP2 contracts: the relevant canonical cards and stage specs.

## Status

Status: active.

These rules apply to new code and to code changed by the current task. Existing
files may be inconsistent; do not mass-reformat or rewrite unrelated code only to
make it match this document.

## Scope

Primary scope:
- C++ headers and implementation files;
- OpenCV-based image-processing code;
- pipeline, stage, detector, tracker, metric, adapter, and configuration-related
  C++ code;
- CMake, Python, Bash, JSON, YAML, XML, and other config/support files when they
  are touched by a coding task.

For non-C++ files, preserve the local style unless a rule is explicitly defined
here or by a tool configuration already present in the repository.

## Rule Priority

When style sources conflict, apply this order:

1. Explicit user instruction in the current task.
2. Active task card constraints.
3. Canonical cards, stage specs, and protocol/configuration contracts.
4. Local style in the file or module being edited.
5. This document.
6. Existing build, lint, format, or tool configuration.
7. General C++20, OpenCV, C++ Core Guidelines, Microsoft Modern C++, and Google
   C++ style guidance.

Do not use this document as a reason to broaden task scope.

External style guides are references, not automatic project rules. The OpenCV
Coding Style Guide is useful for OpenCV-related C++ practices, but this document
defines the canonical code-style rules for this repository.

## General Change Rules

Code changes must be minimal, local, and reviewable.

Agents must:
- preserve local consistency in touched files;
- avoid mixing styles in one file;
- avoid opportunistic cleanup outside the task scope;
- avoid mass formatting unless formatting is the explicit task;
- avoid changing public APIs, serialized formats, file naming conventions,
  configuration keys, or DP1/DP2 exchange contracts as a side effect;
- keep backward compatibility unless the task explicitly changes behavior;
- put business logic constants in named constants or configuration, not inline
  magic numbers or magic strings.

When a requested change becomes larger than expected, stop and report the reason
before continuing with broad edits.

## Formatting

Use 4 spaces for indentation. Do not use tabs.

Prefer a maximum line length of about 100 characters. Longer lines are acceptable
for URLs, diagnostic strings, or signatures where wrapping would make the code
harder to read.

When editing existing code:
- follow the file's local brace style, spacing, and include grouping;
- format only the changed area;
- do not reflow unrelated code.

Use blank lines to separate logical blocks, not to hide oversized functions.

For new or substantially changed code, prefer braces for `if`, `else`, `for`,
`while`, and `switch` bodies. Single-line bodies without braces are acceptable
only when they match clear local style and do not reduce maintainability.

If indentation reaches 3 to 4 nested logical levels, prefer guard clauses,
helper extraction, or condition simplification.

`clang-format` is not globally mandatory unless a canonical `.clang-format` file
exists in the repository. The canonical configuration is the repository-root
`.clang-format`, or the nearest `.clang-format` above the edited file.

If no `.clang-format` file exists, preserve the local style of the edited file
manually and do not perform broad formatting-only changes. Once a canonical
`.clang-format` exists, run it only on new or modified canonical C++ files unless
the task explicitly requests a wider formatting pass.

## Files And Includes

Use `.hpp` for C++ headers and `.cpp` for implementation files unless the module
already uses another convention.

Headers must be self-contained: a header should not rely on unrelated prior
includes from its includer.

Use `#pragma once` or include guards according to the local module style. Do not
mix both styles inside one module without a concrete reason.

Public headers should contain declarations, small justified inline functions,
type definitions, and contract comments. Implementation details belong in
`.cpp` files or private/internal headers.

Do not put mutable global state in headers.

Do not include heavy OpenCV or project headers in public headers unless required
by the public API. Prefer forward declarations when they are correct and do not
make the API misleading.

Each `.cpp` file should include its own matching header first when applicable.

Preferred include order for new code:

1. Matching project header.
2. C and C++ standard library headers.
3. Third-party library headers, including OpenCV.
4. Project headers.

Do not rely on transitive includes. If a file uses a type or function, include
the header that declares it.

Remove unused includes only in the changed area unless the task is include
cleanup.

## Naming

Names must describe domain meaning, not implementation convenience.

Use clear image-processing and project-domain terms such as `frame`, `tile`,
`mask`, `backgroundModel`, `residual`, `detection`, `track`, `roi`,
`frameIndex`, `cameraId`, `sourcePath`, and `timestamp`.

Avoid vague names such as `data`, `value`, `obj`, `item`, `tmp`, `res`, and
`info` unless the surrounding context makes the meaning obvious.

Classes and structs should use noun or noun-phrase names, such as `FrameReader`,
`TileGrid`, `BackgroundSubtractorConfig`, or `DetectionResult`.

Functions should use verb or verb-phrase names, such as `loadFrame`,
`computeResidual`, `applyMask`, `validateConfig`, or `writeResults`.

Boolean variables and functions should read as predicates, such as `isValid`,
`hasMask`, `shouldUpdateModel`, or `enableDebugOutput`.

Use consistent names for the same concept inside a module. Do not mix
`frameNo`, `frameIndex`, and `idxFrame` for the same value.

Avoid unexplained abbreviations. Domain abbreviations are acceptable when they
are already standard in the project, such as `roi`, `fps`, `dp1`, `dp2`, or
`gt`.

For existing code, do not rename broad symbol sets only to satisfy naming
preferences.

For new canonical C++ classes, private data members must use `snake_case_` with
a trailing underscore.

Examples: `frame_index_`, `background_mask_`, `tile_count_`.

Do not use `m_`, leading underscores, Hungarian notation, or mixed conventions
in new canonical code.

For plain data structs that represent contracts or DTO-like data, public fields
must use `snake_case` without a trailing underscore.

When editing legacy files, preserve the local naming style unless the task
explicitly requests a canonical refactoring.

## C++ Rules

Use RAII for resource management. Resource ownership must be represented by
objects, containers, or smart pointers.

Do not introduce owning raw pointers in new code. Use `std::unique_ptr` for
exclusive ownership and `std::shared_ptr` only when shared ownership is real.

Avoid manual `new` and `delete`. If they are necessary, document why and prefer
an existing project abstraction.

Use standard containers such as `std::vector`, `std::array`, `std::map`,
`std::unordered_map`, and `std::string` unless OpenCV containers or project
types are required.

Use `std::string_view` for read-only string parameters when lifetime is clear
and the value is not stored.

Use `constexpr` or `inline constexpr` constants instead of macro constants.

Use `enum class` for new enumerations.

Use `auto` only when the type is obvious from the right-hand side or when it
improves readability for long iterator or template types. Do not use `auto` when
it hides important numeric, ownership, or OpenCV type information.

Use `const` for variables, parameters, and member functions whenever possible.

Use range-based loops when they make ownership and mutation clear.

Prefer standard-library algorithms when they make the code clearer. Do not
replace simple readable loops with complex algorithm expressions only for style.

## API And Function Design

A function should do one clear thing at one level of abstraction.

Split functions that mix validation, transformation, algorithm execution,
logging, and output writing.

For public, stage-level, or protocol-adjacent functions, document:
- accepted input types;
- expected image depth and channels;
- coordinate system;
- ownership and mutation behavior;
- output format;
- error behavior;
- performance-sensitive assumptions.

Preferred parameter order for new APIs:

1. Required input data.
2. Required configuration.
3. Output parameters.
4. Flags and optional parameters.

Pass small scalar types by value. Pass large objects by `const&` unless ownership
transfer or mutation is intended.

Do not hide output mutation. If a function mutates an argument, the name, type,
or documentation must make that clear.

Do not introduce default parameters in widely used public APIs without checking
call-site ambiguity.

Before implementing a large processing stage, explicitly define and document:
- who owns input frame buffers;
- which stages may mutate buffers in place;
- where copies are allowed and where copies are forbidden;
- which buffers are reused between frames;
- who owns queue nodes and payloads;
- whether each handoff is move, reference, view, or copy.

Implicit ownership models are not allowed for large processing stages.

## Class And Struct Design

Use `struct` for passive data objects with public fields and no strong
invariants.

Use `class` when invariants, ownership, validation, or behavior must be
protected.

Keep data members private unless the type is intentionally a simple data
carrier.

Initialize all members. Prefer in-class member initializers for default values.

Avoid two-phase initialization. Construct objects into a valid state whenever
possible.

Do not add virtual methods unless runtime polymorphism is required. Use
`override` when overriding virtual functions.

Avoid inheritance for code reuse. Prefer composition unless a true interface or
subtype relationship exists.

## OpenCV Rules

Use OpenCV types deliberately. OpenCV provides low-level image primitives; it
does not define project architecture or DP1/DP2 contracts.

OpenCV project style guidance is a reference for OpenCV-related practices only.
Do not treat OpenCV repository rules as automatically mandatory for this
repository unless this document adopts them explicitly.

Use `cv::Mat` for image and matrix data. Remember that `cv::Mat` copy
construction and assignment are shallow. Use `.clone()` only when independent
data ownership is required.

Validate input images when assumptions are not guaranteed by the caller. Typical
checks include:
- `empty()`;
- `size()`;
- `type()`;
- `depth()`;
- `channels()`;
- ROI bounds.

Do not assume a `cv::Mat` is continuous unless `isContinuous()` was checked or
the operation is safe for non-continuous matrices.

Use OpenCV geometry and scalar types where appropriate: `cv::Point`,
`cv::Point2f`, `cv::Size`, `cv::Rect`, `cv::Scalar`, `cv::Range`, and
`cv::RotatedRect`.

Use `cv::InputArray`, `cv::OutputArray`, and `cv::InputOutputArray` only for API
boundaries where flexibility is useful. Do not store these proxy types as class
members or use them as ordinary local containers.

For output images in repeated processing paths, prefer `create()` or reusable
buffers when dimensions and type are known.

Avoid unnecessary `clone()`, `copyTo()`, `convertTo()`, `resize()`, and temporary
`cv::Mat` creation in per-frame or per-tile hot paths.

Be explicit about image depth conversions. When converting from 16-bit to 8-bit,
document scale and offset assumptions.

For binary masks, document expected values such as `0/255`, `0/1`, or OpenCV
foreground-mask conventions.

Validate ROI boundaries before slicing. Do not allow negative coordinates,
zero-size regions, or regions outside the source image unless the behavior is
explicitly handled.

## Performance-Sensitive Code

Performance-sensitive code must remain readable and allocation-aware.

In frame, tile, mask, detection, and track loops, avoid:
- repeated dynamic allocation;
- unnecessary deep copies;
- repeated string formatting;
- repeated construction of heavy OpenCV objects;
- hidden repeated scans behind compact expressions.

Prefer preallocated buffers when lifetime and size are clear.

Ring buffers, work buffers, and queue nodes should be preallocated when feasible.

Do not perform blocking I/O in hot per-frame paths.

Avoid heavy logging in hot per-frame paths. If logging is necessary, keep it
bounded, configurable, and outside the hottest loop where possible.

Avoid virtual dispatch in the hottest loops unless it is justified by the design
and measured or otherwise validated for the local performance budget.

Prefer predictable data flow and controlled memory layout over excessive
abstraction in realtime processing paths.

Each backend transition must be explicit, measurable, and easy to disable.

Do not optimize blindly. If a performance-oriented change makes code less
obvious, add a short comment explaining the reason.

Do not introduce micro-optimizations that make correctness harder to review.

## Error Handling And Logging

Do not ignore errors silently.

Do not write empty `catch` blocks. If an exception is intentionally ignored, add
a short comment explaining why the situation is expected and safe.

Prefer one error-handling style inside a module. Do not mix exceptions, boolean
status codes, integer error codes, and nullable outputs without a clear local
reason.

Do not let exceptions cross hot per-frame paths unless explicitly allowed by
local project conventions.

For invalid programmer assumptions, use assertions or fail-fast behavior
according to local module conventions.

For invalid runtime input, return a clear status, throw a meaningful exception,
or log and stop the current operation according to module conventions.

Log messages must include enough context to debug the problem. For image
pipelines, useful context includes stage name, frame index, camera/source
identifier, file path, image size, image type, and configuration key.

Do not catch broad exceptions only to continue with corrupted or incomplete
state.

## Comments

Comments must explain why the code exists, what assumption it depends on, or
what non-obvious constraint must be preserved.

Do not add comments that merely repeat the code.

Use comments for:
- algorithm assumptions;
- coordinate systems;
- image type assumptions;
- mask value conventions;
- ownership or lifetime rules;
- performance-sensitive choices;
- compatibility constraints;
- links between implementation and canonical cards or stage specs.

Public APIs and stage-level functions should use Doxygen-style comments when
they are part of a project contract.

Doxygen comments are mandatory for canonical contract-level APIs: stage
interfaces, cross-module interfaces, public headers, configuration models,
result contracts, protocol boundary structures, and public functions whose
inputs, outputs, ownership, image format, coordinate system, side effects, or
error behavior are not obvious from the signature.

Doxygen comments are not mandatory for every trivial getter, setter, or
implementation-only helper when the name and type already make the intent clear.

When code implements a task-card or stage-spec requirement, prefer a concise
comment naming the domain concept instead of copying long requirement text into
source code.

Remove obsolete comments. Incorrect comments are worse than missing comments.

All new production source-code comments must be written in English.

If nearby legacy code contains non-English comments, do not add new non-English
comments and do not rewrite unrelated comments only for translation. If an
existing non-English comment is directly edited as part of the task, replace it
with an English version when it is safe and does not broaden the task scope.

Documentation comments, Doxygen blocks, TODO notes, error explanations, and
implementation notes must always be in English.

## Configuration-Related Code

Do not silently add or rename configuration keys.

New behavior must preserve backward compatibility by default unless the task
explicitly changes behavior.

New optional behavior must have an explicit configuration switch, such as
`enabled` or an existing module-local equivalent, and a safe default.

Invalid configuration must not silently fall back to a default that hides the
error. Validate values and report the failing key with enough context.

Keep business thresholds, tuning values, and algorithm variants in named
constants or configuration. Do not bury them as literals inside algorithm code.

## Prohibited Patterns

Do not introduce these patterns in new code:
- owning raw pointers;
- manual `new` and `delete` without a strong reason;
- mutable global state;
- `using namespace std;` in headers;
- `using namespace cv;` in headers;
- broad `using namespace` directives in public headers;
- empty `catch` blocks;
- commented-out code;
- unused functions, variables, includes, or parameters;
- macro constants where `constexpr` is possible;
- hidden output mutation;
- large functions that mix several responsibilities;
- deep nesting where guard clauses or extraction would be clearer;
- silent fallback to default configuration after invalid input;
- unexplained conversions between image depths or channel formats;
- unconditional `clone()` or `copyTo()` in frame loops;
- disabling warnings, tests, static analysis, or validation checks only to make a
  patch appear successful.

## Legacy Code Touches

If a legacy module does not follow this document:
- do not rewrite the whole file automatically;
- work locally inside the approved task scope;
- preserve local style where broad cleanup is not approved;
- avoid copying legacy structure into canonical code unless the active task card
  explicitly allows that source and scope.

When a local style conflict is unavoidable, prefer the smallest change that is
correct and easy to review.
