<?php

namespace App\Services;

use Illuminate\Support\Arr;
use Illuminate\Support\Facades\Http;
use Illuminate\Support\Carbon;
use Illuminate\Support\Facades\Storage;
use Illuminate\Support\Str;
use RuntimeException;
use Throwable;

class CurrencyDashboardStore
{
    private const STATE_PATH = 'currency-dashboard/state.json';

    private const SUPPORTED_CURRENCIES = ['usd', 'eur', 'gbp', 'chf', 'pln'];

    private const BASE_RATES = [
        'usd' => ['buy' => 39.95, 'sell' => 40.55],
        'eur' => ['buy' => 43.20, 'sell' => 43.95],
        'gbp' => ['buy' => 50.10, 'sell' => 51.20],
        'chf' => ['buy' => 44.30, 'sell' => 45.30],
        'pln' => ['buy' => 10.00, 'sell' => 10.45],
    ];

    private const BANKS = [
        [
            'slug' => 'privatbank',
            'name' => 'ПриватБанк',
            'description' => 'Найбільший універсальний банк України з широкою цифровою екосистемою.',
            'logo' => 'https://upload.wikimedia.org/wikipedia/commons/9/9b/PrivatBank_Logo.png',
            'site' => 'https://privatbank.ua',
            'phone' => '3700',
            'email' => 'help@privatbank.ua',
            'legalAddress' => '01001, Україна, м. Київ, вул. Грушевського, 1Д',
            'rating' => 4.8,
        ],
        [
            'slug' => 'oschadbank',
            'name' => 'Ощадбанк',
            'description' => 'Державний банк із великою мережею відділень для роздрібних і бізнес-клієнтів.',
            'logo' => 'https://upload.wikimedia.org/wikipedia/commons/8/8a/Oschadbank_logo.svg',
            'site' => 'https://www.oschadbank.ua',
            'phone' => '0 800 210 800',
            'email' => 'contact-centre@oschadbank.ua',
            'legalAddress' => '01001, Україна, м. Київ, вул. Госпітальна, 12Г',
            'rating' => 4.4,
        ],
        [
            'slug' => 'raiffeisen-bank',
            'name' => 'Райффайзен Банк',
            'description' => 'Комерційний банк із фокусом на щоденний банкінг, SME та корпоративний сегмент.',
            'logo' => 'https://upload.wikimedia.org/wikipedia/commons/f/f4/Raiffeisen_Bank_International_logo.svg',
            'site' => 'https://raiffeisen.ua',
            'phone' => '0 800 500 500',
            'email' => 'info@raiffeisen.ua',
            'legalAddress' => '01011, Україна, м. Київ, вул. Генерала Алмазова, 4А',
            'rating' => 4.6,
        ],
        [
            'slug' => 'ukrsibbank',
            'name' => 'UKRSIBBANK',
            'description' => 'Банк групи BNP Paribas для роздрібного банкінгу, бізнесу та premium-сервісів.',
            'logo' => 'https://upload.wikimedia.org/wikipedia/commons/4/43/Ukrsibbank_logo.svg',
            'site' => 'https://ukrsibbank.com',
            'phone' => '729',
            'email' => 'info@ukrsibbank.com',
            'legalAddress' => '04070, Україна, м. Київ, вул. Андріївська, 2/12',
            'rating' => 4.5,
        ],
        [
            'slug' => 'monobank',
            'name' => 'monobank',
            'description' => 'Mobile-first банківський продукт із повністю цифровою взаємодією з клієнтом.',
            'logo' => 'https://www.monobank.ua/resources/1.0.29/img/mono-logo.svg',
            'site' => 'https://www.monobank.ua',
            'phone' => '0 800 205 205',
            'email' => 'support@monobank.ua',
            'legalAddress' => '04073, Україна, м. Київ, просп. Степана Бандери, 16-В',
            'rating' => 4.7,
        ],
    ];

    private const BRANCHES = [
        ['bankSlug' => 'privatbank', 'name' => 'ПриватБанк, Хрещатик', 'address' => 'м. Київ, вул. Хрещатик, 22', 'lat' => 50.4470, 'lng' => 30.5229, 'phone' => '3700'],
        ['bankSlug' => 'privatbank', 'name' => 'ПриватБанк, Львів центр', 'address' => 'м. Львів, просп. Свободи, 15', 'lat' => 49.8421, 'lng' => 24.0319, 'phone' => '3700'],
        ['bankSlug' => 'oschadbank', 'name' => 'Ощадбанк, Майдан', 'address' => 'м. Київ, вул. Мала Житомирська, 5', 'lat' => 50.4520, 'lng' => 30.5210, 'phone' => '0 800 210 800'],
        ['bankSlug' => 'oschadbank', 'name' => 'Ощадбанк, Одеса', 'address' => 'м. Одеса, вул. Дерибасівська, 12', 'lat' => 46.4846, 'lng' => 30.7356, 'phone' => '0 800 210 800'],
        ['bankSlug' => 'raiffeisen-bank', 'name' => 'Райффайзен, Печерськ', 'address' => 'м. Київ, вул. Генерала Алмазова, 4А', 'lat' => 50.4305, 'lng' => 30.5418, 'phone' => '0 800 500 500'],
        ['bankSlug' => 'raiffeisen-bank', 'name' => 'Райффайзен, Дніпро', 'address' => 'м. Дніпро, просп. Дмитра Яворницького, 67', 'lat' => 48.4669, 'lng' => 35.0462, 'phone' => '0 800 500 500'],
        ['bankSlug' => 'ukrsibbank', 'name' => 'UKRSIBBANK, Поділ', 'address' => 'м. Київ, вул. Андріївська, 2/12', 'lat' => 50.4646, 'lng' => 30.5213, 'phone' => '729'],
        ['bankSlug' => 'ukrsibbank', 'name' => 'UKRSIBBANK, Харків', 'address' => 'м. Харків, вул. Сумська, 10', 'lat' => 49.9935, 'lng' => 36.2311, 'phone' => '729'],
        ['bankSlug' => 'monobank', 'name' => 'monobank support hub', 'address' => 'м. Київ, просп. Степана Бандери, 16-В', 'lat' => 50.4890, 'lng' => 30.4870, 'phone' => '0 800 205 205'],
        ['bankSlug' => 'monobank', 'name' => 'monobank virtual branch', 'address' => 'онлайн-відділення', 'lat' => 50.4501, 'lng' => 30.5234, 'phone' => '0 800 205 205'],
    ];

    public function health(): array
    {
        $state = $this->state();

        return [
            'status' => 'ok',
            'time' => now()->toIso8601String(),
            'lastRatesRefresh' => $state['lastRatesRefresh'],
            'lastBranchesRefresh' => $state['lastBranchesRefresh'],
            'externalErrors' => $state['externalErrors'],
        ];
    }

    public function currencies(): array
    {
        return array_map(fn (string $code): array => ['code' => $code, 'label' => strtoupper($code)], self::SUPPORTED_CURRENCIES);
    }

    public function banks(): array
    {
        return $this->state()['banks'];
    }

    public function bank(string $slug): ?array
    {
        $state = $this->state();
        $bank = collect($state['banks'])->firstWhere('slug', $slug);

        if (! $bank) {
            return null;
        }

        return $bank + [
            'rates' => array_values(array_filter($state['bankRates'], fn (array $rate): bool => $rate['bankSlug'] === $slug)),
            'branches' => array_values(array_filter($state['branches'], fn (array $branch): bool => $branch['bankSlug'] === $slug)),
        ];
    }

    public function rates(array $filters): array
    {
        $bankSlugs = $this->csvFilter($filters, 'banks');
        $currencies = $this->csvFilter($filters, 'currencies');

        return array_values(array_filter($this->state()['bankRates'], function (array $rate) use ($bankSlugs, $currencies): bool {
            $bankAllowed = $bankSlugs === [] || in_array($rate['bankSlug'], $bankSlugs, true);
            $currencyAllowed = $currencies === [] || in_array($rate['currency'], $currencies, true);

            return $bankAllowed && $currencyAllowed;
        }));
    }

    public function summary(array $filters): array
    {
        $state = $this->state();
        $rows = $this->rates($filters);
        $requested = $this->csvFilter($filters, 'currencies');
        $currencies = $requested ?: self::SUPPORTED_CURRENCIES;

        return array_map(function (string $currency) use ($rows, $state): array {
            $currencyRows = array_values(array_filter($rows, fn (array $rate): bool => $rate['currency'] === $currency));
            $averageBuy = $this->average($currencyRows, 'buy');
            $averageSell = $this->average($currencyRows, 'sell');

            return [
                'currency' => $currency,
                'averageBuy' => $averageBuy === null ? null : round($averageBuy, 4),
                'averageSell' => $averageSell === null ? null : round($averageSell, 4),
                'nbu' => $state['nbuRates'][$currency] ?? null,
            ];
        }, $currencies);
    }

    public function nearestBranches(float $lat, float $lng, int $limit = 8, ?string $bankSlug = null): array
    {
        $branches = array_filter($this->state()['branches'], fn (array $branch): bool => $bankSlug === null || $bankSlug === '' || $branch['bankSlug'] === $bankSlug);
        $withDistance = array_map(function (array $branch) use ($lat, $lng): array {
            $branch['distanceKm'] = round($this->haversineKm($lat, $lng, (float) $branch['lat'], (float) $branch['lng']), 2);

            return $branch;
        }, $branches);

        usort($withDistance, fn (array $left, array $right): int => $left['distanceKm'] <=> $right['distanceKm']);

        return array_slice($withDistance, 0, min($limit, 50));
    }

    public function statistics(array $filters): array
    {
        $from = $this->parseDate(Arr::get($filters, 'from'), now()->subDays(7));
        $to = $this->parseDate(Arr::get($filters, 'to'), now()->addDay());
        $bank = Arr::get($filters, 'bank');
        $currency = strtolower((string) Arr::get($filters, 'currency', ''));

        $rows = array_filter($this->state()['history'], function (array $row) use ($from, $to, $bank, $currency): bool {
            $timestamp = $this->parseDate($row['timestamp'], now());

            if ($timestamp->lt($from) || $timestamp->gt($to)) {
                return false;
            }

            if ($bank && $row['bankSlug'] !== $bank) {
                return false;
            }

            return $currency === '' || $row['currency'] === $currency;
        });

        return array_slice(array_values($rows), -500);
    }

    public function significantChanges(array $filters): array
    {
        $from = $this->parseDate(Arr::get($filters, 'from'), now()->subDays(30));
        $to = $this->parseDate(Arr::get($filters, 'to'), now()->addDay());

        return array_values(array_filter($this->state()['events'], function (array $event) use ($from, $to): bool {
            $timestamp = $this->parseDate($event['timestamp'], now());

            return $timestamp->between($from, $to);
        }));
    }

    public function refreshRates(): array
    {
        $state = $this->state();
        $errors = [];
        $now = now()->toIso8601String();
        $nbuRates = $this->fetchNbuRates($now, $errors);
        $bankRates = $this->fetchMinfinRates($now, $errors, $state['banks']);

        if ($nbuRates !== []) {
            $state['nbuRates'] = $nbuRates;
        }

        if ($bankRates !== []) {
            $state['bankRates'] = $bankRates;
        }

        $state['lastRatesRefresh'] = $now;
        $state['externalErrors'] = array_slice($errors, -5);
        $state = $this->captureHistory($state, $now);
        $this->saveState($state);

        return $state;
    }

    public function refreshBranches(): array
    {
        $state = $this->state();
        $errors = [];
        $branches = [];

        foreach ($state['banks'] as $bank) {
            try {
                $response = Http::timeout(8)->acceptJson()->get('https://finance.ua/api/organization/v1/branches', [
                    'slug' => $bank['slug'],
                    'locale' => 'uk',
                ]);
            } catch (Throwable $exception) {
                $errors[] = "Finance.ua branches refresh failed for {$bank['slug']}: {$exception->getMessage()}";
                continue;
            }

            if (! $response->successful()) {
                $errors[] = "Finance.ua branches refresh failed for {$bank['slug']}: HTTP {$response->status()}";
                continue;
            }

            $items = $response->json('data', $response->json('items', $response->json())) ?: [];
            foreach (array_slice($items, 0, 20) as $item) {
                $lat = $item['lat'] ?? $item['latitude'] ?? null;
                $lng = $item['lng'] ?? $item['lon'] ?? $item['longitude'] ?? null;

                if ($lat === null || $lng === null) {
                    continue;
                }

                $branches[] = [
                    'id' => (string) ($item['id'] ?? $bank['slug'].'-'.(count($branches) + 1)),
                    'bankSlug' => $bank['slug'],
                    'name' => $item['name'] ?? $item['title'] ?? $bank['name'],
                    'address' => $item['address'] ?? $item['fullAddress'] ?? 'Адресу не вказано',
                    'lat' => (float) $lat,
                    'lng' => (float) $lng,
                    'phone' => $item['phone'] ?? $bank['phone'],
                ];
            }
        }

        if ($branches !== []) {
            $state['branches'] = $branches;
        }

        $state['lastBranchesRefresh'] = now()->toIso8601String();
        $state['externalErrors'] = array_slice(array_merge($state['externalErrors'], $errors), -5);
        $this->saveState($state);

        return $state;
    }

    public function register(array $payload): array
    {
        $state = $this->state();
        $email = strtolower(trim((string) ($payload['email'] ?? '')));
        $password = (string) ($payload['password'] ?? '');

        if ($email === '' || strlen($password) < 6) {
            throw new RuntimeException('Email and password with at least 6 characters are required.');
        }

        if (isset($state['users'][$email])) {
            throw new RuntimeException('User already exists.');
        }

        $user = [
            'email' => $email,
            'name' => $payload['name'] ?? Str::before($email, '@'),
            'passwordHash' => password_hash($password, PASSWORD_DEFAULT),
            'notificationsEnabled' => true,
            'subscriptions' => ['banks' => [], 'currencies' => self::SUPPORTED_CURRENCIES],
        ];
        $token = Str::random(48);
        $state['users'][$email] = $user;
        $state['tokens'][$token] = $email;
        $this->saveState($state);

        return ['token' => $token, 'user' => $this->publicUser($user)];
    }

    public function login(array $payload): array
    {
        $state = $this->state();
        $email = strtolower(trim((string) ($payload['email'] ?? '')));
        $password = (string) ($payload['password'] ?? '');
        $user = $state['users'][$email] ?? null;

        if (! $user || ! password_verify($password, $user['passwordHash'])) {
            throw new RuntimeException('Invalid credentials.');
        }

        $token = Str::random(48);
        $state['tokens'][$token] = $email;
        $this->saveState($state);

        return ['token' => $token, 'user' => $this->publicUser($user)];
    }

    public function profile(?string $bearerToken): ?array
    {
        $state = $this->state();
        $token = Str::replaceFirst('Bearer ', '', (string) $bearerToken);
        $email = $state['tokens'][$token] ?? null;

        return $email ? $this->publicUser($state['users'][$email]) : null;
    }

    public function updateProfile(?string $bearerToken, array $payload): ?array
    {
        $state = $this->state();
        $token = Str::replaceFirst('Bearer ', '', (string) $bearerToken);
        $email = $state['tokens'][$token] ?? null;

        if (! $email || ! isset($state['users'][$email])) {
            return null;
        }

        $state['users'][$email]['name'] = $payload['name'] ?? $state['users'][$email]['name'];
        $state['users'][$email]['notificationsEnabled'] = (bool) ($payload['notificationsEnabled'] ?? $state['users'][$email]['notificationsEnabled']);

        if (isset($payload['subscriptions']) && is_array($payload['subscriptions'])) {
            $state['users'][$email]['subscriptions'] = $payload['subscriptions'];
        }

        $this->saveState($state);

        return $this->publicUser($state['users'][$email]);
    }

    private function state(): array
    {
        if (! Storage::exists(self::STATE_PATH)) {
            $state = $this->seedState();
            $this->saveState($state);

            return $state;
        }

        return json_decode(Storage::get(self::STATE_PATH), true, flags: JSON_THROW_ON_ERROR);
    }

    private function seedState(): array
    {
        $now = now()->toIso8601String();
        $bankRates = [];

        foreach (self::BANKS as $bankIndex => $bank) {
            $spreadAdjustment = $bankIndex * 0.05;
            foreach (self::BASE_RATES as $currency => $baseRate) {
                $bankRates[] = [
                    'bankSlug' => $bank['slug'],
                    'bankName' => $bank['name'],
                    'currency' => $currency,
                    'buy' => round($baseRate['buy'] + $spreadAdjustment, 4),
                    'sell' => round($baseRate['sell'] + $spreadAdjustment, 4),
                    'source' => 'seeded-fallback',
                    'updatedAt' => $now,
                ];
            }
        }

        $branches = array_map(fn (array $branch, int $index): array => $branch + ['id' => 'br-'.($index + 1)], self::BRANCHES, array_keys(self::BRANCHES));
        $state = [
            'banks' => self::BANKS,
            'branches' => $branches,
            'nbuRates' => [],
            'bankRates' => $bankRates,
            'history' => [],
            'events' => [],
            'users' => [],
            'tokens' => [],
            'lastRatesRefresh' => $now,
            'lastBranchesRefresh' => $now,
            'externalErrors' => [],
        ];

        foreach (self::BASE_RATES as $currency => $baseRate) {
            $state['nbuRates'][$currency] = [
                'currency' => $currency,
                'rate' => round(($baseRate['buy'] + $baseRate['sell']) / 2, 4),
                'source' => 'seeded-fallback',
                'updatedAt' => $now,
            ];
        }

        return $this->seedHistory($state);
    }

    private function seedHistory(array $state): array
    {
        $baseDate = now()->subDays(13)->setTime(12, 0);

        for ($day = 0; $day < 14; $day++) {
            $seasonal = sin($day / 2.7) * 0.32;
            $timestamp = $baseDate->copy()->addDays($day)->toIso8601String();

            foreach ($state['bankRates'] as $rate) {
                $multiplier = 1 + ($seasonal / 100);
                $state['history'][] = $rate + [
                    'timestamp' => $timestamp,
                    'buy' => round($rate['buy'] * $multiplier, 4),
                    'sell' => round($rate['sell'] * $multiplier, 4),
                    'source' => 'seeded-history',
                ];
            }
        }

        $last = end($state['history']);
        if ($last) {
            $state['events'][] = $last + [
                'id' => 'event-seeded-1',
                'changePercent' => 5.4,
                'direction' => 'up',
                'message' => 'Демонстраційна істотна зміна курсу понад 5%.',
            ];
        }

        return $state;
    }

    private function saveState(array $state): void
    {
        Storage::put(self::STATE_PATH, json_encode($state, JSON_UNESCAPED_UNICODE | JSON_PRETTY_PRINT | JSON_THROW_ON_ERROR));
    }

    private function fetchNbuRates(string $now, array &$errors): array
    {
        try {
            $response = Http::timeout(8)->acceptJson()->get('https://bank.gov.ua/NBUStatService/v1/statdirectory/exchange?json');
        } catch (Throwable $exception) {
            $errors[] = 'NBU refresh failed: '.$exception->getMessage();
            return [];
        }

        if (! $response->successful()) {
            $errors[] = 'NBU refresh failed: HTTP '.$response->status();
            return [];
        }

        $rates = [];
        foreach ($response->json() ?? [] as $item) {
            $code = strtolower((string) ($item['cc'] ?? ''));
            if (in_array($code, self::SUPPORTED_CURRENCIES, true)) {
                $rates[$code] = ['currency' => $code, 'rate' => (float) $item['rate'], 'source' => 'nbu', 'updatedAt' => $now];
            }
        }

        return $rates;
    }

    private function fetchMinfinRates(string $now, array &$errors, array $banks): array
    {
        $rows = [];

        foreach (self::SUPPORTED_CURRENCIES as $currency) {
            try {
                $response = Http::timeout(8)->acceptJson()->get("https://minfin.com.ua/api/currency/rates/banks/{$currency}");
            } catch (Throwable $exception) {
                $errors[] = "MinFin {$currency} refresh failed: {$exception->getMessage()}";
                continue;
            }

            if (! $response->successful()) {
                $errors[] = "MinFin {$currency} refresh failed: HTTP {$response->status()}";
                continue;
            }

            $items = $response->json('data', $response->json()) ?: [];
            foreach ($items as $item) {
                $bankName = strtolower(trim((string) ($item['bankName'] ?? $item['bank'] ?? $item['title'] ?? '')));
                $matched = collect($banks)->first(fn (array $bank): bool => $bankName !== '' && (str_contains(strtolower($bank['name']), $bankName) || str_contains($bankName, strtolower($bank['name']))));

                if (! $matched) {
                    continue;
                }

                $buy = $item['bid'] ?? $item['buy'] ?? $item['ask'] ?? null;
                $sell = $item['ask'] ?? $item['sell'] ?? $item['bid'] ?? null;
                if ($buy === null || $sell === null) {
                    continue;
                }

                $rows[] = [
                    'bankSlug' => $matched['slug'],
                    'bankName' => $matched['name'],
                    'currency' => $currency,
                    'buy' => (float) $buy,
                    'sell' => (float) $sell,
                    'source' => 'minfin',
                    'updatedAt' => $now,
                ];
            }
        }

        return $rows;
    }

    private function captureHistory(array $state, string $timestamp): array
    {
        $previous = collect(array_slice($state['history'], -count($state['bankRates'])))->keyBy(fn (array $row): string => $row['bankSlug'].'|'.$row['currency']);

        foreach ($state['bankRates'] as $rate) {
            $snapshot = $rate + ['timestamp' => $timestamp];
            $state['history'][] = $snapshot;
            $previousRate = $previous->get($rate['bankSlug'].'|'.$rate['currency']);

            if (! $previousRate) {
                continue;
            }

            $previousMid = ($previousRate['buy'] + $previousRate['sell']) / 2;
            $currentMid = ($rate['buy'] + $rate['sell']) / 2;
            $change = $previousMid == 0.0 ? 0.0 : (($currentMid - $previousMid) / $previousMid) * 100;

            if (abs($change) >= 5.0) {
                $state['events'][] = $snapshot + [
                    'id' => 'event-'.(count($state['events']) + 1),
                    'changePercent' => round($change, 2),
                    'direction' => $change > 0 ? 'up' : 'down',
                    'message' => 'Істотна зміна курсу понад 5%.',
                ];
            }
        }

        $state['history'] = array_slice($state['history'], -3000);
        $state['events'] = array_slice($state['events'], -200);

        return $state;
    }

    private function csvFilter(array $filters, string $key): array
    {
        $value = Arr::get($filters, $key, []);
        $values = is_array($value) ? $value : [$value];

        return array_values(array_filter(array_map('strtolower', array_map('trim', explode(',', implode(',', $values))))));
    }

    private function average(array $rows, string $field): ?float
    {
        if ($rows === []) {
            return null;
        }

        return array_sum(array_column($rows, $field)) / count($rows);
    }

    private function haversineKm(float $lat1, float $lon1, float $lat2, float $lon2): float
    {
        $radius = 6371.0;
        $phi1 = deg2rad($lat1);
        $phi2 = deg2rad($lat2);
        $dPhi = deg2rad($lat2 - $lat1);
        $dLambda = deg2rad($lon2 - $lon1);
        $a = sin($dPhi / 2) ** 2 + cos($phi1) * cos($phi2) * sin($dLambda / 2) ** 2;

        return $radius * 2 * atan2(sqrt($a), sqrt(1 - $a));
    }

    private function parseDate(mixed $value, mixed $default): Carbon
    {
        if ($value === null || $value === '') {
            return $default instanceof Carbon ? $default : Carbon::parse($default);
        }

        return Carbon::parse($value);
    }

    private function publicUser(array $user): array
    {
        unset($user['passwordHash']);

        return $user;
    }
}
