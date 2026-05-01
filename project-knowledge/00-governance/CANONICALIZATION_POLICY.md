# CANONICALIZATION_POLICY

## Purpose
Define a minimal, explicit rule set for moving validated knowledge from draft/task-card content into canonical domain cards.

## Scope
- Applies to content created in task cards, ad-hoc drafts, and execution notes.
- Applies only to domain knowledge (`project.*`, `dp1.*`, `dp2.*`, `protocols.*`, `validation.*`).
- Does not replace `AGENTS.md`, `CODE_STYLE.md`, or `TESTING_POLICY.md`.

## Source-of-truth rule
- Code and contract-relevant source files remain authoritative for technical facts.
- Task cards are execution artifacts, not canonical domain knowledge.

## Ready-to-canonicalize criteria
Content may be moved into canonical domain cards only when all are true:
1. Fact is confirmed by code/config or explicitly approved requirement.
2. Fact is stable for current scope (not a temporary experiment note).
3. Target domain card is known (existing card or explicitly approved new card).
4. Contradictions with existing cards are resolved or explicitly documented as open questions.

## Minimal workflow
1. Identify candidate facts in draft/task-card content.
2. Verify each fact against authoritative sources in repo.
3. Propose exact destination card(s) and minimal diff.
4. Get explicit approval when required by `AGENTS.md`.
5. Apply update to canonical card(s); keep task card concise and execution-focused.

## Non-goals
- No bulk migration.
- No automatic restructuring of knowledge base.
- No framework/process expansion beyond this short policy.
