"""
Mock Firebase Service for Testing
Use this when you don't have firebase-service-account.json yet
"""
import logging
from datetime import datetime

logger = logging.getLogger(__name__)

class MockFirebaseService:
    """Mock Firebase service for testing without credentials"""
    
    def __init__(self):
        logger.warning("Using MOCK Firebase service - data will not persist!")
        self.mock_data = {
            'devices': {},
            'users': {}
        }
    
    def get_user_id_by_mac(self, mac_address):
        """Get user ID by MAC address"""
        device = self.mock_data['devices'].get(mac_address)
        if device:
            return device.get('userId')
        return None
    
    def bind_device_to_user(self, user_id, mac_address, device_name="My Device"):
        """Bind device to user"""
        self.mock_data['devices'][mac_address] = {
            'userId': user_id,
            'deviceName': device_name,
            'boundAt': datetime.utcnow().isoformat(),
            'status': 'active'
        }
        
        if user_id not in self.mock_data['users']:
            self.mock_data['users'][user_id] = {'devices': {}}
        
        self.mock_data['users'][user_id]['devices'][mac_address] = {
            'deviceName': device_name,
            'boundAt': datetime.utcnow().isoformat(),
            'lastSeen': None
        }
        
        logger.info(f"[MOCK] Device {mac_address} bound to user {user_id}")
        return True
    
    def unbind_device(self, user_id, mac_address):
        """Unbind device"""
        if mac_address in self.mock_data['devices']:
            del self.mock_data['devices'][mac_address]
        
        if user_id in self.mock_data['users'] and 'devices' in self.mock_data['users'][user_id]:
            if mac_address in self.mock_data['users'][user_id]['devices']:
                del self.mock_data['users'][user_id]['devices'][mac_address]
        
        logger.info(f"[MOCK] Device {mac_address} unbound")
        return True
    
    def store_telemetry(self, user_id, mac_address, telemetry_data):
        """Store telemetry"""
        logger.info(f"[MOCK] Storing telemetry for {mac_address}: {telemetry_data}")
        return True
    
    def get_latest_telemetry(self, user_id, mac_address):
        """Get latest telemetry"""
        logger.info(f"[MOCK] Getting telemetry for {mac_address}")
        return {
            'latitude': 6.927079,
            'longitude': 79.861244,
            'speed': 45.5,
            'fireDetected': False,
            'smokeDetected': False,
            'accel_x': 1000,
            'accel_y': 500,
            'accel_z': 16000,
            'gyro_x': 0,
            'gyro_y': 0,
            'gyro_z': 0,
            'temperature': 28.5,
            'humidity': 65.0,
            'gsmSignalLevel': 4,
            'timestamp': datetime.utcnow().isoformat()
        }
    
    def get_device_config(self, mac_address):
        """Get device config"""
        logger.info(f"[MOCK] Getting config for {mac_address}")
        return {
            'uploadInterval': 5,
            'fireDetectionEnabled': True,
            'smokeDetectionEnabled': True,
            'vibrationDetectionEnabled': True,
            'emergencyContacts': ['+94771234567']
        }
    
    def update_device_settings(self, user_id, mac_address, settings):
        """Update settings"""
        logger.info(f"[MOCK] Updating settings for {mac_address}: {settings}")
        return True
    
    def add_contact(self, user_id, mac_address, contact_data):
        """Add contact"""
        logger.info(f"[MOCK] Adding contact for {mac_address}: {contact_data}")
        return "mock_contact_id_123"
    
    def update_contact(self, user_id, mac_address, contact_id, contact_data):
        """Update contact"""
        logger.info(f"[MOCK] Updating contact {contact_id}")
        return True
    
    def delete_contact(self, user_id, mac_address, contact_id):
        """Delete contact"""
        logger.info(f"[MOCK] Deleting contact {contact_id}")
        return True
    
    def migrate_old_telemetry_to_firestore(self):
        """Mock migration"""
        logger.info("[MOCK] Running telemetry migration (no-op)")
    
    def verify_firebase_token(self, id_token):
        """Mock token verification"""
        logger.info("[MOCK] Verifying Firebase token")
        # For testing, accept any token that starts with "test_"
        if id_token.startswith("test_"):
            return {
                'uid': 'test_user_123',
                'email': 'test@example.com'
            }
        return None
    
    def get_trip_history(self, user_id, mac_address, page=1, limit=20):
        """Mock trip history"""
        return {
            'trips': [],
            'page': page,
            'totalPages': 0
        }
    
    def get_alerts_history(self, user_id, mac_address, page=1, limit=20):
        """Mock alerts history"""
        return {
            'alerts': [],
            'page': page,
            'totalPages': 0
        }
    
    def store_alert(self, user_id, mac_address, alert_data):
        """Mock store alert"""
        logger.info(f"[MOCK] Storing alert for {mac_address}: {alert_data}")
        return True
