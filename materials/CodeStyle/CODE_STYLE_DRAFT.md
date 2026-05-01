# CODE_STYLE_DRAFT.md

Status: Draft

Target project: C++20, OpenCV 4.9.0

Primary audience: AI coding agents that create, modify, refactor, or review code in this repository.

## 1. Purpose

This document defines the code style and engineering rules that an AI coding agent must follow when working with the project codebase.

The purpose of this document is not only visual consistency. The main goals are readability, maintainability, predictable behavior, controlled complexity, stable performance, and safe integration of generated code into an existing C++ and OpenCV codebase.

AI-generated code must be easy for a human developer to read, review, debug, test, and maintain. The agent must optimize for the future reader of the code, not for producing the shortest or fastest possible patch.

## 2. Scope

These rules apply to all new C++ code and to all modified C++ code created by an AI coding agent.

They apply to:

- headers and implementation files;
- OpenCV-based image processing code;
- pipeline, stage, detector, tracker, metric, and adapter code;
- configuration-related C++ code;
- tests and test utilities;
- small helper scripts only when they directly support the C++ build or test workflow.

For existing code, the agent must preserve local consistency. Do not reformat or rewrite unrelated code only to make it match this document.

## 3. Rule Priority

When rules conflict, apply the following priority order:

1. Explicit task card requirements.
2. Stage specification and canonical project documentation.
3. Existing local style in the file or module being edited.
4. This `CODE_STYLE_DRAFT.md` document.
5. Project build, test, lint, and formatting configuration.
6. OpenCV style and API conventions.
7. General C++20, C++ Core Guidelines, Google C++ Style Guide, and Microsoft Modern C++ guidance.

The agent must not use this document as a reason to perform broad refactoring outside the current task scope.

## 4. AI Agent Operating Rules

Before changing code, the agent must read the relevant task card, project index, stage specification, headers, implementation files, and nearby code.

The agent must keep changes minimal and directly connected to the requested task. Avoid opportunistic refactoring, mass renaming, mass formatting, directory reorganization, or interface redesign unless explicitly requested.

The agent must not modify legacy code or copy legacy design into canonical code unless the task card explicitly allows legacy access. If legacy code is used as a reference, the final response must clearly state what was inspected and why.

The agent must not silently change public APIs, serialized formats, file naming conventions, DP1 to DP2 protocol boundaries, configuration keys, or result contracts. Any such change requires an explicit task-level decision.

The agent must prefer small, reviewable commits. If the requested change becomes larger than expected, stop and report the reason instead of continuing with uncontrolled edits.

At the end of the task, the agent must report:

- files changed;
- interfaces changed, if any;
- build or tests executed;
- checks that could not be executed;
- assumptions made;
- any remaining risks or follow-up tasks.

## 5. Formatting Rules

Use 4 spaces for indentation. Do not use tabs.

Prefer a maximum line length of 100 characters. A longer line is acceptable only when wrapping would reduce readability, for example in a long URL, a generated diagnostic string, or a strongly coupled function signature.

Do not reformat an entire file unless formatting is the explicit task. When editing existing code, follow the local brace style, include ordering, and spacing conventions of that file.

Use blank lines to separate logical blocks. Do not use blank lines to hide overly long functions.

Use braces for all non-trivial `if`, `else`, `for`, `while`, and `switch` blocks. For new code, prefer braces even for single-line bodies when it improves maintainability or prevents future mistakes.

Avoid deeply nested formatting. If indentation exceeds 3 to 4 logical levels, consider extracting a helper function, using guard clauses, or simplifying conditions.

## 6. File Organization

Use `.hpp` for C++ headers and `.cpp` for implementation files unless the existing module uses another convention.

A header must be self-contained. It should compile when included independently after the project precompiled headers or common build setup, if such setup exists.

Use `#pragma once` or include guards according to the local module style. Do not mix styles inside the same module without reason.

Headers must contain declarations, small inline functions when justified, type definitions, and public API documentation. Implementation details belong in `.cpp` files or private/internal headers.

Do not put mutable global state in headers.

Do not include heavy OpenCV or project headers in public headers unless required by the public API. Prefer forward declarations when practical and safe.

Keep platform-specific code isolated behind a small abstraction or a clearly named implementation file.

## 7. Include Rules

Each `.cpp` file should include its own header first when applicable. This helps detect missing dependencies in the header.

Prefer the following include order unless the existing file uses a different consistent order:

1. The matching project header.
2. C and C++ standard library headers.
3. Third-party library headers, including OpenCV.
4. Project headers.

Do not rely on transitive includes. If a file uses a type or function, include the header that declares it.

Remove unused includes when they are part of the changed area. Do not perform large include cleanup outside the task scope.

## 8. Naming Rules

Names must describe domain meaning, not implementation convenience.

Use clear names for image-processing concepts, for example `frame`, `tile`, `mask`, `backgroundModel`, `residual`, `detection`, `track`, `roi`, `frameIndex`, `cameraId`, `sourcePath`, and `timestamp`.

Avoid vague names such as `data`, `value`, `obj`, `item`, `tmp`, `res`, and `info` unless the surrounding context makes the meaning obvious.

Classes and structs should use noun or noun-phrase names, for example `FrameReader`, `TileGrid`, `BackgroundSubtractorConfig`, or `DetectionResult`.

Functions should use verb or verb-phrase names, for example `loadFrame`, `computeResidual`, `applyMask`, `validateConfig`, or `writeResults`.

Boolean variables and functions should read as predicates, for example `isValid`, `hasMask`, `shouldUpdateModel`, or `enableDebugOutput`.

Use consistent naming for related concepts. Do not use `frameNo`, `frameIndex`, and `idxFrame` for the same concept in the same module.

Avoid unexplained abbreviations. Domain abbreviations are acceptable only when they are already standard in the project, for example `roi`, `fps`, `dp1`, `dp2`, or `gt`.

## 9. C++20 Language Rules

Use RAII for resource management. Resource ownership must be represented by objects, containers, or smart pointers.

Do not use owning raw pointers in new code. Use `std::unique_ptr` for exclusive ownership and `std::shared_ptr` only when shared ownership is real and necessary.

Avoid manual `new` and `delete`. If they appear necessary, document why and prefer an existing project abstraction instead.

Use standard containers such as `std::vector`, `std::array`, `std::map`, `std::unordered_map`, and `std::string` unless OpenCV containers or project-specific containers are required.

Use `std::string_view` for read-only string parameters when lifetime is clear and the value is not stored.

Use `constexpr` or `inline constexpr` for constants instead of macros.

Use `enum class` for new enumerations.

Use `auto` only when the type is obvious from the right-hand side or when it improves readability for long iterator or template types. Do not use `auto` when it hides important numeric, ownership, or OpenCV type information.

Use `const` for variables, parameters, and member functions whenever possible.

Use range-based loops when they make ownership and mutation clear.

Prefer algorithms from the standard library when they make the code clearer. Do not replace simple readable loops with complex algorithm expressions only for style.

## 10. Function and API Design

A function should do one clear thing at one level of abstraction.

Keep functions short enough to review comfortably. If a function mixes validation, transformation, algorithm execution, logging, and output writing, split it.

Prefer explicit input and output contracts. For public or stage-level functions, document:

- accepted input types;
- expected image depth and channels;
- coordinate system;
- ownership and mutation behavior;
- output format;
- error behavior;
- performance-sensitive assumptions.

Prefer this parameter order for new APIs:

1. Required input data.
2. Required configuration.
3. Output parameters.
4. Flags and optional parameters.

Pass small scalar types by value. Pass large objects by `const&` unless ownership transfer or mutation is intended.

Do not hide output mutation. If a function mutates an argument, the name, type, or documentation must make that clear.

Do not introduce default parameters in widely used public APIs without checking call-site ambiguity.

## 11. Class and Struct Design

Use `struct` for passive data objects with public fields and no strong invariants.

Use `class` when invariants, ownership, validation, or behavior must be protected.

Keep data members private unless the type is intentionally a simple data carrier.

Initialize all members. Prefer in-class member initializers for default values.

Avoid two-phase initialization. Construct objects into a valid state whenever possible.

Do not add virtual methods unless runtime polymorphism is required.

When overriding virtual functions, use `override`.

Avoid inheritance for code reuse. Prefer composition unless a true interface or subtype relationship exists.

## 12. Error Handling

Do not ignore errors silently.

Do not write empty `catch` blocks. If an exception is intentionally ignored, add a comment that explains why the situation is expected and safe.

Prefer one consistent error-handling style inside a module. Do not mix exceptions, boolean status codes, integer error codes, and nullable outputs without a clear reason.

For invalid programmer assumptions, use assertions or fail-fast behavior according to project policy.

For invalid runtime input, return a clear status, throw a meaningful exception, or log and stop the current operation according to module conventions.

Log messages must include enough context to debug the problem. For image-processing pipelines, include relevant values such as stage name, frame index, camera/source identifier, file path, image size, image type, and configuration key.

Do not catch broad exceptions only to continue execution with corrupted or incomplete state.

## 13. OpenCV-Specific Rules

The project uses OpenCV 4.9.0. New OpenCV code must use OpenCV types and idioms deliberately.

Use `cv::Mat` for image and matrix data. Remember that `cv::Mat` copy construction and assignment are shallow. Use `.clone()` only when independent data ownership is required.

Check input images before processing when the assumptions are not guaranteed by the caller. Typical checks include:

- `empty()`;
- `size()`;
- `type()`;
- `depth()`;
- `channels()`;
- expected ROI bounds.

Do not assume that a `cv::Mat` is continuous unless you have checked `isContinuous()` or the operation is safe for non-continuous matrices.

Use OpenCV scalar and geometry types where appropriate: `cv::Point`, `cv::Point2f`, `cv::Size`, `cv::Rect`, `cv::Scalar`, `cv::Range`, and `cv::RotatedRect`.

Use `cv::InputArray`, `cv::OutputArray`, and `cv::InputOutputArray` only for API boundaries where flexibility is useful. Do not store these proxy types as class members. Do not use them as ordinary local data containers.

Inside a function that accepts `cv::InputArray`, obtain a concrete object such as `cv::Mat` when needed and validate it before use.

For output images in repeated processing paths, prefer `create()` or reusable buffers to reduce allocations.

Avoid unnecessary `clone()`, `copyTo()`, `convertTo()`, `resize()`, and temporary `cv::Mat` creation in per-frame or per-tile hot paths.

Be explicit about image depth conversions. When converting from 16-bit to 8-bit, document scale and offset assumptions.

For binary masks, document expected values, for example `0/255`, `0/1`, or OpenCV foreground mask conventions.

For ROI operations, validate boundaries before slicing. Do not allow negative coordinates, zero-size regions, or regions outside the source image unless the behavior is explicitly handled.

## 14. Performance-Sensitive Code

This project may process frames, tiles, masks, detections, and tracks in loops. Performance-sensitive code must be readable and allocation-aware.

In hot paths, avoid repeated dynamic allocation, unnecessary deep copies, repeated string formatting, and repeated construction of heavy OpenCV objects.

Prefer preallocated buffers where the lifetime and size are clear.

Do not optimize blindly. If a performance-oriented change makes code less obvious, add a short comment explaining the reason.

Do not introduce micro-optimizations that make correctness harder to review.

Avoid hidden algorithmic complexity. A clear `O(n)` pass is usually better than a compact expression that hides repeated scans or allocations.

## 15. Comments and Documentation

Comments must explain why the code exists, what assumption it depends on, or what non-obvious constraint must be preserved.

Do not write comments that merely repeat the code.

Use comments for:

- algorithm assumptions;
- coordinate systems;
- image type assumptions;
- mask value conventions;
- ownership or lifetime rules;
- performance-sensitive choices;
- compatibility constraints;
- links between canonical code and task/stage specifications.

Public APIs and stage-level functions should use Doxygen-style comments when they are part of the project contract.

When the code implements a task card requirement, prefer a concise comment naming the domain concept rather than copying a long requirement into the source file.

Remove obsolete comments. Incorrect comments are worse than missing comments.

## 16. Prohibited and Discouraged Patterns

The AI agent must not introduce the following patterns in new code:

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
- test-disabling changes made only to pass the build.

## 17. Testing Rules

The agent must run the most relevant available build or test command when it can do so safely.

The agent must not claim that full acceptance testing was performed unless the complete acceptance procedure was actually executed.

For new logic, prefer adding or updating tests near the changed module. If full tests are not possible, provide a clear manual verification note.

For OpenCV-related code, test or explicitly consider these edge cases:

- empty frame;
- wrong image type;
- wrong number of channels;
- non-continuous matrix;
- invalid ROI;
- zero-size image or tile;
- first and last frame;
- missing input file;
- unsupported image format;
- invalid configuration value.

Tests should validate observable behavior, not implementation details, unless the implementation detail is itself the contract.

## 18. Tooling Rules

Do not ignore compiler warnings introduced by the change.

Do not disable warnings, tests, static analysis, or sanitizer checks to make a patch appear successful.

Use `clang-format` only if the repository already defines a formatting configuration or the task explicitly requests formatting.

Use `clang-tidy`, sanitizers, and other static or dynamic analysis tools when they are already part of the project workflow or when the task explicitly asks for them.

If a tool reports issues outside the changed area, do not automatically rewrite unrelated code. Report the issue separately unless it blocks the current task.

## 19. Canonical and Legacy Code Boundary

Canonical code is the target implementation path. Legacy code is historical reference material unless a task card explicitly authorizes its use.

The agent must not derive new canonical structure by blindly copying legacy code.

If legacy behavior must be preserved, the agent should express it through clear canonical interfaces, tests, configuration, and documented contracts.

Any direct legacy access must be reported in the final response with:

- what legacy file or module was read;
- why it was needed;
- what behavior or constraint was extracted;
- whether any legacy code was copied or only inspected.

## 20. Final Agent Checklist

Before finishing a coding task, the agent must check:

- The change is limited to the task scope.
- No unrelated file was reformatted.
- No legacy code was modified or copied without explicit authorization.
- Public API changes are documented and justified.
- `cv::Mat` ownership, type, size, and channel assumptions are clear.
- No unnecessary `clone()` or allocation was added to a hot path.
- No empty `catch` block was introduced.
- No unused code or commented-out code remains.
- New names are meaningful in the project domain.
- Build or relevant tests were run when possible.
- Any unrun checks are clearly reported.

## 21. Source Materials Used for This Draft

This draft was prepared from the provided materials and adapted to a C++20 and OpenCV 4.9.0 project context.

Provided research materials:

- `2501.11264v3.pdf` - code readability in the age of large language models and human-in-the-loop coding agents.
- `2601.09832v1.pdf` - adoption and evolution of code style and best programming practices in open-source projects.

Provided reference links:

- Microsoft Modern C++ overview: https://learn.microsoft.com/en-us/cpp/cpp/welcome-back-to-cpp-modern-cpp?view=msvc-170
- Google C++ Style Guide: https://google.github.io/styleguide/cppguide.html
- C++ Core Guidelines: https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines
- OpenCV project wiki: https://github.com/opencv/opencv/wiki

