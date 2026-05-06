<?php

return [
    'name' => env('APP_NAME', 'Currency Dashboard'),
    'env' => env('APP_ENV', 'production'),
    'debug' => (bool) env('APP_DEBUG', false),
    'url' => env('APP_URL', 'http://localhost'),
    'timezone' => 'UTC',
    'locale' => env('APP_LOCALE', 'uk'),
    'fallback_locale' => 'en',
    'faker_locale' => 'uk_UA',
    'cipher' => 'AES-256-CBC',
    'key' => env('APP_KEY'),
    'previous_keys' => [],
];
