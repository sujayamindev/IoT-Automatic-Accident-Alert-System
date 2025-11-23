# Accident Alert System - Flutter App

IoT-based accident detection and telemetry mobile application.

## Features

- **User Authentication** (Firebase Auth)
- **Real-time GPS Tracking**
- **Live Telemetry Data** (speed, battery, signal)
- **Sensor Monitoring** (fire, smoke, vibration)
- **Trip History**
- **Alert Management**
- **Device Settings Configuration**
- **Emergency Contacts**

## Project Structure

```
lib/
├── main.dart                 # App entry point
├── models/                   # Data models
│   ├── telemetry.dart
│   ├── trip.dart
│   ├── alert.dart
│   └── settings.dart
├── providers/                # State management
│   ├── user_provider.dart
│   ├── telemetry_provider.dart
│   └── settings_provider.dart
├── services/                 # API & business logic
│   ├── api_service.dart
│   ├── auth_service.dart
│   └── telemetry_service.dart
├── screens/                  # UI screens
│   ├── auth/
│   │   ├── login.dart
│   │   └── register.dart
│   ├── home.dart
│   ├── live_tracking.dart
│   ├── history.dart
│   ├── settings.dart
│   └── profile.dart
└── widgets/                  # Reusable components
    ├── common_button.dart
    ├── data_card.dart
    └── sensor_indicator.dart
```

## Setup Instructions

### Quick Start (Mock Data - No Backend Required)

The app is configured to run with **mock data** by default. You can test all features without Firebase or a backend!

```bash
flutter pub get
flutter run
```

**Login:** Use any email and password (e.g., `test@example.com` / `password123`)

### Switching to Real Backend

When your backend is ready, update these constants:

1. **Enable Firebase Authentication** (`lib/providers/user_provider.dart`):
   ```dart
   const bool useMockAuth = false;  // Change to false
   ```

2. **Enable Real API** (`lib/providers/telemetry_provider.dart` & `lib/providers/settings_provider.dart`):
   ```dart
   const bool useMockData = false;  // Change to false
   ```

3. **Configure Backend URL** (`lib/services/api_service.dart`):
   ```dart
   static const String baseUrl = 'https://your-backend-api.com';
   ```

4. **Set up Firebase** (`lib/main.dart`):
   - Change `const bool useFirebase = true;`
   - Run `flutterfire configure`
   - Uncomment Firebase initialization code

## Backend API Endpoints

The app expects the following backend endpoints:

### Authentication
- `POST /register` - Register new user
- `POST /login` - Login user
- `POST /logout` - Logout user

### Telemetry
- `GET /getLiveData` - Get current device telemetry
- `GET /getTripHistory` - Get trip history
- `GET /getTripDetails/:id` - Get specific trip details
- `GET /getAlertsHistory` - Get alert history

### Device Settings
- `GET /getSettings` - Get device settings
- `PUT /updateSettings` - Update device settings
- `POST /addContact` - Add emergency contact
- `PUT /updateContact/:id` - Update emergency contact
- `DELETE /deleteContact/:id` - Delete emergency contact

### Device Management
- `POST /bindDeviceToUser` - Bind device to user account
- `POST /unbindDevice` - Unbind device

## Key Dependencies

- **firebase_core** & **firebase_auth** - Authentication
- **provider** - State management
- **flutter_map** & **latlong2** - Map display
- **http** - API communication
- **shared_preferences** - Local storage
- **intl** - Date formatting

## Development Notes

- Uses Provider for state management
- Firebase Authentication for user management
- OpenStreetMap tiles for mapping (no API key required)
- Token-based API authentication
- Supports both Android and iOS

## TODO Before Production

- [ ] Complete Firebase setup
- [ ] Configure backend API URL
- [ ] Add Firebase options configuration
- [ ] Test all API endpoints
- [ ] Add error handling and retry logic
- [ ] Implement proper token refresh
- [ ] Add loading states and empty states
- [ ] Implement pull-to-refresh on all screens
- [ ] Add analytics and crash reporting
- [ ] Implement push notifications for alerts
- [ ] Add offline support
- [ ] Optimize map performance
- [ ] Add app icons and splash screens

## License

Copyright © 2025 IoT Accident Alert System
