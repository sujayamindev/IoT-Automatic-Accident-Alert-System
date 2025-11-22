"""
Test Script for Backend API
Tests endpoints without requiring Firebase credentials
"""
import requests
import json

BASE_URL = "http://localhost:5000"

def print_section(title):
    print("\n" + "="*60)
    print(f" {title}")
    print("="*60)

def test_root():
    """Test root endpoint"""
    print_section("Testing Root Endpoint")
    try:
        response = requests.get(f"{BASE_URL}/")
        print(f"Status: {response.status_code}")
        print(f"Response: {json.dumps(response.json(), indent=2)}")
        return response.status_code == 200
    except Exception as e:
        print(f"❌ Error: {e}")
        return False

def test_health():
    """Test health endpoint"""
    print_section("Testing Health Endpoint")
    try:
        response = requests.get(f"{BASE_URL}/health")
        print(f"Status: {response.status_code}")
        print(f"Response: {json.dumps(response.json(), indent=2)}")
        return response.status_code == 200
    except Exception as e:
        print(f"❌ Error: {e}")
        return False

def test_device_ping():
    """Test device ping endpoint"""
    print_section("Testing Device Ping")
    try:
        response = requests.get(f"{BASE_URL}/api/device/ping")
        print(f"Status: {response.status_code}")
        print(f"Response: {json.dumps(response.json(), indent=2)}")
        return response.status_code == 200
    except Exception as e:
        print(f"❌ Error: {e}")
        return False

def test_upload_telemetry_without_auth():
    """Test upload telemetry without device ID (should fail)"""
    print_section("Testing Upload Telemetry (No Auth - Should Fail)")
    try:
        data = {
            'lat': '6.927079',
            'lng': '79.861244',
            'spd': '45.5',
            'fire': '0',
            'smoke': '0',
            'vib': '0',
            'bat': '87',
            'gsm': '4'
        }
        response = requests.post(f"{BASE_URL}/api/device/uploadTelemetry", data=data)
        print(f"Status: {response.status_code}")
        print(f"Response: {json.dumps(response.json(), indent=2)}")
        return response.status_code == 400  # Should fail with 400
    except Exception as e:
        print(f"❌ Error: {e}")
        return False

def test_upload_telemetry_unregistered():
    """Test upload telemetry with unregistered device (should fail)"""
    print_section("Testing Upload Telemetry (Unregistered Device - Should Fail)")
    try:
        data = {
            'deviceId': 'AA:BB:CC:DD:EE:FF',
            'lat': '6.927079',
            'lng': '79.861244',
            'spd': '45.5',
            'fire': '0',
            'smoke': '0',
            'vib': '0',
            'bat': '87',
            'gsm': '4'
        }
        response = requests.post(f"{BASE_URL}/api/device/uploadTelemetry", data=data)
        print(f"Status: {response.status_code}")
        print(f"Response: {json.dumps(response.json(), indent=2)}")
        return response.status_code == 403  # Should fail with 403 (not registered)
    except Exception as e:
        print(f"❌ Error: {e}")
        return False

def test_fetch_config_unregistered():
    """Test fetch config for unregistered device (should fail)"""
    print_section("Testing Fetch Config (Unregistered - Should Fail)")
    try:
        response = requests.get(f"{BASE_URL}/api/device/fetchConfig?deviceId=AA:BB:CC:DD:EE:FF")
        print(f"Status: {response.status_code}")
        print(f"Response: {json.dumps(response.json(), indent=2)}")
        return response.status_code == 403  # Should fail
    except Exception as e:
        print(f"❌ Error: {e}")
        return False

def test_bind_device_without_auth():
    """Test bind device without Firebase token (should fail)"""
    print_section("Testing Bind Device (No Auth - Should Fail)")
    try:
        data = {
            'macAddress': 'AA:BB:CC:DD:EE:FF',
            'deviceName': 'Test Device'
        }
        response = requests.post(
            f"{BASE_URL}/api/app/bindDevice",
            json=data
        )
        print(f"Status: {response.status_code}")
        print(f"Response: {json.dumps(response.json(), indent=2)}")
        return response.status_code == 401  # Should fail with 401
    except Exception as e:
        print(f"❌ Error: {e}")
        return False

def run_tests():
    """Run all tests"""
    print("\n" + "🚀 BACKEND API TESTING SUITE" + "\n")
    print("Make sure the backend server is running on http://localhost:5000")
    print("Run: python app.py")
    
    input("\nPress Enter to start tests...")
    
    results = []
    
    # Basic endpoint tests
    results.append(("Root Endpoint", test_root()))
    results.append(("Health Check", test_health()))
    results.append(("Device Ping", test_device_ping()))
    
    # Device endpoint tests (should fail without proper auth)
    results.append(("Upload Telemetry (No Auth)", test_upload_telemetry_without_auth()))
    results.append(("Upload Telemetry (Unregistered)", test_upload_telemetry_unregistered()))
    results.append(("Fetch Config (Unregistered)", test_fetch_config_unregistered()))
    
    # App endpoint tests (should fail without Firebase token)
    results.append(("Bind Device (No Auth)", test_bind_device_without_auth()))
    
    # Print summary
    print_section("TEST SUMMARY")
    passed = sum(1 for _, result in results if result)
    total = len(results)
    
    for test_name, result in results:
        status = "✅ PASS" if result else "❌ FAIL"
        print(f"{status} - {test_name}")
    
    print(f"\n{passed}/{total} tests passed")
    
    if passed == total:
        print("\n🎉 All tests passed! Backend is working correctly.")
    else:
        print("\n⚠️ Some tests failed. Check the output above for details.")

if __name__ == "__main__":
    run_tests()
