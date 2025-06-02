# Railway Ticket System - Web Interface

This directory contains a web interface for the Railway Ticket System.

## Features

### User Features
- **User Authentication**: Login and registration system
- **Ticket Search**: Search for direct train tickets between stations
- **Transfer Search**: Find routes with transfers
- **Ticket Booking**: Purchase tickets with queue support
- **Order Management**: View and refund tickets
- **Profile Management**: View user profile information

### Admin Features
- **Train Management**: Add new trains to the system
- **User Management**: Manage user accounts
- **System Administration**: Clean system data and shutdown

## File Structure

```
web/
├── index.html      # Main HTML file with complete UI structure
├── styles.css      # Modern CSS with responsive design
├── script.js       # JavaScript for API interactions and UI logic
└── README.md       # This file
```

## API Integration

The web interface integrates with the following API endpoints:

### User Management
- `POST /users` - Register new user
- `POST /sessions` - User login
- `DELETE /sessions/{username}` - User logout
- `GET /users/{username}` - Get user profile
- `PUT /users/{username}` - Update user profile

### Train Management
- `POST /trains` - Add new train
- `DELETE /trains/{trainId}` - Delete train
- `POST /trains/{trainId}/release` - Release train
- `GET /trains/{trainId}` - Query train details

### Ticket Operations
- `GET /tickets` - Search direct tickets
- `GET /transfer` - Search transfer routes
- `POST /orders/{username}/buy` - Buy tickets
- `GET /orders/{username}` - Query user orders
- `DELETE /orders/{username}` - Refund tickets

### System Administration
- `POST /system/clean` - Clean system data
- `POST /system/exit` - Shutdown system