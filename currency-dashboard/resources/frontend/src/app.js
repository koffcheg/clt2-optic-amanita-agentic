const { createApp, computed, onMounted, ref } = Vue;
const { createRouter, createWebHistory } = VueRouter;
const { createPinia, defineStore } = Pinia;

const api = axios.create({ baseURL: window.API_BASE_URL || '/api', timeout: 12000 });

const useDashboardStore = defineStore('dashboard', {
  state: () => ({
    banks: [],
    currencies: [],
    rates: [],
    summary: [],
    branches: [],
    statistics: [],
    events: [],
    health: null,
    filters: { banks: [], currencies: ['usd', 'eur'] },
    loading: false,
    error: '',
  }),
  actions: {
    async bootstrap() {
      this.loading = true;
      this.error = '';
      try {
        const [health, banks, currencies] = await Promise.all([
          api.get('/health'),
          api.get('/banks'),
          api.get('/currencies'),
        ]);
        this.health = health.data;
        this.banks = banks.data.items;
        this.currencies = currencies.data.items;
        await this.refreshRates();
      } catch (error) {
        this.error = error.message || 'Не вдалося завантажити дані';
      } finally {
        this.loading = false;
      }
    },
    queryParams() {
      const params = {};
      if (this.filters.banks.length) params.banks = this.filters.banks.join(',');
      if (this.filters.currencies.length) params.currencies = this.filters.currencies.join(',');
      return params;
    },
    async refreshRates() {
      const [rates, summary] = await Promise.all([
        api.get('/rates', { params: this.queryParams() }),
        api.get('/rates/summary', { params: this.queryParams() }),
      ]);
      this.rates = rates.data.items;
      this.summary = summary.data.items;
    },
    async loadNearest(position) {
      const response = await api.get('/branches/nearest', {
        params: { lat: position.lat, lng: position.lng, limit: 8 },
      });
      this.branches = response.data.items;
    },
    async loadStatistics(params = {}) {
      const response = await api.get('/statistics', { params });
      this.statistics = response.data.items;
      const events = await api.get('/significant-changes', { params });
      this.events = events.data.items;
    },
  },
});

const Shell = {
  setup() {
    const store = useDashboardStore();
    onMounted(() => store.bootstrap());
    return { store };
  },
  template: `
    <header class="hero">
      <nav class="nav">
        <router-link to="/" class="brand">Bank FX Radar</router-link>
        <div class="links">
          <router-link to="/">Курси</router-link>
          <router-link to="/banks">Банки</router-link>
          <router-link to="/branches">Відділення</router-link>
          <router-link to="/statistics">Статистика</router-link>
          <router-link to="/profile">Профіль</router-link>
        </div>
      </nav>
      <section class="hero-grid">
        <div>
          <p class="eyebrow">Vue 3 + Python stdlib API</p>
          <h1>Актуальні курси валют, банки та найближчі відділення</h1>
          <p>Дашборд агрегує НБУ, банківські курси, fallback-дані та історію змін в одному адаптивному інтерфейсі.</p>
        </div>
        <div class="status-card">
          <span class="status-dot"></span>
          <strong>{{ store.health?.status || 'loading' }}</strong>
          <small>Оновлення курсів: {{ store.health?.lastRatesRefresh || '—' }}</small>
          <small v-if="store.health?.externalErrors?.length">API warnings: {{ store.health.externalErrors.length }}</small>
        </div>
      </section>
    </header>
    <main class="container">
      <p v-if="store.error" class="alert">{{ store.error }}</p>
      <router-view />
    </main>
  `,
};

const Filters = {
  setup() {
    const store = useDashboardStore();
    const toggle = async (group, value) => {
      const bucket = store.filters[group];
      const index = bucket.indexOf(value);
      if (index >= 0) bucket.splice(index, 1);
      else bucket.push(value);
      await store.refreshRates();
    };
    return { store, toggle };
  },
  template: `
    <section class="panel filters">
      <div>
        <h2>Фільтри</h2>
        <p>Обирайте банки та валюти, щоб оновити таблиці й summary.</p>
      </div>
      <div class="chips">
        <button v-for="bank in store.banks" :key="bank.slug" @click="toggle('banks', bank.slug)" :class="['chip', { active: store.filters.banks.includes(bank.slug) }]">{{ bank.name }}</button>
      </div>
      <div class="chips">
        <button v-for="currency in store.currencies" :key="currency.code" @click="toggle('currencies', currency.code)" :class="['chip', { active: store.filters.currencies.includes(currency.code) }]">{{ currency.label }}</button>
      </div>
    </section>
  `,
};

const Dashboard = {
  components: { Filters },
  setup() {
    const store = useDashboardStore();
    const groupedRates = computed(() => store.rates);
    return { store, groupedRates };
  },
  template: `
    <Filters />
    <section class="grid cards">
      <article v-for="item in store.summary" :key="item.currency" class="metric-card">
        <span>{{ item.currency.toUpperCase() }}</span>
        <strong>{{ item.nbu?.rate?.toFixed?.(4) || '—' }}</strong>
        <small>НБУ</small>
        <div class="muted">Середній buy/sell: {{ item.averageBuy || '—' }} / {{ item.averageSell || '—' }}</div>
      </article>
    </section>
    <section class="panel">
      <div class="section-title"><h2>Актуальні курси банків</h2><span>{{ store.rates.length }} записів</span></div>
      <div class="table-wrap">
        <table>
          <thead><tr><th>Банк</th><th>Валюта</th><th>Купівля</th><th>Продаж</th><th>Джерело</th><th>Оновлено</th></tr></thead>
          <tbody>
            <tr v-for="rate in groupedRates" :key="rate.bankSlug + rate.currency">
              <td>{{ rate.bankName }}</td><td>{{ rate.currency.toUpperCase() }}</td><td>{{ rate.buy }}</td><td>{{ rate.sell }}</td><td>{{ rate.source }}</td><td>{{ rate.updatedAt }}</td>
            </tr>
          </tbody>
        </table>
      </div>
    </section>
  `,
};

const Banks = {
  setup() {
    const store = useDashboardStore();
    return { store };
  },
  template: `
    <section class="grid bank-grid">
      <router-link v-for="bank in store.banks" :key="bank.slug" :to="'/banks/' + bank.slug" class="bank-card">
        <img :src="bank.logo" :alt="bank.name" @error="$event.target.style.display='none'" />
        <div><h2>{{ bank.name }}</h2><p>{{ bank.description }}</p><strong>★ {{ bank.rating }}</strong></div>
      </router-link>
    </section>
  `,
};

const BankDetails = {
  setup() {
    const route = VueRouter.useRoute();
    const bank = ref(null);
    const loading = ref(true);
    onMounted(async () => {
      const response = await api.get(`/banks/${route.params.slug}`);
      bank.value = response.data;
      loading.value = false;
    });
    return { bank, loading };
  },
  template: `
    <section v-if="bank" class="panel detail">
      <div class="section-title"><h2>{{ bank.name }}</h2><strong>★ {{ bank.rating }}</strong></div>
      <p>{{ bank.description }}</p>
      <div class="info-grid"><span>Телефон: {{ bank.phone }}</span><span>Email: {{ bank.email }}</span><span>Сайт: <a :href="bank.site" target="_blank">{{ bank.site }}</a></span><span>Юр. адреса: {{ bank.legalAddress }}</span></div>
      <h3>Поточні курси</h3>
      <div class="mini-grid"><div v-for="rate in bank.rates" :key="rate.currency" class="metric-card"><span>{{ rate.currency.toUpperCase() }}</span><strong>{{ rate.buy }} / {{ rate.sell }}</strong></div></div>
      <h3>Відділення</h3>
      <ul class="branch-list"><li v-for="branch in bank.branches" :key="branch.id"><strong>{{ branch.name }}</strong><span>{{ branch.address }}</span></li></ul>
    </section>
    <p v-else class="panel">Завантаження...</p>
  `,
};

const Branches = {
  setup() {
    const store = useDashboardStore();
    const status = ref('Натисніть кнопку, щоб використати геолокацію або fallback Київ центр.');
    const locate = () => {
      if (!navigator.geolocation) {
        store.loadNearest({ lat: 50.4501, lng: 30.5234 });
        status.value = 'Geolocation недоступна, використано координати центру Києва.';
        return;
      }
      navigator.geolocation.getCurrentPosition(
        (pos) => {
          store.loadNearest({ lat: pos.coords.latitude, lng: pos.coords.longitude });
          status.value = 'Найближчі відділення розраховано за вашими координатами.';
        },
        () => {
          store.loadNearest({ lat: 50.4501, lng: 30.5234 });
          status.value = 'Доступ до геолокації відхилено, використано координати центру Києва.';
        },
      );
    };
    onMounted(() => store.loadNearest({ lat: 50.4501, lng: 30.5234 }));
    return { store, status, locate };
  },
  template: `
    <section class="panel">
      <div class="section-title"><div><h2>Найближчі відділення</h2><p>{{ status }}</p></div><button class="primary" @click="locate">Визначити поруч</button></div>
      <div class="map-grid">
        <article v-for="branch in store.branches" :key="branch.id" class="branch-card"><strong>{{ branch.name }}</strong><span>{{ branch.address }}</span><small>{{ branch.distanceKm }} км · {{ branch.phone }}</small></article>
      </div>
    </section>
  `,
};

const Statistics = {
  setup() {
    const store = useDashboardStore();
    const bank = ref('');
    const currency = ref('usd');
    const from = ref(new Date(Date.now() - 7 * 86400000).toISOString().slice(0, 10));
    const to = ref(new Date().toISOString().slice(0, 10));
    const load = () => store.loadStatistics({ bank: bank.value, currency: currency.value, from: from.value, to: to.value });
    const chartBars = computed(() => store.statistics.slice(-30).map((row) => ({ ...row, height: Math.max(10, Math.min(100, ((row.sell || 0) / 55) * 100)) })));
    onMounted(load);
    return { store, bank, currency, from, to, load, chartBars };
  },
  template: `
    <section class="panel filters">
      <h2>Статистика курсу</h2>
      <select v-model="bank"><option value="">Всі банки</option><option v-for="item in store.banks" :value="item.slug">{{ item.name }}</option></select>
      <select v-model="currency"><option v-for="item in store.currencies" :value="item.code">{{ item.label }}</option></select>
      <input v-model="from" type="date" /><input v-model="to" type="date" /><button class="primary" @click="load">Оновити</button>
    </section>
    <section class="panel">
      <div class="section-title"><h2>Графік sell rate</h2><span>{{ store.statistics.length }} точок</span></div>
      <div class="chart"><div v-for="bar in chartBars" :style="{ height: bar.height + '%' }" :title="bar.timestamp + ': ' + bar.sell"></div></div>
      <h3>Істотні зміни</h3>
      <ul class="branch-list"><li v-for="event in store.events" :key="event.id"><strong>{{ event.currency.toUpperCase() }} {{ event.changePercent }}%</strong><span>{{ event.bankName }} · {{ event.message }}</span></li></ul>
    </section>
  `,
};

const Profile = {
  setup() {
    const email = ref('demo@example.com');
    const password = ref('password');
    const token = ref(localStorage.getItem('token') || '');
    const user = ref(null);
    const message = ref('In-memory account demo: реєстрація, логін, профіль і підписки живуть до перезапуску backend.');
    const register = async () => {
      const response = await api.post('/auth/register', { email: email.value, password: password.value, name: 'Demo User' });
      token.value = response.data.token;
      user.value = response.data.user;
      localStorage.setItem('token', token.value);
    };
    const login = async () => {
      const response = await api.post('/auth/login', { email: email.value, password: password.value });
      token.value = response.data.token;
      user.value = response.data.user;
      localStorage.setItem('token', token.value);
    };
    const save = async () => {
      const response = await api.put('/profile', user.value, { headers: { Authorization: `Bearer ${token.value}` } });
      user.value = response.data.user;
      message.value = 'Профіль оновлено.';
    };
    return { email, password, token, user, message, register, login, save };
  },
  template: `
    <section class="panel profile">
      <h2>Обліковий запис і підписки</h2><p>{{ message }}</p>
      <div class="form-row"><input v-model="email" placeholder="email" /><input v-model="password" type="password" placeholder="password" /><button class="primary" @click="register">Register</button><button @click="login">Login</button></div>
      <div v-if="user" class="profile-box"><input v-model="user.name" /><label><input type="checkbox" v-model="user.notificationsEnabled" /> email-сповіщення про істотні зміни</label><textarea v-model="user.subscriptions.currencies" /><button class="primary" @click="save">Зберегти профіль</button></div>
    </section>
  `,
};

const router = createRouter({
  history: createWebHistory(),
  routes: [
    { path: '/', component: Dashboard },
    { path: '/banks', component: Banks },
    { path: '/banks/:slug', component: BankDetails },
    { path: '/branches', component: Branches },
    { path: '/statistics', component: Statistics },
    { path: '/profile', component: Profile },
  ],
});

createApp(Shell).use(createPinia()).use(router).mount('#app');
