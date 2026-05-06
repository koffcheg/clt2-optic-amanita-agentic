<?php

namespace App\Console\Commands;

use App\Services\CurrencyDashboardStore;
use Illuminate\Console\Command;

class RefreshCurrencyDashboardData extends Command
{
    protected $signature = 'currency-dashboard:refresh {--rates : Refresh NBU and MinFin rates} {--branches : Refresh Finance.ua branches} {--all : Refresh rates and branches}';

    protected $description = 'Refresh external banking data for the currency dashboard.';

    public function handle(CurrencyDashboardStore $store): int
    {
        $refreshRates = (bool) $this->option('rates') || (bool) $this->option('all');
        $refreshBranches = (bool) $this->option('branches') || (bool) $this->option('all');

        if (! $refreshRates && ! $refreshBranches) {
            $refreshRates = true;
        }

        if ($refreshRates) {
            $store->refreshRates();
            $this->info('Rates refreshed.');
        }

        if ($refreshBranches) {
            $store->refreshBranches();
            $this->info('Branches refreshed.');
        }

        return self::SUCCESS;
    }
}
