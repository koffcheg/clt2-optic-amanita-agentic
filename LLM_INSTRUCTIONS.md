# LLM / AI agent usage notes

## Models and agents used

- OpenAI GPT-5.2-Codex through the repository AI-agent interface.

## Tasks solved with AI assistance

- Parsed the Ukrainian fullstack test assignment and converted it into scoped implementation tasks.
- Replaced the previous Python backend direction with a PHP Laravel backend in `currency-dashboard/backend/` after the user requested Laravel.
- Added Laravel API routes, controller, service-layer state handling, seeded/fallback data, JSON-file storage and a scheduler command for refreshes.
- Added a Vue 3 frontend in `currency-dashboard/frontend/` using Vue Router, Pinia and Axios from CDN builds.
- Added repository run instructions for the Laravel/Vue dashboard.
- Added task cards for traceability.

## Prompts used

The primary user prompts were:

1. The full Ukrainian test assignment requesting a backend with PHP Laravel or Python without a framework, a Vue frontend, banking API integrations, automatic updates, bank/currency/rate/branch/statistics functionality, optional account/subscription/notification functionality, AI usage documentation and an 8-hour timebox.
2. The follow-up correction: `go with php laravel`.

Repository governance instructions from `AGENTS.md` were also used as operational prompts: create/read task cards for non-trivial work, avoid changing unrelated Amanita C++ contracts, document validation, and keep changes local.

## Code manually reviewed and improved after generation

- Reworked the backend choice from Python stdlib to Laravel to match the user's preference.
- Kept the backend logic framework-native: Laravel routes, controller, service class, storage facade, HTTP client and scheduler command.
- Added deterministic seeded fallback data so the project remains demonstrable when external APIs are unavailable or blocked.
- Added API error visibility through `/api/health` instead of silently hiding integration failures.
- Kept the frontend build-free to reduce setup friction while still satisfying Vue Router, Pinia and Axios requirements.
- Limited auth/profile/subscription functionality to an explicit MVP and documented that JSON-file storage is not a production database.

## Decisions changed after generation

- Backend implementation was changed from Python without a framework to PHP Laravel.
- External API calls are best-effort refreshes rather than hard startup dependencies because the execution environment returned `Tunnel connection failed: 403 Forbidden` for Packagist/NBU/MinFin/Finance.ua checks.
- A Laravel server now serves both `/api/*` and the Vue static frontend to keep local launch instructions simple.
- Email notifications are represented as user notification preferences and significant-change events; actual SMTP delivery is intentionally left out of the MVP scope.

## Timebox estimate

The requested maximum is 8 working hours. This implementation is scoped as a polished Laravel/Vue MVP that should fit in that timebox for an experienced fullstack developer, with additional production work left for database migrations/models, queues, SMTP, tests, deployment and API-contract hardening.
