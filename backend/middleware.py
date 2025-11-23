"""
Authentication and validation middleware
"""
from functools import wraps
from flask import request, jsonify
from firebase_service import FirebaseService
from config import Config
import logging

logger = logging.getLogger(__name__)
firebase_service = None


def init_middleware(fb_service):
    """Initialize middleware with Firebase service"""
    global firebase_service
    firebase_service = fb_service


def require_device_auth(f):
    """
    Middleware to authenticate device requests using MAC address
    Expects 'deviceId' (MAC address) in request data
    """
    @wraps(f)
    def decorated_function(*args, **kwargs):
        # Get MAC address from form data, JSON, or query params
        mac_address = None
        if request.is_json:
            mac_address = request.json.get('deviceId')
        elif request.form:
            mac_address = request.form.get('deviceId')
        elif request.args:
            mac_address = request.args.get('deviceId')
        
        if not mac_address:
            return jsonify({'error': 'Missing deviceId (MAC address)'}), 400
        
        # Validate MAC address format (basic check)
        mac_address = mac_address.upper().replace('-', ':')
        if len(mac_address) != 17:  # XX:XX:XX:XX:XX:XX
            return jsonify({'error': 'Invalid MAC address format'}), 400
        
        # Check if device is bound to a user
        user_id = firebase_service.get_user_id_by_mac(mac_address)
        if not user_id:
            return jsonify({'error': 'Device not registered. Please bind device in mobile app.'}), 403
        
        # Optional: Verify device secret key if configured
        if Config.DEVICE_SECRET_KEY:
            device_secret = request.form.get('secret') or request.json.get('secret', '')
            if device_secret != Config.DEVICE_SECRET_KEY:
                logger.warning(f"Invalid device secret for MAC {mac_address}")
                return jsonify({'error': 'Invalid device credentials'}), 403
        
        # Attach user_id and mac_address to request for use in route handler
        request.user_id = user_id
        request.mac_address = mac_address
        
        return f(*args, **kwargs)
    
    return decorated_function


def require_user_auth(f):
    """
    Middleware to authenticate mobile app requests using Firebase ID token
    Expects 'Authorization: Bearer <token>' header
    """
    @wraps(f)
    def decorated_function(*args, **kwargs):
        auth_header = request.headers.get('Authorization')
        
        if not auth_header or not auth_header.startswith('Bearer '):
            return jsonify({'error': 'Missing or invalid Authorization header'}), 401
        
        id_token = auth_header.split('Bearer ')[1]
        
        # Verify Firebase ID token
        decoded_token = firebase_service.verify_firebase_token(id_token)
        if not decoded_token:
            return jsonify({'error': 'Invalid or expired token'}), 401
        
        # Attach user_id to request
        request.user_id = decoded_token['uid']
        request.user_email = decoded_token.get('email')
        
        return f(*args, **kwargs)
    
    return decorated_function


def validate_telemetry_data(data):
    """
    Validate telemetry data from device
    
    Args:
        data: Dictionary with telemetry fields
        
    Returns:
        tuple: (is_valid, error_message, cleaned_data)
    """
    try:
        # Required fields
        required_fields = ['lat', 'lng', 'spd', 'fire', 'smoke', 'vib']
        for field in required_fields:
            if field not in data:
                return False, f"Missing required field: {field}", None
        
        # Clean and convert data
        cleaned = {
            'latitude': float(data['lat']),
            'longitude': float(data['lng']),
            'speed': float(data['spd']),
            'fireDetected': bool(int(data['fire'])),
            'smokeDetected': bool(int(data['smoke'])),
            'vibrationDetected': bool(int(data['vib'])),
            'batteryPercentage': int(data.get('bat', 0)),
            'gsmSignalLevel': int(data.get('gsm', 0))
        }
        
        # Validate ranges
        if not (-90 <= cleaned['latitude'] <= 90):
            return False, "Invalid latitude", None
        if not (-180 <= cleaned['longitude'] <= 180):
            return False, "Invalid longitude", None
        if not (0 <= cleaned['speed'] <= 300):
            return False, "Invalid speed", None
        if not (0 <= cleaned['batteryPercentage'] <= 100):
            cleaned['batteryPercentage'] = max(0, min(100, cleaned['batteryPercentage']))
        if not (0 <= cleaned['gsmSignalLevel'] <= 5):
            cleaned['gsmSignalLevel'] = max(0, min(5, cleaned['gsmSignalLevel']))
        
        return True, None, cleaned
    except (ValueError, TypeError) as e:
        return False, f"Data validation error: {str(e)}", None


def validate_settings_data(data):
    """
    Validate device settings update
    
    Args:
        data: Dictionary with settings fields
        
    Returns:
        tuple: (is_valid, error_message, cleaned_data)
    """
    try:
        cleaned = {}
        
        if 'uploadInterval' in data:
            interval = int(data['uploadInterval'])
            if interval not in [3, 5, 10, 15, 30]:
                return False, "Invalid upload interval. Must be 3, 5, 10, 15, or 30 seconds", None
            cleaned['uploadInterval'] = interval
        
        if 'fireDetectionEnabled' in data:
            cleaned['fireDetectionEnabled'] = bool(data['fireDetectionEnabled'])
        
        if 'smokeDetectionEnabled' in data:
            cleaned['smokeDetectionEnabled'] = bool(data['smokeDetectionEnabled'])
        
        if 'vibrationDetectionEnabled' in data:
            cleaned['vibrationDetectionEnabled'] = bool(data['vibrationDetectionEnabled'])
        
        if 'deviceName' in data:
            name = str(data['deviceName']).strip()
            if len(name) < 1 or len(name) > 50:
                return False, "Device name must be 1-50 characters", None
            cleaned['deviceName'] = name
        
        return True, None, cleaned
    except (ValueError, TypeError) as e:
        return False, f"Settings validation error: {str(e)}", None


def validate_contact_data(data):
    """
    Validate emergency contact data
    
    Args:
        data: Dictionary with contact fields
        
    Returns:
        tuple: (is_valid, error_message, cleaned_data)
    """
    try:
        required_fields = ['name', 'phoneNumber']
        for field in required_fields:
            if field not in data:
                return False, f"Missing required field: {field}", None
        
        name = str(data['name']).strip()
        phone = str(data['phoneNumber']).strip()
        
        if len(name) < 1 or len(name) > 50:
            return False, "Contact name must be 1-50 characters", None
        
        if len(phone) < 10 or len(phone) > 20:
            return False, "Invalid phone number", None
        
        cleaned = {
            'name': name,
            'phoneNumber': phone,
            'isPrimary': bool(data.get('isPrimary', False))
        }
        
        return True, None, cleaned
    except (ValueError, TypeError) as e:
        return False, f"Contact validation error: {str(e)}", None
