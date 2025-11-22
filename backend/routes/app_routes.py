"""
Mobile App API Routes
Endpoints for Flutter mobile app communication
"""
from flask import Blueprint, request, jsonify
from middleware import (
    require_user_auth, 
    validate_settings_data, 
    validate_contact_data
)
from firebase_service import FirebaseService
import logging

logger = logging.getLogger(__name__)

app_bp = Blueprint('app', __name__, url_prefix='/api/app')
firebase_service = None


def init_routes(fb_service):
    """Initialize routes with Firebase service"""
    global firebase_service
    firebase_service = fb_service


@app_bp.route('/bindDevice', methods=['POST'])
@require_user_auth
def bind_device():
    """
    Bind a device MAC address to user account
    
    Request JSON:
    - macAddress: Device MAC address
    - deviceName: Friendly name (optional)
    
    Returns:
    - 200: Success
    - 400: Invalid data
    - 401: Unauthorized
    - 500: Server error
    """
    try:
        data = request.json
        mac_address = data.get('macAddress', '').upper().replace('-', ':')
        device_name = data.get('deviceName', 'My Device')
        
        if not mac_address:
            return jsonify({'error': 'Missing macAddress'}), 400
        
        if len(mac_address) != 17:  # XX:XX:XX:XX:XX:XX
            return jsonify({'error': 'Invalid MAC address format. Expected format: XX:XX:XX:XX:XX:XX'}), 400
        
        # Check if device is already bound to another user
        existing_user = firebase_service.get_user_id_by_mac(mac_address)
        if existing_user and existing_user != request.user_id:
            return jsonify({'error': 'Device is already bound to another account'}), 409
        
        # Bind device to user
        success = firebase_service.bind_device_to_user(
            request.user_id,
            mac_address,
            device_name
        )
        
        if not success:
            return jsonify({'error': 'Failed to bind device'}), 500
        
        return jsonify({
            'status': 'success',
            'message': 'Device bound successfully',
            'macAddress': mac_address,
            'deviceName': device_name
        }), 200
        
    except Exception as e:
        logger.error(f"Error in bindDevice: {e}")
        return jsonify({'error': 'Internal server error'}), 500


@app_bp.route('/unbindDevice', methods=['POST'])
@require_user_auth
def unbind_device():
    """
    Unbind a device from user account
    
    Request JSON:
    - macAddress: Device MAC address
    
    Returns:
    - 200: Success
    - 400: Invalid data
    - 401: Unauthorized
    - 500: Server error
    """
    try:
        data = request.json
        mac_address = data.get('macAddress', '').upper().replace('-', ':')
        
        if not mac_address:
            return jsonify({'error': 'Missing macAddress'}), 400
        
        success = firebase_service.unbind_device(request.user_id, mac_address)
        
        if not success:
            return jsonify({'error': 'Failed to unbind device'}), 500
        
        return jsonify({
            'status': 'success',
            'message': 'Device unbound successfully'
        }), 200
        
    except Exception as e:
        logger.error(f"Error in unbindDevice: {e}")
        return jsonify({'error': 'Internal server error'}), 500


@app_bp.route('/getLiveData', methods=['GET'])
@require_user_auth
def get_live_data():
    """
    Get latest telemetry data for user's device
    
    Query params:
    - macAddress: Device MAC address
    
    Returns:
    - 200: Telemetry data
    - 400: Missing parameters
    - 401: Unauthorized
    - 404: No data found
    - 500: Server error
    """
    try:
        mac_address = request.args.get('macAddress', '').upper().replace('-', ':')
        
        if not mac_address:
            return jsonify({'error': 'Missing macAddress parameter'}), 400
        
        # Get latest telemetry
        telemetry = firebase_service.get_latest_telemetry(request.user_id, mac_address)
        
        if not telemetry:
            return jsonify({'error': 'No telemetry data found'}), 404
        
        return jsonify(telemetry), 200
        
    except Exception as e:
        logger.error(f"Error in getLiveData: {e}")
        return jsonify({'error': 'Internal server error'}), 500


@app_bp.route('/getTripHistory', methods=['GET'])
@require_user_auth
def get_trip_history():
    """
    Get paginated trip history
    
    Query params:
    - macAddress: Device MAC address
    - page: Page number (default: 1)
    - limit: Items per page (default: 20)
    
    Returns:
    - 200: Trip data
    - 400: Invalid parameters
    - 401: Unauthorized
    - 500: Server error
    """
    try:
        mac_address = request.args.get('macAddress', '').upper().replace('-', ':')
        page = int(request.args.get('page', 1))
        limit = int(request.args.get('limit', 20))
        
        if not mac_address:
            return jsonify({'error': 'Missing macAddress parameter'}), 400
        
        # Get trip history from Firestore
        result = firebase_service.get_trip_history(request.user_id, mac_address, page, limit)
        
        return jsonify(result), 200
        
    except Exception as e:
        logger.error(f"Error in getTripHistory: {e}")
        return jsonify({'error': 'Internal server error'}), 500


@app_bp.route('/getAlertsHistory', methods=['GET'])
@require_user_auth
def get_alerts_history():
    """
    Get paginated alerts history
    
    Query params:
    - macAddress: Device MAC address
    - page: Page number (default: 1)
    - limit: Items per page (default: 20)
    
    Returns:
    - 200: Alerts data
    - 400: Invalid parameters
    - 401: Unauthorized
    - 500: Server error
    """
    try:
        mac_address = request.args.get('macAddress', '').upper().replace('-', ':')
        page = int(request.args.get('page', 1))
        limit = int(request.args.get('limit', 20))
        
        if not mac_address:
            return jsonify({'error': 'Missing macAddress parameter'}), 400
        
        # Get alerts from Firestore
        result = firebase_service.get_alerts_history(request.user_id, mac_address, page, limit)
        
        return jsonify(result), 200
        
    except Exception as e:
        logger.error(f"Error in getAlertsHistory: {e}")
        return jsonify({'error': 'Internal server error'}), 500


@app_bp.route('/getSettings', methods=['GET'])
@require_user_auth
def get_settings():
    """
    Get device settings
    
    Query params:
    - macAddress: Device MAC address
    
    Returns:
    - 200: Settings data
    - 400: Invalid parameters
    - 401: Unauthorized
    - 404: Settings not found
    - 500: Server error
    """
    try:
        mac_address = request.args.get('macAddress', '').upper().replace('-', ':')
        
        if not mac_address:
            return jsonify({'error': 'Missing macAddress parameter'}), 400
        
        # Get device config
        config = firebase_service.get_device_config(mac_address)
        
        if not config:
            return jsonify({'error': 'Settings not found'}), 404
        
        return jsonify(config), 200
        
    except Exception as e:
        logger.error(f"Error in getSettings: {e}")
        return jsonify({'error': 'Internal server error'}), 500


@app_bp.route('/updateSettings', methods=['PUT'])
@require_user_auth
def update_settings():
    """
    Update device settings
    
    Request JSON:
    - macAddress: Device MAC address
    - settings: Object with settings to update
    
    Returns:
    - 200: Success
    - 400: Invalid data
    - 401: Unauthorized
    - 500: Server error
    """
    try:
        data = request.json
        mac_address = data.get('macAddress', '').upper().replace('-', ':')
        settings = data.get('settings', {})
        
        if not mac_address:
            return jsonify({'error': 'Missing macAddress'}), 400
        
        # Validate settings
        is_valid, error_msg, cleaned_settings = validate_settings_data(settings)
        if not is_valid:
            return jsonify({'error': error_msg}), 400
        
        # Update settings
        success = firebase_service.update_device_settings(
            request.user_id,
            mac_address,
            cleaned_settings
        )
        
        if not success:
            return jsonify({'error': 'Failed to update settings'}), 500
        
        return jsonify({
            'status': 'success',
            'message': 'Settings updated successfully'
        }), 200
        
    except Exception as e:
        logger.error(f"Error in updateSettings: {e}")
        return jsonify({'error': 'Internal server error'}), 500


@app_bp.route('/addContact', methods=['POST'])
@require_user_auth
def add_contact():
    """
    Add emergency contact
    
    Request JSON:
    - macAddress: Device MAC address
    - contact: Contact object (name, phoneNumber, isPrimary)
    
    Returns:
    - 200: Success with contact ID
    - 400: Invalid data
    - 401: Unauthorized
    - 500: Server error
    """
    try:
        data = request.json
        mac_address = data.get('macAddress', '').upper().replace('-', ':')
        contact = data.get('contact', {})
        
        if not mac_address:
            return jsonify({'error': 'Missing macAddress'}), 400
        
        # Validate contact
        is_valid, error_msg, cleaned_contact = validate_contact_data(contact)
        if not is_valid:
            return jsonify({'error': error_msg}), 400
        
        # Add contact
        contact_id = firebase_service.add_contact(
            request.user_id,
            mac_address,
            cleaned_contact
        )
        
        if not contact_id:
            return jsonify({'error': 'Failed to add contact'}), 500
        
        return jsonify({
            'status': 'success',
            'message': 'Contact added successfully',
            'contactId': contact_id
        }), 200
        
    except Exception as e:
        logger.error(f"Error in addContact: {e}")
        return jsonify({'error': 'Internal server error'}), 500


@app_bp.route('/updateContact/<contact_id>', methods=['PUT'])
@require_user_auth
def update_contact(contact_id):
    """
    Update emergency contact
    
    Request JSON:
    - macAddress: Device MAC address
    - contact: Contact object (name, phoneNumber, isPrimary)
    
    Returns:
    - 200: Success
    - 400: Invalid data
    - 401: Unauthorized
    - 500: Server error
    """
    try:
        data = request.json
        mac_address = data.get('macAddress', '').upper().replace('-', ':')
        contact = data.get('contact', {})
        
        if not mac_address:
            return jsonify({'error': 'Missing macAddress'}), 400
        
        # Validate contact
        is_valid, error_msg, cleaned_contact = validate_contact_data(contact)
        if not is_valid:
            return jsonify({'error': error_msg}), 400
        
        # Update contact
        success = firebase_service.update_contact(
            request.user_id,
            mac_address,
            contact_id,
            cleaned_contact
        )
        
        if not success:
            return jsonify({'error': 'Failed to update contact'}), 500
        
        return jsonify({
            'status': 'success',
            'message': 'Contact updated successfully'
        }), 200
        
    except Exception as e:
        logger.error(f"Error in updateContact: {e}")
        return jsonify({'error': 'Internal server error'}), 500


@app_bp.route('/deleteContact/<contact_id>', methods=['DELETE'])
@require_user_auth
def delete_contact(contact_id):
    """
    Delete emergency contact
    
    Query params:
    - macAddress: Device MAC address
    
    Returns:
    - 200: Success
    - 400: Invalid data
    - 401: Unauthorized
    - 500: Server error
    """
    try:
        mac_address = request.args.get('macAddress', '').upper().replace('-', ':')
        
        if not mac_address:
            return jsonify({'error': 'Missing macAddress parameter'}), 400
        
        # Delete contact
        success = firebase_service.delete_contact(
            request.user_id,
            mac_address,
            contact_id
        )
        
        if not success:
            return jsonify({'error': 'Failed to delete contact'}), 500
        
        return jsonify({
            'status': 'success',
            'message': 'Contact deleted successfully'
        }), 200
        
    except Exception as e:
        logger.error(f"Error in deleteContact: {e}")
        return jsonify({'error': 'Internal server error'}), 500
