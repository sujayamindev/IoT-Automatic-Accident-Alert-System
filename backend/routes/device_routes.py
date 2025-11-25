"""
Device API Routes
Endpoints for ESP32 + SIM800L device communication
"""
from flask import Blueprint, request, jsonify
from middleware import require_device_auth, validate_telemetry_data
from firebase_service import FirebaseService
import logging

logger = logging.getLogger(__name__)

device_bp = Blueprint('device', __name__, url_prefix='/api/device')
firebase_service = None


def init_routes(fb_service):
    """Initialize routes with Firebase service"""
    global firebase_service
    firebase_service = fb_service


@device_bp.route('/uploadTelemetry', methods=['POST'])
@require_device_auth
def upload_telemetry():
    """
    Upload telemetry data from ESP32 + SIM800L
    
    Expected URL-encoded or JSON data:
    - deviceId: MAC address (validated by middleware)
    - lat: Latitude
    - lng: Longitude
    - spd: Speed (km/h)
    - fire: Fire detected (0 or 1)
    - smoke: Smoke detected (0 or 1)
    - accel_x, accel_y, accel_z: Accelerometer data (optional)
    - gyro_x, gyro_y, gyro_z: Gyroscope data (optional)
    - temp: Temperature (optional)
    - humidity: Humidity (optional)
    - coLevel: CO sensor level (optional)
    - flameLevel: Flame sensor level (optional)
    - gpsTimestamp: GPS timestamp in ISO 8601 format (optional)
    - gsm: GSM signal level (optional, 0-5)
    
    Returns:
    - 200: Success
    - 400: Invalid data
    - 403: Device not authenticated
    - 500: Server error
    """
    try:
        # Get data from form (URL-encoded) or JSON
        if request.is_json:
            data = request.json
        else:
            data = request.form.to_dict()
        
        # Validate telemetry data
        is_valid, error_msg, cleaned_data = validate_telemetry_data(data)
        if not is_valid:
            logger.warning(f"Invalid telemetry data: {error_msg}")
            return jsonify({'error': error_msg}), 400
        
        # Store in Firebase (user_id and mac_address are set by middleware)
        success = firebase_service.store_telemetry(
            request.user_id,
            request.mac_address,
            cleaned_data
        )
        
        if not success:
            return jsonify({'error': 'Failed to store telemetry'}), 500
        
        # Check for alert conditions and store alerts
        alert_triggered = False
        if cleaned_data['fireDetected']:
            firebase_service.store_alert(request.user_id, request.mac_address, {
                'type': 'fire',
                'latitude': cleaned_data['latitude'],
                'longitude': cleaned_data['longitude'],
                'description': 'Fire detected'
            })
            alert_triggered = True
        
        if cleaned_data['smokeDetected']:
            firebase_service.store_alert(request.user_id, request.mac_address, {
                'type': 'smoke',
                'latitude': cleaned_data['latitude'],
                'longitude': cleaned_data['longitude'],
                'description': 'Smoke detected'
            })
            alert_triggered = True
        
        # Check for high impact based on MPU data
        impact_threshold = 25000
        if (abs(cleaned_data['accel_x']) > impact_threshold or 
            abs(cleaned_data['accel_y']) > impact_threshold or 
            abs(cleaned_data['accel_z']) > impact_threshold):
            firebase_service.store_alert(request.user_id, request.mac_address, {
                'type': 'impact',
                'latitude': cleaned_data['latitude'],
                'longitude': cleaned_data['longitude'],
                'description': 'High impact detected',
                'accel_x': cleaned_data['accel_x'],
                'accel_y': cleaned_data['accel_y'],
                'accel_z': cleaned_data['accel_z']
            })
            alert_triggered = True
        
        response = {
            'status': 'success',
            'message': 'Telemetry uploaded',
            'alert': alert_triggered
        }
        
        logger.debug(f"Telemetry uploaded for device {request.mac_address}")
        return jsonify(response), 200
        
    except Exception as e:
        logger.error(f"Error in uploadTelemetry: {e}")
        return jsonify({'error': 'Internal server error'}), 500


@device_bp.route('/fetchConfig', methods=['GET'])
@require_device_auth
def fetch_config():
    """
    Fetch device configuration
    Device polls this endpoint to get updated settings
    
    Query params:
    - deviceId: MAC address
    
    Returns:
    - 200: Configuration data
    - 403: Device not authenticated
    - 404: Configuration not found
    - 500: Server error
    """
    try:
        # Get device configuration from Firebase
        config = firebase_service.get_device_config(request.mac_address)
        
        if not config:
            return jsonify({'error': 'Configuration not found'}), 404
        
        # Format response for device (simplified)
        device_config = {
            'uploadInterval': config.get('uploadInterval', 5),
            'fireDetectionEnabled': int(config.get('fireDetectionEnabled', True)),
            'smokeDetectionEnabled': int(config.get('smokeDetectionEnabled', True)),
            'vibrationDetectionEnabled': int(config.get('vibrationDetectionEnabled', True)),
            'emergencyContacts': ','.join(config.get('emergencyContacts', []))  # Comma-separated string
        }
        
        logger.debug(f"Config fetched for device {request.mac_address}")
        return jsonify(device_config), 200
        
    except Exception as e:
        logger.error(f"Error in fetchConfig: {e}")
        return jsonify({'error': 'Internal server error'}), 500


@device_bp.route('/ping', methods=['GET', 'POST'])
def ping():
    """
    Simple ping endpoint for device health check
    No authentication required
    """
    return jsonify({'status': 'ok', 'message': 'Server is running'}), 200
