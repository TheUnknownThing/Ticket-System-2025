function railwayApp() {
    return {
        // State
        isLoggedIn: false,
        currentUser: null,
        activeTab: 'search',
        showLogin: false,
        showRegister: false,
        loading: false,
        
        // Toast notifications
        toast: {
            show: false,
            type: 'success',
            title: '',
            message: ''
        },

        // Forms
        loginForm: {
            username: '',
            password: ''
        },
        
        registerForm: {
            username: '',
            password: '',
            name: '',
            mailAddr: '',
            privilege: 1,
            cur_username: '' // For first user, this can be empty
        },

        searchForm: {
            from: '',
            to: '',
            date: '',
            sort: 'time'
        },

        // Data
        tickets: [],
        transfers: [],
        orders: [],
        trains: [],

        // API Base URL
        apiBase: '/api',

        // Initialize
        init() {
            // Set default date to today
            const today = new Date().toISOString().split('T')[0];
            this.searchForm.date = today;
            
            // Check if user is logged in (from localStorage)
            const savedUser = localStorage.getItem('currentUser');
            if (savedUser) {
                this.currentUser = JSON.parse(savedUser);
                this.isLoggedIn = true;
            }
        },

        // Utility Methods
        showToast(type, title, message) {
            this.toast = { show: true, type, title, message };
            setTimeout(() => {
                this.toast.show = false;
            }, 5000);
        },

        async apiCall(endpoint, method = 'GET', data = null, params = null) {
            this.loading = true;
            try {
                let url = `${this.apiBase}${endpoint}`;
                if (params) {
                    const searchParams = new URLSearchParams(params);
                    url += `?${searchParams.toString()}`;
                }

                const options = {
                    method,
                    headers: {
                        'Content-Type': 'application/json',
                    }
                };

                if (data) {
                    options.body = JSON.stringify(data);
                }

                // Log the API call details
                console.log(`[API] ${method} ${url}`);
                if (params) {
                    console.log('[API] Params:', params);
                }
                if (data) {
                    console.log('[API] Body:', data);
                }

                const response = await fetch(url, options);
                const result = await response.json();
                return result;
            } catch (error) {
                console.error('API call failed:', error);
                this.showToast('error', 'Error', 'Network error occurred');
                return { status: -1 };
            } finally {
                this.loading = false;
            }
        },

        // Authentication Methods
        async login() {
            const result = await this.apiCall('/sessions', 'POST', this.loginForm);
            
            if (result.status === 0) {
                // Get user profile
                const profileResult = await this.apiCall(`/users/${this.loginForm.username}`, 'GET', null, {
                    cur: this.loginForm.username
                });
                
                if (profileResult.status === 0) {
                    this.currentUser = profileResult.profile;
                    this.isLoggedIn = true;
                    localStorage.setItem('currentUser', JSON.stringify(this.currentUser));
                    this.showLogin = false;
                    this.loginForm = { username: '', password: '' };
                    this.showToast('success', 'Success', 'Logged in successfully');
                }
            } else {
                this.showToast('error', 'Login Failed', 'Invalid username or password');
            }
        },

        async register() {
            const result = await this.apiCall('/users', 'POST', this.registerForm);
            
            if (result.status === 0) {
                this.showRegister = false;
                this.registerForm = { username: '', password: '', name: '', mailAddr: '', privilege: 1, cur_username: '' };
                this.showToast('success', 'Success', 'Account created successfully. Please log in.');
            } else {
                this.showToast('error', 'Registration Failed', 'Username already exists or insufficient privileges');
            }
        },

        async logout() {
            if (this.currentUser) {
                await this.apiCall(`/sessions/${this.currentUser.username}`, 'DELETE');
                this.isLoggedIn = false;
                this.currentUser = null;
                localStorage.removeItem('currentUser');
                this.showToast('success', 'Success', 'Logged out successfully');
            }
        },

        // Search Methods
        async searchTickets() {
            if (!this.searchForm.from || !this.searchForm.to || !this.searchForm.date) {
                this.showToast('error', 'Validation Error', 'Please fill all search fields');
                return;
            }

            const dateParts = this.searchForm.date.split('-');
            const shortDate = dateParts.length === 3 ? `${dateParts[1]}-${dateParts[2]}` : this.searchForm.date;

            const result = await this.apiCall('/tickets', 'GET', null, {
                from: this.searchForm.from,
                to: this.searchForm.to,
                date: shortDate,
                sort: this.searchForm.sort
            });

            if (result.status === 0) {
                if (result.ticket_list && result.ticket_list.length > 0) {
                    result.ticket_list.shift();
                }
                this.tickets = result.ticket_list || [];
                this.showToast('success', 'Success', `Found ${this.tickets.length} tickets`);
            } else {
                this.showToast('error', 'Search Failed', 'No tickets found for this route');
                this.tickets = [];
            }
        },

        async searchTransfers() {
            if (!this.searchForm.from || !this.searchForm.to || !this.searchForm.date) {
                this.showToast('error', 'Validation Error', 'Please fill all search fields');
                return;
            }

            const dateParts = this.searchForm.date.split('-');
            const shortDate = dateParts.length === 3 ? `${dateParts[1]}-${dateParts[2]}` : this.searchForm.date;

            const result = await this.apiCall('/transfer', 'GET', null, {
                from: this.searchForm.from,
                to: this.searchForm.to,
                date: shortDate,
                sort: this.searchForm.sort
            });

            if (result.status === 0) {
                this.transfers = result.transfer || [];
                this.showToast('success', 'Success', `Found ${this.transfers.length} transfer options`);
            } else {
                this.showToast('error', 'Search Failed', 'No transfer options found for this route');
                this.transfers = [];
            }
        },

        async buyTicket(trainID, num = 1, queue = false) {

            const dateParts = this.searchForm.date.split('-');
            const shortDate = dateParts.length === 3 ? `${dateParts[1]}-${dateParts[2]}` : this.searchForm.date;

            const result = await this.apiCall(`/orders/${this.currentUser.username}/buy`, 'POST', {
                trainID: trainID,
                date: shortDate,
                num: num,
                from: this.searchForm.from,
                to: this.searchForm.to,
                queue: queue
            });

            if (result.status === 0) {
                const message = result.result === 'queue' ? 'Ticket added to queue' : `Tickets purchased. Total: $${result.result}`;
                this.showToast('success', 'Success', message);
                this.loadOrders(); // Refresh orders
            } else {
                this.showToast('error', 'Purchase Failed', 'Unable to purchase ticket');
            }
        },

        // Order Methods
        async loadOrders() {
            const result = await this.apiCall(`/orders/${this.currentUser.username}`, 'GET');
            
            if (result.status === 0) {
                this.orders = result.orders || [];
            } else {
                this.orders = [];
            }
        },

        async refundTicket(orderIndex) {
            const result = await this.apiCall(`/orders/${this.currentUser.username}/refund`, 'POST', {
                index: orderIndex
            });

            if (result.status === 0) {
                this.showToast('success', 'Success', 'Ticket refunded successfully');
                this.loadOrders(); // Refresh orders
            } else {
                this.showToast('error', 'Refund Failed', 'Unable to refund ticket');
            }
        },

        // Admin Methods
        async addTrain(trainData) {
            const result = await this.apiCall('/trains', 'POST', trainData);
            
            if (result.status === 0) {
                this.showToast('success', 'Success', 'Train added successfully');
            } else {
                this.showToast('error', 'Failed', 'Unable to add train');
            }
        },

        async deleteTrain(trainID) {
            const result = await this.apiCall(`/trains/${trainID}`, 'DELETE');
            
            if (result.status === 0) {
                this.showToast('success', 'Success', 'Train deleted successfully');
            } else {
                this.showToast('error', 'Failed', 'Unable to delete train');
            }
        },

        async releaseTrain(trainID) {
            const result = await this.apiCall(`/trains/${trainID}/release`, 'POST');
            
            if (result.status === 0) {
                this.showToast('success', 'Success', 'Train released successfully');
            } else {
                this.showToast('error', 'Failed', 'Unable to release train');
            }
        },

        // Render Methods
        renderSearchTab() {
            return `
                <div class="space-y-8">
                    <!-- Search Form -->
                    <div class="bg-white rounded-lg shadow p-6">
                        <h3 class="text-lg font-semibold mb-4">Search Tickets</h3>
                        <div class="grid grid-cols-1 md:grid-cols-4 gap-4 mb-4">
                            <div>
                                <label class="block text-sm font-medium text-gray-700 mb-2">From</label>
                                <input x-model="searchForm.from" type="text" placeholder="Departure station" 
                                       class="w-full px-3 py-2 border border-gray-300 rounded-md focus:outline-none focus:ring-2 focus:ring-blue-500">
                            </div>
                            <div>
                                <label class="block text-sm font-medium text-gray-700 mb-2">To</label>
                                <input x-model="searchForm.to" type="text" placeholder="Arrival station"
                                       class="w-full px-3 py-2 border border-gray-300 rounded-md focus:outline-none focus:ring-2 focus:ring-blue-500">
                            </div>
                            <div>
                                <label class="block text-sm font-medium text-gray-700 mb-2">Date</label>
                                <input x-model="searchForm.date" type="date" 
                                       class="w-full px-3 py-2 border border-gray-300 rounded-md focus:outline-none focus:ring-2 focus:ring-blue-500">
                            </div>
                            <div>
                                <label class="block text-sm font-medium text-gray-700 mb-2">Sort by</label>
                                <div class="relative">
                                    <select x-model="searchForm.sort"
                                            class="block appearance-none w-full px-3 py-2 border border-gray-300 rounded-md bg-white focus:outline-none focus:ring-2 focus:ring-blue-500 pr-8">
                                        <option value="time">Time</option>
                                        <option value="cost">Cost</option>
                                    </select>
                                    <div class="pointer-events-none absolute inset-y-0 right-0 flex items-center px-2 text-gray-400">
                                        <svg class="h-4 w-4" fill="none" stroke="currentColor" stroke-width="2" viewBox="0 0 24 24">
                                            <path stroke-linecap="round" stroke-linejoin="round" d="M19 9l-7 7-7-7"/>
                                        </svg>
                                    </div>
                                </div>
                            </div>
                        </div>
                        <div class="flex space-x-4">
                            <button @click="searchTickets()" class="px-6 py-2 bg-blue-600 text-white rounded-md hover:bg-blue-700">
                                <i class="fas fa-search mr-2"></i>Search Direct Tickets
                            </button>
                            <button @click="searchTransfers()" class="px-6 py-2 bg-green-600 text-white rounded-md hover:bg-green-700">
                                <i class="fas fa-exchange-alt mr-2"></i>Search Transfers
                            </button>
                        </div>
                    </div>

                    <!-- Direct Tickets Results -->
                    <div x-show="tickets.length > 0" class="bg-white rounded-lg shadow">
                        <div class="px-6 py-4 border-b border-gray-200">
                            <h3 class="text-lg font-semibold">Direct Tickets</h3>
                        </div>
                        <div class="divide-y divide-gray-200">
                            <template x-for="(ticket, index) in tickets" :key="index">
                                <div class="p-6 hover:bg-gray-50">
                                    <div class="flex justify-between items-center">
                                        <div class="flex-1">
                                            <div class="text-sm text-gray-900" x-text="ticket"></div>
                                        </div>
                                        <div class="ml-4 space-x-2">
                                            <button @click="buyTicket(ticket.split(' ')[0], 1, false)" 
                                                    class="px-4 py-2 bg-blue-600 text-white rounded hover:bg-blue-700">
                                                Buy Ticket
                                            </button>
                                            <button @click="buyTicket(ticket.split(' ')[0], 1, true)" 
                                                    class="px-4 py-2 bg-yellow-600 text-white rounded hover:bg-yellow-700">
                                                Queue
                                            </button>
                                        </div>
                                    </div>
                                </div>
                            </template>
                        </div>
                    </div>

                    <!-- Transfer Results -->
                    <div x-show="transfers.length > 0" class="bg-white rounded-lg shadow">
                        <div class="px-6 py-4 border-b border-gray-200">
                            <h3 class="text-lg font-semibold">Transfer Options</h3>
                        </div>
                        <div class="divide-y divide-gray-200">
                            <template x-for="(transfer, index) in transfers" :key="index">
                                <div class="p-6 hover:bg-gray-50">
                                    <div class="text-sm text-gray-900" x-text="transfer"></div>
                                </div>
                            </template>
                        </div>
                    </div>

                    <!-- No Results -->
                    <div x-show="tickets.length === 0 && transfers.length === 0" class="text-center py-8 text-gray-500">
                        <i class="fas fa-search text-4xl mb-4"></i>
                        <p>No tickets found. Try searching with different criteria.</p>
                    </div>
                </div>
            `;
        },

        renderOrdersTab() {
            return `
                <div class="bg-white rounded-lg shadow">
                    <div class="px-6 py-4 border-b border-gray-200">
                        <h3 class="text-lg font-semibold">My Orders</h3>
                    </div>
                    <div x-show="orders.length > 0" class="divide-y divide-gray-200">
                        <template x-for="(order, index) in orders" :key="index">
                            <div class="p-6">
                                <div class="flex justify-between items-start">
                                    <div class="flex-1">
                                        <div class="grid grid-cols-2 md:grid-cols-4 gap-4">
                                            <div>
                                                <p class="text-sm font-medium text-gray-900">Train ID</p>
                                                <p class="text-sm text-gray-600" x-text="order.trainID"></p>
                                            </div>
                                            <div>
                                                <p class="text-sm font-medium text-gray-900">Route</p>
                                                <p class="text-sm text-gray-600" x-text="order.from_station_name + ' → ' + order.to_station_name"></p>
                                            </div>
                                            <div>
                                                <p class="text-sm font-medium text-gray-900">Departure</p>
                                                <p class="text-sm text-gray-600" x-text="order.departureFromStation"></p>
                                            </div>
                                            <div>
                                                <p class="text-sm font-medium text-gray-900">Arrival</p>
                                                <p class="text-sm text-gray-600" x-text="order.arrivalAtStation"></p>
                                            </div>
                                            <div>
                                                <p class="text-sm font-medium text-gray-900">Price</p>
                                                <p class="text-sm text-gray-600">$<span x-text="order.price"></span></p>
                                            </div>
                                            <div>
                                                <p class="text-sm font-medium text-gray-900">Quantity</p>
                                                <p class="text-sm text-gray-600" x-text="order.num"></p>
                                            </div>
                                            <div>
                                                <p class="text-sm font-medium text-gray-900">Status</p>
                                                <span :class="order.status === 'success' ? 'bg-green-100 text-green-800' : 
                                                             order.status === 'pending' ? 'bg-yellow-100 text-yellow-800' : 
                                                             'bg-red-100 text-red-800'"
                                                      class="px-2 py-1 text-xs rounded-full font-medium" x-text="order.status"></span>
                                            </div>
                                            <div x-show="order.status === 'success'">
                                                <button @click="refundTicket(index + 1)" 
                                                        class="px-3 py-1 bg-red-600 text-white text-sm rounded hover:bg-red-700">
                                                    Refund
                                                </button>
                                            </div>
                                        </div>
                                    </div>
                                </div>
                            </div>
                        </template>
                    </div>
                    <div x-show="orders.length === 0" class="text-center py-8 text-gray-500">
                        <i class="fas fa-ticket-alt text-4xl mb-4"></i>
                        <p>No orders found.</p>
                    </div>
                </div>
            `;
        },

        renderProfileTab() {
            return `
                <div class="bg-white rounded-lg shadow p-6">
                    <h3 class="text-lg font-semibold mb-6">Profile Information</h3>
                    <div class="grid grid-cols-1 md:grid-cols-2 gap-6">
                        <div>
                            <label class="block text-sm font-medium text-gray-700 mb-2">Username</label>
                            <p class="text-sm text-gray-900 py-2 px-3 bg-gray-50 rounded" x-text="currentUser.username"></p>
                        </div>
                        <div>
                            <label class="block text-sm font-medium text-gray-700 mb-2">Name</label>
                            <p class="text-sm text-gray-900 py-2 px-3 bg-gray-50 rounded" x-text="currentUser.name"></p>
                        </div>
                        <div>
                            <label class="block text-sm font-medium text-gray-700 mb-2">Email</label>
                            <p class="text-sm text-gray-900 py-2 px-3 bg-gray-50 rounded" x-text="currentUser.mailAddr"></p>
                        </div>
                        <div>
                            <label class="block text-sm font-medium text-gray-700 mb-2">Privilege Level</label>
                            <p class="text-sm text-gray-900 py-2 px-3 bg-gray-50 rounded" x-text="currentUser.privilege"></p>
                        </div>
                    </div>
                </div>
            `;
        },

        renderAdminTab() {
            return `
                <div class="space-y-8">
                    <!-- Add Train -->
                    <div class="bg-white rounded-lg shadow p-6">
                        <h3 class="text-lg font-semibold mb-4">Add Train</h3>
                        <div class="grid grid-cols-1 md:grid-cols-2 gap-4">
                            <div>
                                <label class="block text-sm font-medium text-gray-700 mb-2">Train ID</label>
                                <input x-model="adminForms.addTrain.trainID" type="text" 
                                       class="w-full px-3 py-2 border border-gray-300 rounded-md focus:outline-none focus:ring-2 focus:ring-blue-500">
                            </div>
                            <div>
                                <label class="block text-sm font-medium text-gray-700 mb-2">Station Number</label>
                                <input x-model="adminForms.addTrain.stationNum" type="number" 
                                       class="w-full px-3 py-2 border border-gray-300 rounded-md focus:outline-none focus:ring-2 focus:ring-blue-500">
                            </div>
                            <div>
                                <label class="block text-sm font-medium text-gray-700 mb-2">Seat Number</label>
                                <input x-model="adminForms.addTrain.seatNum" type="number" 
                                       class="w-full px-3 py-2 border border-gray-300 rounded-md focus:outline-none focus:ring-2 focus:ring-blue-500">
                            </div>
                            <div>
                                <label class="block text-sm font-medium text-gray-700 mb-2">Type</label>
                                <input x-model="adminForms.addTrain.type" type="text" maxlength="1"
                                       class="w-full px-3 py-2 border border-gray-300 rounded-md focus:outline-none focus:ring-2 focus:ring-blue-500">
                            </div>
                            <div class="md:col-span-2">
                                <label class="block text-sm font-medium text-gray-700 mb-2">Stations (pipe-separated)</label>
                                <input x-model="adminForms.addTrain.stations" type="text" placeholder="StationA|StationB|StationC"
                                       class="w-full px-3 py-2 border border-gray-300 rounded-md focus:outline-none focus:ring-2 focus:ring-blue-500">
                            </div>
                            <div class="md:col-span-2">
                                <label class="block text-sm font-medium text-gray-700 mb-2">Prices (pipe-separated)</label>
                                <input x-model="adminForms.addTrain.prices" type="text" placeholder="100|200|300"
                                       class="w-full px-3 py-2 border border-gray-300 rounded-md focus:outline-none focus:ring-2 focus:ring-blue-500">
                            </div>
                            <div>
                                <label class="block text-sm font-medium text-gray-700 mb-2">Start Time</label>
                                <input x-model="adminForms.addTrain.startTime" type="time"
                                       class="w-full px-3 py-2 border border-gray-300 rounded-md focus:outline-none focus:ring-2 focus:ring-blue-500">
                            </div>
                            <div>
                                <label class="block text-sm font-medium text-gray-700 mb-2">Sale Date</label>
                                <input x-model="adminForms.addTrain.saleDate" type="text" placeholder="06-01|08-17"
                                       class="w-full px-3 py-2 border border-gray-300 rounded-md focus:outline-none focus:ring-2 focus:ring-blue-500">
                            </div>
                            <div class="md:col-span-2">
                                <label class="block text-sm font-medium text-gray-700 mb-2">Travel Times (pipe-separated minutes)</label>
                                <input x-model="adminForms.addTrain.travelTimes" type="text" placeholder="60|90|120"
                                       class="w-full px-3 py-2 border border-gray-300 rounded-md focus:outline-none focus:ring-2 focus:ring-blue-500">
                            </div>
                            <div class="md:col-span-2">
                                <label class="block text-sm font-medium text-gray-700 mb-2">Stopover Times (pipe-separated minutes)</label>
                                <input x-model="adminForms.addTrain.stopoverTimes" type="text" placeholder="5|10|0"
                                       class="w-full px-3 py-2 border border-gray-300 rounded-md focus:outline-none focus:ring-2 focus:ring-blue-500">
                            </div>
                        </div>
                        <div class="mt-4">
                            <button @click="addTrain(adminForms.addTrain)" class="px-4 py-2 bg-blue-600 text-white rounded hover:bg-blue-700">
                                Add Train
                            </button>
                        </div>
                    </div>

                    <!-- Train Management -->
                    <div class="bg-white rounded-lg shadow p-6">
                        <h3 class="text-lg font-semibold mb-4">Train Management</h3>
                        <div class="flex space-x-4 mb-4">
                            <input x-model="adminForms.trainID" type="text" placeholder="Train ID" 
                                   class="px-3 py-2 border border-gray-300 rounded-md focus:outline-none focus:ring-2 focus:ring-blue-500">
                            <button @click="releaseTrain(adminForms.trainID)" class="px-4 py-2 bg-green-600 text-white rounded hover:bg-green-700">
                                Release Train
                            </button>
                            <button @click="deleteTrain(adminForms.trainID)" class="px-4 py-2 bg-red-600 text-white rounded hover:bg-red-700">
                                Delete Train
                            </button>
                        </div>
                    </div>

                    <!-- System Management -->
                    <div class="bg-white rounded-lg shadow p-6">
                        <h3 class="text-lg font-semibold mb-4 text-red-600">System Management</h3>
                        <div class="space-x-4">
                            <button @click="cleanSystem()" class="px-4 py-2 bg-yellow-600 text-white rounded hover:bg-yellow-700">
                                Clean System
                            </button>
                            <button @click="exitSystem()" class="px-4 py-2 bg-red-600 text-white rounded hover:bg-red-700">
                                Exit System
                            </button>
                        </div>
                    </div>
                </div>
            `;
        },

        // Admin form data
        adminForms: {
            addTrain: {
                trainID: '',
                stationNum: 0,
                seatNum: 0,
                stations: '',
                prices: '',
                startTime: '',
                travelTimes: '',
                stopoverTimes: '',
                saleDate: '',
                type: 'G'
            },
            trainID: ''
        },

        async cleanSystem() {
            if (confirm('Are you sure you want to clean the system? This will remove all data.')) {
                const result = await this.apiCall('/system/clean', 'POST');
                if (result.status === 0) {
                    this.showToast('success', 'Success', 'System cleaned successfully');
                }
            }
        },

        async exitSystem() {
            if (confirm('Are you sure you want to exit the system?')) {
                await this.apiCall('/system/exit', 'POST');
                this.showToast('success', 'Success', 'System exit requested');
            }
        }
    }
}
