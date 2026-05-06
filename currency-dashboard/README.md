# Currency Dashboard

Standalone downloadable Laravel + Vue 3 MVP for the banking currency dashboard test assignment.

Everything needed for the assignment lives in this single folder:

```text
currency-dashboard/
  app/                 Laravel application code
  bootstrap/           Laravel bootstrap
  config/              Laravel config
  public/              Laravel front controller
  resources/frontend/  Vue 3 UI loaded by Laravel web routes
  routes/              API and web routes
  storage/             JSON fallback state and Laravel runtime files
  composer.json        PHP/Laravel dependencies and scripts
  LLM_INSTRUCTIONS.md  AI usage disclosure
```

## Implemented scope

- PHP Laravel backend.
- Vue 3 frontend with Vue Router, Pinia and Axios loaded from CDN.
- Five seeded banks with metadata, supported currencies (`usd`, `eur`, `gbp`, `chf`, `pln`) and seeded branches.
- Best-effort integrations with:
  - NBU exchange rates: `https://bank.gov.ua/NBUStatService/v1/statdirectory/exchange?json`
  - MinFin bank rates: `https://minfin.com.ua/api/currency/rates/banks/{currency_code}`
  - Finance.ua branches: `https://finance.ua/api/organization/v1/branches?slug={bank_slug}&locale=uk`
- Laravel scheduler command for regular rates and branches refreshes.
- JSON-file fallback storage in `storage/app/currency-dashboard/state.json` when external APIs are unavailable.
- APIs for banks, bank details, currencies, rates, NBU/average summary, nearest branches, statistics, significant changes and MVP profile/subscriptions.

## Run locally

From inside this folder:

```bash
cp .env.example .env
composer install
php artisan key:generate
php artisan serve --host=127.0.0.1 --port=8000
```

Open:

```text
http://127.0.0.1:8000
```

The Laravel app serves the Vue files from `resources/frontend/`, so a separate frontend build step is not required for this MVP.

## Refresh external data

Run one refresh manually:

```bash
php artisan currency-dashboard:refresh --all
```

Run Laravel's scheduler worker during local development:

```bash
php artisan schedule:work
```

## API quick checks

```bash
curl http://127.0.0.1:8000/api/health
curl http://127.0.0.1:8000/api/banks
curl 'http://127.0.0.1:8000/api/rates?currencies=usd,eur'
curl 'http://127.0.0.1:8000/api/rates/summary?currencies=usd,eur'
curl 'http://127.0.0.1:8000/api/branches/nearest?lat=50.4501&lng=30.5234&limit=5'
curl 'http://127.0.0.1:8000/api/statistics?currency=usd'
```

## Notes and limitations

- The MVP stores dashboard state, users, subscriptions, rates history and significant-change events in a JSON file rather than a production database.
- Real email delivery is not wired in; the MVP exposes notification preferences and significant-change events that an SMTP/queue worker could consume.
- External banking APIs are not required for startup. If they fail, `/api/health` reports the most recent integration errors and the UI continues to work with fallback data.
- `vendor/` is not committed. Run `composer install` after downloading this folder.
- AI-agent usage is documented in `LLM_INSTRUCTIONS.md`.
