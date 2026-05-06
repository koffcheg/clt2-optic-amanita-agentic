<?php

use Illuminate\Support\Facades\Route;
use Symfony\Component\HttpFoundation\BinaryFileResponse;

Route::get('/', fn (): BinaryFileResponse => response()->file(resource_path('frontend/index.html')));

Route::get('/assets/{path}', fn (string $path): BinaryFileResponse => response()->file(resource_path('frontend/assets/'.$path)))->where('path', '.*');
Route::get('/src/{path}', fn (string $path): BinaryFileResponse => response()->file(resource_path('frontend/src/'.$path)))->where('path', '.*');

Route::fallback(fn (): BinaryFileResponse => response()->file(resource_path('frontend/index.html')));
