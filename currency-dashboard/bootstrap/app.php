<?php

use App\Console\Commands\RefreshCurrencyDashboardData;
use Illuminate\Console\Scheduling\Schedule;
use Illuminate\Foundation\Application;
use Illuminate\Foundation\Configuration\Exceptions;
use Illuminate\Foundation\Configuration\Middleware;

return Application::configure(basePath: dirname(__DIR__))
    ->withRouting(
        web: __DIR__.'/../routes/web.php',
        api: __DIR__.'/../routes/api.php',
        commands: __DIR__.'/../routes/console.php',
        health: '/up',
    )
    ->withCommands([
        RefreshCurrencyDashboardData::class,
    ])
    ->withSchedule(function (Schedule $schedule): void {
        $schedule->command('currency-dashboard:refresh --rates')->everyFifteenMinutes();
        $schedule->command('currency-dashboard:refresh --branches')->everySixHours();
    })
    ->withMiddleware(function (Middleware $middleware): void {
        $middleware->validateCsrfTokens(except: ['api/*']);
    })
    ->withExceptions(function (Exceptions $exceptions): void {
        // Laravel default exception rendering is enough for this MVP.
    })
    ->create();
