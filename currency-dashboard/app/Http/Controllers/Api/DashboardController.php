<?php

namespace App\Http\Controllers\Api;

use App\Services\CurrencyDashboardStore;
use Illuminate\Http\JsonResponse;
use Illuminate\Http\Request;
use Illuminate\Routing\Controller;
use RuntimeException;

class DashboardController extends Controller
{
    public function __construct(private readonly CurrencyDashboardStore $store)
    {
    }

    public function health(): JsonResponse
    {
        return response()->json($this->store->health());
    }

    public function currencies(): JsonResponse
    {
        return response()->json(['items' => $this->store->currencies()]);
    }

    public function banks(): JsonResponse
    {
        return response()->json(['items' => $this->store->banks()]);
    }

    public function bank(string $slug): JsonResponse
    {
        $bank = $this->store->bank($slug);

        if (! $bank) {
            return response()->json(['error' => 'Bank not found'], 404);
        }

        return response()->json($bank);
    }

    public function rates(Request $request): JsonResponse
    {
        return response()->json([
            'items' => $this->store->rates($request->query()),
            'updatedAt' => $this->store->health()['lastRatesRefresh'],
        ]);
    }

    public function summary(Request $request): JsonResponse
    {
        return response()->json([
            'items' => $this->store->summary($request->query()),
            'updatedAt' => $this->store->health()['lastRatesRefresh'],
        ]);
    }

    public function nearestBranches(Request $request): JsonResponse
    {
        return response()->json([
            'items' => $this->store->nearestBranches(
                (float) $request->query('lat', 50.4501),
                (float) $request->query('lng', 30.5234),
                (int) $request->query('limit', 8),
                $request->query('bank')
            ),
        ]);
    }

    public function statistics(Request $request): JsonResponse
    {
        return response()->json(['items' => $this->store->statistics($request->query())]);
    }

    public function significantChanges(Request $request): JsonResponse
    {
        return response()->json(['items' => $this->store->significantChanges($request->query())]);
    }

    public function register(Request $request): JsonResponse
    {
        return $this->guarded(fn (): JsonResponse => response()->json($this->store->register($request->all()), 201));
    }

    public function login(Request $request): JsonResponse
    {
        return $this->guarded(fn (): JsonResponse => response()->json($this->store->login($request->all())));
    }

    public function profile(Request $request): JsonResponse
    {
        $profile = $this->store->profile($request->header('Authorization'));

        if (! $profile) {
            return response()->json(['error' => 'Unauthorized'], 401);
        }

        return response()->json(['user' => $profile]);
    }

    public function updateProfile(Request $request): JsonResponse
    {
        $profile = $this->store->updateProfile($request->header('Authorization'), $request->all());

        if (! $profile) {
            return response()->json(['error' => 'Unauthorized'], 401);
        }

        return response()->json(['user' => $profile]);
    }

    private function guarded(callable $callback): JsonResponse
    {
        try {
            return $callback();
        } catch (RuntimeException $exception) {
            return response()->json(['error' => $exception->getMessage()], 400);
        }
    }
}
