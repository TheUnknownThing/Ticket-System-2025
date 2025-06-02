// API Configuration
const API_BASE = '/api';

// State Management
let currentUser = null;
let isLoggedIn = false;

// DOM Elements
const sections = document.querySelectorAll('.section');
const navLinks = document.querySelectorAll('.nav-link');
const loginModal = document.getElementById('login-modal');
const registerModal = document.getElementById('register-modal');
const addTrainModal = document.getElementById('add-train-modal');
const buyTicketModal = document.getElementById('buy-ticket-modal');
const notification = document.getElementById('notification');

// Initialize the application
document.addEventListener('DOMContentLoaded', function() {
    initializeNavigation();
    initializeModals();
    initializeForms();
    initializeAuth();
    setCurrentDate();
    showSection('search');
});

// Navigation
function initializeNavigation() {
    navLinks.forEach(link => {
        link.addEventListener('click', (e) => {
            e.preventDefault();
            const section = link.getAttribute('data-section');
            showSection(section);
            
            // Update active nav link
            navLinks.forEach(l => l.classList.remove('active'));
            link.classList.add('active');
        });
    });
}

function showSection(sectionName) {
    sections.forEach(section => section.classList.remove('active'));
    document.getElementById(`${sectionName}-section`).classList.add('active');
    
    // Load section-specific data
    switch(sectionName) {
        case 'orders':
            loadOrders();
            break;
        case 'profile':
            loadProfile();
            break;
        case 'admin':
            loadAdminPanel();
            break;
    }
}

// Modal Management
function initializeModals() {
    const modals = [loginModal, registerModal, addTrainModal, buyTicketModal];
    
    modals.forEach(modal => {
        const closeBtn = modal.querySelector('.close');
        closeBtn.addEventListener('click', () => closeModal(modal));
        
        modal.addEventListener('click', (e) => {
            if (e.target === modal) closeModal(modal);
        });
    });
    
    document.getElementById('login-btn').addEventListener('click', () => openModal(loginModal));
    document.getElementById('register-btn').addEventListener('click', () => openModal(registerModal));
    document.getElementById('add-train-btn').addEventListener('click', () => openModal(addTrainModal));
    document.getElementById('logout-btn').addEventListener('click', logout);
}

function openModal(modal) {
    modal.style.display = 'block';
}

function closeModal(modal) {
    modal.style.display = 'none';
}

// Form Initialization
function initializeForms() {
    document.getElementById('login-form').addEventListener('submit', handleLogin);
    document.getElementById('register-form').addEventListener('submit', handleRegister);
    document.getElementById('add-train-form').addEventListener('submit', handleAddTrain);
    document.getElementById('buy-ticket-form').addEventListener('submit', handleBuyTicket);
    document.getElementById('search-btn').addEventListener('click', searchTickets);
    document.getElementById('search-transfer-btn').addEventListener('click', searchTransfer);
}

// Authentication
function initializeAuth() {
    // Check if user is already logged in (you might want to implement session persistence)
    updateAuthUI();
}

function updateAuthUI() {
    const authSection = document.getElementById('auth-section');
    const logoutBtn = document.getElementById('logout-btn');
    
    if (isLoggedIn) {
        authSection.style.display = 'none';
        logoutBtn.style.display = 'block';
        logoutBtn.textContent = `Logout (${currentUser})`;
    } else {
        authSection.style.display = 'flex';
        logoutBtn.style.display = 'none';
    }
}

async function handleLogin(e) {
    e.preventDefault();
    const username = document.getElementById('login-username').value;
    const password = document.getElementById('login-password').value;
    
    try {
        const response = await fetch(`${API_BASE}/sessions`, {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ username, password })
        });
        
        const data = await response.json();
        
        if (data.status === 0) {
            currentUser = username;
            isLoggedIn = true;
            updateAuthUI();
            closeModal(loginModal);
            showNotification('Login successful!', 'success');
            document.getElementById('login-form').reset();
        } else {
            showNotification('Login failed. Please check your credentials.', 'error');
        }
    } catch (error) {
        showNotification('Network error. Please try again.', 'error');
    }
}

async function handleRegister(e) {
    e.preventDefault();
    const curUsername = document.getElementById('reg-cur-username').value || '';
    const username = document.getElementById('reg-username').value;
    const password = document.getElementById('reg-password').value;
    const name = document.getElementById('reg-name').value;
    const mailAddr = document.getElementById('reg-email').value;
    const privilege = parseInt(document.getElementById('reg-privilege').value);
    
    try {
        const response = await fetch(`${API_BASE}/users`, {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({
                cur_username: curUsername,
                username,
                password,
                name,
                mailAddr,
                privilege
            })
        });
        
        const data = await response.json();
        
        if (data.status === 0) {
            closeModal(registerModal);
            showNotification('Registration successful! Please login.', 'success');
            document.getElementById('register-form').reset();
        } else {
            showNotification('Registration failed. Please check your information.', 'error');
        }
    } catch (error) {
        showNotification('Network error. Please try again.', 'error');
    }
}

async function logout() {
    if (!currentUser) return;
    
    try {
        const response = await fetch(`${API_BASE}/sessions/${currentUser}`, {
            method: 'DELETE'
        });
        
        const data = await response.json();
        
        if (data.status === 0) {
            currentUser = null;
            isLoggedIn = false;
            updateAuthUI();
            showNotification('Logged out successfully!', 'success');
            showSection('search');
        }
    } catch (error) {
        showNotification('Logout error. Please try again.', 'error');
    }
}

// Search Functions
async function searchTickets() {
    const from = document.getElementById('from-station').value;
    const to = document.getElementById('to-station').value;
    const date = document.getElementById('travel-date').value;
    const sort = document.getElementById('sort-by').value;
    
    if (!from || !to || !date) {
        showNotification('Please fill in all search fields.', 'error');
        return;
    }
    
    try {
        const response = await fetch(`${API_BASE}/tickets?from=${encodeURIComponent(from)}&to=${encodeURIComponent(to)}&date=${date}&sort=${sort}`);
        const data = await response.json();
        
        if (data.status === 0) {
            displaySearchResults(data.ticket_list, false);
        } else {
            showNotification('No tickets found for the specified route.', 'info');
            displaySearchResults([], false);
        }
    } catch (error) {
        showNotification('Search error. Please try again.', 'error');
    }
}

async function searchTransfer() {
    const from = document.getElementById('from-station').value;
    const to = document.getElementById('to-station').value;
    const date = document.getElementById('travel-date').value;
    const sort = document.getElementById('sort-by').value;
    
    if (!from || !to || !date) {
        showNotification('Please fill in all search fields.', 'error');
        return;
    }
    
    try {
        const response = await fetch(`${API_BASE}/transfer?from=${encodeURIComponent(from)}&to=${encodeURIComponent(to)}&date=${date}&sort=${sort}`);
        const data = await response.json();
        
        if (data.status === 0) {
            displaySearchResults(data.transfer, true);
        } else {
            showNotification('No transfer routes found.', 'info');
            displaySearchResults([], true);
        }
    } catch (error) {
        showNotification('Transfer search error. Please try again.', 'error');
    }
}

function displaySearchResults(results, isTransfer) {
    const resultsContainer = document.getElementById('results-container');
    const searchResults = document.getElementById('search-results');
    
    searchResults.style.display = 'block';
    
    if (!results || results.length === 0) {
        resultsContainer.innerHTML = '<p>No results found.</p>';
        return;
    }
    
    resultsContainer.innerHTML = '';
    
    results.forEach((result, index) => {
        const ticketCard = createTicketCard(result, index, isTransfer);
        resultsContainer.appendChild(ticketCard);
    });
}

function createTicketCard(ticketInfo, index, isTransfer) {
    const card = document.createElement('div');
    card.className = 'ticket-card';
    
    // Parse ticket information (assuming it's a string format)
    const lines = ticketInfo.split('\n');
    const trainInfo = lines[0].split(' ');
    const trainId = trainInfo[0];
    const trainType = trainInfo[1] || 'Unknown';
    
    // Extract route information from subsequent lines
    let routeInfo = '';
    if (lines.length > 1) {
        routeInfo = lines.slice(1).join('<br>');
    }
    
    card.innerHTML = `
        <div class="ticket-header">
            <div class="train-info">
                <span class="train-id">${trainId}</span>
                <span class="train-type">${trainType}</span>
            </div>
            <div class="price-info">
                <div class="price">查看详情</div>
            </div>
        </div>
        <div class="ticket-details">
            <pre style="font-family: inherit; white-space: pre-wrap;">${routeInfo}</pre>
        </div>
        <div class="ticket-actions">
            ${isLoggedIn ? `<button class="btn primary" onclick="openBuyTicketModal('${trainId}', '${document.getElementById('travel-date').value}', '${document.getElementById('from-station').value}', '${document.getElementById('to-station').value}')">Buy Ticket</button>` : '<button class="btn secondary" onclick="showNotification(\'Please login to buy tickets\', \'info\')">Login to Buy</button>'}
        </div>
    `;
    
    return card;
}

// Ticket Buying
function openBuyTicketModal(trainId, date, from, to) {
    if (!isLoggedIn) {
        showNotification('Please login to buy tickets.', 'error');
        return;
    }
    
    const ticketDetails = document.getElementById('ticket-details');
    ticketDetails.innerHTML = `
        <div class="form-group">
            <strong>Train:</strong> ${trainId}<br>
            <strong>Date:</strong> ${date}<br>
            <strong>From:</strong> ${from}<br>
            <strong>To:</strong> ${to}
        </div>
    `;
    
    // Store ticket info for form submission
    buyTicketModal.setAttribute('data-train-id', trainId);
    buyTicketModal.setAttribute('data-date', date);
    buyTicketModal.setAttribute('data-from', from);
    buyTicketModal.setAttribute('data-to', to);
    
    openModal(buyTicketModal);
}

async function handleBuyTicket(e) {
    e.preventDefault();
    
    const trainId = buyTicketModal.getAttribute('data-train-id');
    const date = buyTicketModal.getAttribute('data-date');
    const from = buyTicketModal.getAttribute('data-from');
    const to = buyTicketModal.getAttribute('data-to');
    const num = parseInt(document.getElementById('ticket-num').value);
    const queue = document.getElementById('queue-option').checked;
    
    try {
        const response = await fetch(`${API_BASE}/orders/${currentUser}/buy`, {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({
                trainID: trainId,
                date: date,
                num: num,
                from: from,
                to: to,
                queue: queue
            })
        });
        
        const data = await response.json();
        
        if (data.status === 0) {
            closeModal(buyTicketModal);
            showNotification(`Ticket purchase successful! ${data.result === 'queue' ? 'Added to queue.' : 'Total cost: ' + data.result}`, 'success');
            document.getElementById('buy-ticket-form').reset();
        } else {
            showNotification('Ticket purchase failed. Please try again.', 'error');
        }
    } catch (error) {
        showNotification('Purchase error. Please try again.', 'error');
    }
}

// Orders Management
async function loadOrders() {
    if (!isLoggedIn) {
        document.getElementById('orders-container').innerHTML = '<p class="login-required">Please login to view your orders.</p>';
        return;
    }
    
    try {
        const response = await fetch(`${API_BASE}/orders/${currentUser}`);
        const data = await response.json();
        
        if (data.status === 0) {
            displayOrders(data.orders);
        } else {
            document.getElementById('orders-container').innerHTML = '<p>Failed to load orders.</p>';
        }
    } catch (error) {
        document.getElementById('orders-container').innerHTML = '<p>Error loading orders.</p>';
    }
}

function displayOrders(orders) {
    const container = document.getElementById('orders-container');
    
    if (!orders || orders.length === 0) {
        container.innerHTML = '<p>No orders found.</p>';
        return;
    }
    
    container.innerHTML = orders.map((order, index) => `
        <div class="order-card">
            <div class="order-header">
                <div class="train-info">
                    <span class="train-id">${order.trainID}</span>
                </div>
                <span class="order-status ${order.status}">${order.status}</span>
            </div>
            <div class="order-details">
                <div><strong>From:</strong> ${order.from_station_name}</div>
                <div><strong>To:</strong> ${order.to_station_name}</div>
                <div><strong>Departure:</strong> ${order.departureFromStation}</div>
                <div><strong>Arrival:</strong> ${order.arrivalAtStation}</div>
                <div><strong>Tickets:</strong> ${order.num}</div>
                <div><strong>Price:</strong> ¥${order.price}</div>
            </div>
            ${order.status === 'success' ? `
                <div class="ticket-actions">
                    <button class="btn danger" onclick="refundTicket(${orders.length - index})">Refund</button>
                </div>
            ` : ''}
        </div>
    `).join('');
}

async function refundTicket(index) {
    if (!confirm('Are you sure you want to refund this ticket?')) return;
    
    try {
        const response = await fetch(`${API_BASE}/orders/${currentUser}?index=${index}`, {
            method: 'DELETE'
        });
        
        const data = await response.json();
        
        if (data.status === 0) {
            showNotification('Ticket refunded successfully!', 'success');
            loadOrders(); // Reload orders
        } else {
            showNotification('Refund failed. Please try again.', 'error');
        }
    } catch (error) {
        showNotification('Refund error. Please try again.', 'error');
    }
}

// Profile Management
async function loadProfile() {
    if (!isLoggedIn) {
        document.getElementById('profile-container').innerHTML = '<p class="login-required">Please login to view your profile.</p>';
        return;
    }
    
    try {
        const response = await fetch(`${API_BASE}/users/${currentUser}?cur=${currentUser}`);
        const data = await response.json();
        
        if (data.status === 0) {
            displayProfile(data.profile);
        } else {
            document.getElementById('profile-container').innerHTML = '<p>Failed to load profile.</p>';
        }
    } catch (error) {
        document.getElementById('profile-container').innerHTML = '<p>Error loading profile.</p>';
    }
}

function displayProfile(profile) {
    document.getElementById('profile-container').innerHTML = `
        <div class="profile-card">
            <h3>User Profile</h3>
            <div class="profile-info">
                <div class="info-item">
                    <div class="info-label">Username</div>
                    <div class="info-value">${profile.username}</div>
                </div>
                <div class="info-item">
                    <div class="info-label">Name</div>
                    <div class="info-value">${profile.name}</div>
                </div>
                <div class="info-item">
                    <div class="info-label">Email</div>
                    <div class="info-value">${profile.mailAddr}</div>
                </div>
                <div class="info-item">
                    <div class="info-label">Privilege Level</div>
                    <div class="info-value">${profile.privilege}</div>
                </div>
            </div>
        </div>
    `;
}

// Admin Panel
function loadAdminPanel() {
    if (!isLoggedIn) {
        document.getElementById('admin-container').innerHTML = '<p class="login-required">Please login with admin privileges.</p>';
        return;
    }
    
    // For now, show admin actions if logged in (you might want to check privilege level)
    document.querySelector('.admin-actions').style.display = 'grid';
    
    // Initialize admin action buttons
    document.getElementById('clean-system-btn').onclick = cleanSystem;
    document.getElementById('exit-system-btn').onclick = exitSystem;
}

async function handleAddTrain(e) {
    e.preventDefault();
    
    const trainData = {
        trainID: document.getElementById('train-id').value,
        stationNum: parseInt(document.getElementById('station-num').value),
        seatNum: parseInt(document.getElementById('seat-num').value),
        stations: document.getElementById('stations').value,
        prices: document.getElementById('prices').value,
        startTime: document.getElementById('start-time').value,
        travelTimes: document.getElementById('travel-times').value,
        stopoverTimes: document.getElementById('stopover-times').value,
        saleDate: document.getElementById('sale-date').value,
        type: document.getElementById('train-type').value
    };
    
    try {
        const response = await fetch(`${API_BASE}/trains`, {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify(trainData)
        });
        
        const data = await response.json();
        
        if (data.status === 0) {
            closeModal(addTrainModal);
            showNotification('Train added successfully!', 'success');
            document.getElementById('add-train-form').reset();
        } else {
            showNotification('Failed to add train. Please check the information.', 'error');
        }
    } catch (error) {
        showNotification('Network error. Please try again.', 'error');
    }
}

async function cleanSystem() {
    if (!confirm('Are you sure you want to clean the system? This will remove all data.')) return;
    
    try {
        const response = await fetch(`${API_BASE}/system/clean`, { method: 'POST' });
        const data = await response.json();
        
        if (data.status === 0) {
            showNotification('System cleaned successfully!', 'success');
        } else {
            showNotification('Failed to clean system.', 'error');
        }
    } catch (error) {
        showNotification('System clean error.', 'error');
    }
}

async function exitSystem() {
    if (!confirm('Are you sure you want to exit the system?')) return;
    
    try {
        await fetch(`${API_BASE}/system/exit`, { method: 'POST' });
        showNotification('System shutdown initiated.', 'info');
    } catch (error) {
        showNotification('System may have been shut down.', 'info');
    }
}

// Utility Functions
function showNotification(message, type = 'info') {
    notification.textContent = message;
    notification.className = `notification ${type} show`;
    
    setTimeout(() => {
        notification.classList.remove('show');
    }, 3000);
}

function setCurrentDate() {
    const today = new Date().toISOString().split('T')[0];
    document.getElementById('travel-date').value = today;
}

// Escape key to close modals
document.addEventListener('keydown', (e) => {
    if (e.key === 'Escape') {
        const openModal = document.querySelector('.modal[style*="block"]');
        if (openModal) {
            closeModal(openModal);
        }
    }
});
