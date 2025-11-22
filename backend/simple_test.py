"""
Simple Device Test - Test uploading telemetry data
"""
import requests

# Step 1: Bind device first
print("Step 1: Binding device to user...")
bind_response = requests.post(
    "http://localhost:5000/api/app/bindDevice",
    headers={
        "Authorization": "Bearer test_token_abc",
        "Content-Type": "application/json"
    },
    json={
        "macAddress": "11:22:33:44:55:66",
        "deviceName": "My Test Device"
    }
)
print(f"Status: {bind_response.status_code}")
print(f"Response: {bind_response.json()}\n")

# Step 2: Upload telemetry (as JSON for testing - ESP32 can also send JSON)
print("Step 2: Uploading telemetry data...")
telemetry_data = {
    'deviceId': '11:22:33:44:55:66',
    'lat': '6.927079',
    'lng': '79.861244',
    'spd': '45.5',
    'fire': '1',
    'smoke': '0',
    'vib': '1',
    'bat': '85',
    'gsm': '4'
}
upload_response = requests.post(
    "http://localhost:5000/api/device/uploadTelemetry",
    json=telemetry_data,  # Using JSON (ESP32 HTTPClient supports this)
    headers={"Content-Type": "application/json"}
)
print(f"Status: {upload_response.status_code}")
print(f"Response: {upload_response.json()}\n")

# Step 3: Fetch device config
print("Step 3: Fetching device configuration...")
config_response = requests.get(
    "http://localhost:5000/api/device/fetchConfig",
    params={'deviceId': '11:22:33:44:55:66'}
)
print(f"Status: {config_response.status_code}")
print(f"Response: {config_response.json()}\n")

# Step 4: Get live data (mobile app view)
print("Step 4: Getting live data from mobile app...")
live_data_response = requests.get(
    "http://localhost:5000/api/app/getLiveData",
    headers={"Authorization": "Bearer test_token_abc"},
    params={'macAddress': '11:22:33:44:55:66'}
)
print(f"Status: {live_data_response.status_code}")
print(f"Response: {live_data_response.json()}\n")

print("✅ Full device lifecycle test complete!")
