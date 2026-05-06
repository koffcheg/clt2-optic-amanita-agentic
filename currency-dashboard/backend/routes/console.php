<?php

use Illuminate\Support\Facades\Artisan;

Artisan::command('about:currency-dashboard', function (): void {
    $this->info('Currency Dashboard Laravel backend is installed.');
})->purpose('Display Currency Dashboard backend information');
