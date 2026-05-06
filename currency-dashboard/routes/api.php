<?php

use App\Http\Controllers\Api\DashboardController;
use Illuminate\Support\Facades\Route;

Route::get('/health', [DashboardController::class, 'health']);
Route::get('/currencies', [DashboardController::class, 'currencies']);
Route::get('/banks', [DashboardController::class, 'banks']);
Route::get('/banks/{slug}', [DashboardController::class, 'bank']);
Route::get('/rates', [DashboardController::class, 'rates']);
Route::get('/rates/summary', [DashboardController::class, 'summary']);
Route::get('/branches/nearest', [DashboardController::class, 'nearestBranches']);
Route::get('/statistics', [DashboardController::class, 'statistics']);
Route::get('/significant-changes', [DashboardController::class, 'significantChanges']);
Route::post('/auth/register', [DashboardController::class, 'register']);
Route::post('/auth/login', [DashboardController::class, 'login']);
Route::get('/profile', [DashboardController::class, 'profile']);
Route::put('/profile', [DashboardController::class, 'updateProfile']);
