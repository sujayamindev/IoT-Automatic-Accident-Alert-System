# IoT Accident Alert System - Backend

Python Flask backend server that bridges ESP32 + SIM800L devices to Firebase, enabling real-time telemetry data collection and mobile app integration.

## 🎯 Purpose

This backend serves as an HTTP-to-HTTPS bridge because:
- **SIM800L limitation**: Only supports HTTP (not HTTPS)
- **Firebase requirement**: Requires HTTPS and authentication tokens
- **Data validation**: Validates and formats sensor data before storing
- **User-device mapping**: Links devices to user accounts via MAC address

## 🏗️ Architecture

```
ESP32 + SIM800L (HTTP) → Backend Server → Firebase (HTTPS)
                                            ↓
Mobile App (Flutter) ← Firebase (Real-time updates)
```

## 📋 Features

### Device Endpoints (for ESP32 + SIM800L)
- `POST /api/device/uploadTelemetry` - Upload sensor data
- `GET /api/device/fetchConfig` - Fetch device settings
- `GET /api/device/ping` - Health check

### Mobile App Endpoints (for Flutter app)
- `POST /api/app/bindDevice` - Bind device MAC to user account
- `POST /api/app/unbindDevice` - Unbind device
- `GET /api/app/getLiveData` - Get latest telemetry
- `GET /api/app/getTripHistory` - Get trip history (paginated)
- `GET /api/app/getAlertsHistory` - Get alerts history (paginated)
- `GET /api/app/getSettings` - Get device settings
- `PUT /api/app/updateSettings` - Update device settings
- `POST /api/app/addContact` - Add emergency contact
- `PUT /api/app/updateContact/<id>` - Update contact
- `DELETE /api/app/deleteContact/<id>` - Delete contact

## 🚀 Setup Instructions

### Prerequisites
- Python 3.11 or higher
- Firebase project with Realtime Database and Firestore
- Firebase service account JSON credentials

### 1. Install Dependencies

```bash
cd backend
pip install -r requirements.txt
```

### 2. Firebase Setup

1. Go to [Firebase Console](https://console.firebase.google.com/)
2. Select your project: `iot-project-fa54e`
3. Go to **Project Settings** → **Service Accounts**
4. Click **Generate New Private Key**
5. Save the JSON file as `firebase-service-account.json` in the `backend/` folder

### 3. Environment Configuration

Copy the example environment file:
```bash
cp .env.example .env
```

Edit `.env` and configure:
```env
FIREBASE_PROJECT_ID=iot-project-fa54e
FIREBASE_DATABASE_URL=https://iot-project-fa54e-default-rtdb.asia-southeast1.firebasedatabase.app
FIREBASE_CREDENTIALS_PATH=./firebase-service-account.json

PORT=5000
FLASK_ENV=development
SECRET_KEY=your-random-secret-key-here

# Optional: Add device secret for extra security
DEVICE_SECRET_KEY=your-device-secret

# Data retention (1 hour = 3600 seconds)
TELEMETRY_RETENTION_SECONDS=3600
```

### 4. Run Locally

```bash
python app.py
```

Server will start on `http://localhost:5000`

### 5. Test Endpoints

**Test device ping:**
```bash
curl http://localhost:5000/api/device/ping
```

**Test root endpoint:**
```bash
curl http://localhost:5000/
```

## 📡 API Usage Examples

### Device: Upload Telemetry (URL-encoded)

```bash
curl -X POST http://localhost:5000/api/device/uploadTelemetry \
  -H "Content-Type: application/x-www-form-urlencoded" \
  -d "deviceId=AA:BB:CC:DD:EE:FF&lat=6.927079&lng=79.861244&spd=45.5&fire=0&smoke=0&vib=0&bat=87&gsm=4"
```

### Device: Fetch Configuration

```bash
curl "http://localhost:5000/api/device/fetchConfig?deviceId=AA:BB:CC:DD:EE:FF"
```

### App: Bind Device

```bash
curl -X POST http://localhost:5000/api/app/bindDevice \
  -H "Content-Type: application/json" \
  -H "Authorization: Bearer YOUR_FIREBASE_ID_TOKEN" \
  -d '{"macAddress":"AA:BB:CC:DD:EE:FF","deviceName":"My Car"}'
```

### App: Get Live Data

```bash
curl "http://localhost:5000/api/app/getLiveData?macAddress=AA:BB:CC:DD:EE:FF" \
  -H "Authorization: Bearer YOUR_FIREBASE_ID_TOKEN"
```

## 🌐 Deployment to Render

### 1. Create `render.yaml`

Already included in the project.

### 2. Push to GitHub

```bash
git add .
git commit -m "Add Python Flask backend"
git push origin main
```

### 3. Deploy on Render

1. Go to [Render Dashboard](https://dashboard.render.com/)
2. Click **New** → **Web Service**
3. Connect your GitHub repository
4. Configure:
   - **Name**: `iot-accident-alert-backend`
   - **Environment**: `Python 3`
   - **Build Command**: `pip install -r backend/requirements.txt`
   - **Start Command**: `cd backend && gunicorn app:app`
   - **Instance Type**: Free or Starter

### 4. Add Environment Variables on Render

In Render dashboard, add these environment variables:
- `FIREBASE_PROJECT_ID`
- `FIREBASE_DATABASE_URL`
- `SECRET_KEY`
- `FLASK_ENV=production`
- `PORT=10000` (Render default)

### 5. Upload Firebase Credentials

**Option A: Environment Variable (Recommended)**
- Copy entire content of `firebase-service-account.json`
- Create environment variable `FIREBASE_CREDENTIALS_JSON` with JSON content
- Update `firebase_service.py` to read from env variable

**Option B: Secret File**
- Use Render's secret files feature
- Upload `firebase-service-account.json` as a secret file

### 6. Deploy

Click **Deploy** and wait for deployment to complete.

Your backend will be available at:
```
https://iot-accident-alert-backend.onrender.com
```

## 📊 Firebase Data Structure

### Realtime Database (RTDB)
```
/devices
  /{macAddress}
    /userId: "user123"
    /deviceName: "My Car"
    /boundAt: "2025-11-22T..."
    /status: "active"

/users
  /{userId}
    /devices
      /{macAddress}
        /deviceName: "My Car"
        /lastSeen: "2025-11-22T..."
        /telemetry
          /latest
            /latitude: 6.927079
            /longitude: 79.861244
            /speed: 45.5
            /fireDetected: false
            /smokeDetected: false
            /vibrationDetected: false
            /batteryPercentage: 87
            /gsmSignalLevel: 4
            /timestamp: "2025-11-22T..."
          /history
            /20251122_143000: {...}
            /20251122_143005: {...}
        /settings
          /uploadInterval: 5
          /fireDetectionEnabled: true
          /smokeDetectionEnabled: true
          /vibrationDetectionEnabled: true
        /contacts
          /{contactId}
            /name: "John Doe"
            /phoneNumber: "+94771234567"
            /isPrimary: true
```

### Firestore
```
/telemetry_history
  /{userId}
    /devices
      /{macAddress}
        /data
          /{timestamp}: {...telemetry...}

/alerts
  /{userId}
    /devices
      /{macAddress}
        /data
          /{alertId}
            /type: "fire" | "smoke" | "vibration" | "accident"
            /timestamp: "2025-11-22T..."
            /latitude: 6.927079
            /longitude: 79.861244
            /description: "..."
            /acknowledged: false
```

## 🔄 Data Flow

### 1. Device Registration
1. User opens mobile app
2. User enters device MAC address
3. App calls `/api/app/bindDevice` with Firebase auth token
4. Backend creates mapping in `/devices/{mac}` and `/users/{userId}/devices/{mac}`

### 2. Telemetry Upload
1. ESP32 collects sensor data every 5 seconds
2. SIM800L sends HTTP POST to `/api/device/uploadTelemetry`
3. Backend validates MAC address and data
4. Backend stores in Firebase RTDB under user's device path
5. If alert detected, also stores in Firestore `/alerts` collection
6. Mobile app reads real-time updates from Firebase

### 3. Data Retention
1. Background scheduler runs every 15 minutes
2. Moves telemetry older than 1 hour from RTDB to Firestore
3. Keeps RTDB lightweight for real-time updates
4. Historical data remains in Firestore for queries

## 🔐 Security

### Device Authentication
- Devices identified by MAC address
- Optional device secret key for extra security
- Must be bound to user before accepting data

### Mobile App Authentication
- Uses Firebase Authentication (email/password)
- All app endpoints require valid Firebase ID token
- Token verified on every request

### Firebase Security Rules (Recommended)

**Realtime Database:**
```json
{
  "rules": {
    "users": {
      "$uid": {
        ".read": "$uid === auth.uid",
        ".write": "$uid === auth.uid"
      }
    },
    "devices": {
      ".read": false,
      ".write": false
    }
  }
}
```

**Firestore:**
```javascript
rules_version = '2';
service cloud.firestore {
  match /databases/{database}/documents {
    match /telemetry_history/{userId}/{document=**} {
      allow read: if request.auth.uid == userId;
      allow write: if false;
    }
    match /alerts/{userId}/{document=**} {
      allow read: if request.auth.uid == userId;
      allow write: if false;
    }
  }
}
```

## 🐛 Troubleshooting

### Firebase Initialization Error
- Check `firebase-service-account.json` exists
- Verify JSON is valid
- Ensure environment variables are set correctly

### Device Authentication Failed
- Verify MAC address format: `XX:XX:XX:XX:XX:XX`
- Device must be bound to user via mobile app first
- Check if device secret matches (if enabled)

### CORS Errors
- Update `CORS_ORIGINS` in `config.py` for production
- Add your mobile app domain to allowed origins

### Data Not Appearing in Firebase
- Check Firebase console for data
- Verify user_id and mac_address are correct
- Check server logs for errors

## 📝 Logging

View logs on Render:
```bash
render logs -s your-service-name
```

Log levels:
- `INFO`: General operations
- `DEBUG`: Detailed telemetry operations
- `WARNING`: Invalid data or auth failures
- `ERROR`: Server errors

## 🔧 Development

### Project Structure
```
backend/
├── app.py                  # Main Flask application
├── config.py               # Configuration
├── firebase_service.py     # Firebase operations
├── middleware.py           # Authentication & validation
├── routes/
│   ├── device_routes.py    # Device endpoints
│   └── app_routes.py       # Mobile app endpoints
├── requirements.txt        # Python dependencies
├── .env.example            # Environment template
├── .gitignore
├── render.yaml             # Render deployment config
└── README.md
```

### Adding New Endpoints

1. Define route in `routes/device_routes.py` or `routes/app_routes.py`
2. Add authentication decorator (`@require_device_auth` or `@require_user_auth`)
3. Implement validation if needed
4. Add Firebase operations in `firebase_service.py`
5. Update this README with endpoint documentation

## 📞 Support

For issues or questions:
1. Check troubleshooting section
2. Review Firebase console for data
3. Check Render logs for errors
4. Verify environment variables are set

## 📄 License

This project is part of an IoT coursework assignment.
