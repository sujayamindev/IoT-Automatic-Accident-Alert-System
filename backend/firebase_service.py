"""
Firebase Service for database operations
"""
import firebase_admin
from firebase_admin import credentials, db, firestore, auth
from datetime import datetime, timedelta
from config import Config
import logging

logger = logging.getLogger(__name__)


class FirebaseService:
    """Handles all Firebase operations"""
    
    def __init__(self):
        """Initialize Firebase Admin SDK"""
        try:
            cred = credentials.Certificate(Config.FIREBASE_CREDENTIALS_PATH)
            firebase_admin.initialize_app(cred, {
                'databaseURL': Config.FIREBASE_DATABASE_URL
            })
            self.db = db
            self.firestore_db = firestore.client()
            logger.info("Firebase initialized successfully")
        except Exception as e:
            logger.error(f"Firebase initialization error: {e}")
            raise
    
    def get_user_id_by_mac(self, mac_address):
        """
        Get user ID associated with a MAC address
        
        Args:
            mac_address: Device MAC address
            
        Returns:
            user_id or None
        """
        try:
            ref = self.db.reference(f'/devices/{mac_address}/userId')
            user_id = ref.get()
            return user_id
        except Exception as e:
            logger.error(f"Error getting user ID for MAC {mac_address}: {e}")
            return None
    
    def bind_device_to_user(self, user_id, mac_address, device_name="My Device"):
        """
        Bind a device MAC address to a user
        
        Args:
            user_id: Firebase user ID
            mac_address: Device MAC address
            device_name: Friendly device name
            
        Returns:
            bool: Success status
        """
        try:
            # Store in /devices/{macAddress}
            device_ref = self.db.reference(f'/devices/{mac_address}')
            device_ref.set({
                'userId': user_id,
                'deviceName': device_name,
                'boundAt': datetime.utcnow().isoformat(),
                'status': 'active'
            })
            
            # Store in /users/{userId}/devices/{macAddress}
            user_device_ref = self.db.reference(f'/users/{user_id}/devices/{mac_address}')
            user_device_ref.set({
                'deviceName': device_name,
                'boundAt': datetime.utcnow().isoformat(),
                'lastSeen': None
            })
            
            logger.info(f"Device {mac_address} bound to user {user_id}")
            return True
        except Exception as e:
            logger.error(f"Error binding device: {e}")
            return False
    
    def unbind_device(self, user_id, mac_address):
        """
        Unbind a device from a user
        
        Args:
            user_id: Firebase user ID
            mac_address: Device MAC address
            
        Returns:
            bool: Success status
        """
        try:
            # Remove from /devices
            device_ref = self.db.reference(f'/devices/{mac_address}')
            device_ref.delete()
            
            # Remove from /users
            user_device_ref = self.db.reference(f'/users/{user_id}/devices/{mac_address}')
            user_device_ref.delete()
            
            logger.info(f"Device {mac_address} unbound from user {user_id}")
            return True
        except Exception as e:
            logger.error(f"Error unbinding device: {e}")
            return False
    
    def store_telemetry(self, user_id, mac_address, telemetry_data):
        """
        Store telemetry data in Firebase RTDB
        
        Args:
            user_id: Firebase user ID
            mac_address: Device MAC address
            telemetry_data: Dictionary with sensor data
            
        Returns:
            bool: Success status
        """
        try:
            timestamp = datetime.utcnow().isoformat()
            telemetry_data['timestamp'] = timestamp
            telemetry_data['deviceId'] = mac_address
            
            # Store latest telemetry in RTDB (will be auto-migrated to Firestore after 1 hour)
            ref = self.db.reference(f'/users/{user_id}/devices/{mac_address}/telemetry/latest')
            ref.set(telemetry_data)
            
            # Store timestamped telemetry (for 1-hour retention)
            timestamp_key = datetime.utcnow().strftime('%Y%m%d_%H%M%S')
            history_ref = self.db.reference(f'/users/{user_id}/devices/{mac_address}/telemetry/history/{timestamp_key}')
            history_ref.set(telemetry_data)
            
            # Update last seen
            device_ref = self.db.reference(f'/users/{user_id}/devices/{mac_address}')
            device_ref.update({'lastSeen': timestamp})
            
            logger.debug(f"Telemetry stored for device {mac_address}")
            return True
        except Exception as e:
            logger.error(f"Error storing telemetry: {e}")
            return False
    
    def get_latest_telemetry(self, user_id, mac_address):
        """
        Get latest telemetry data for a device
        
        Args:
            user_id: Firebase user ID
            mac_address: Device MAC address
            
        Returns:
            Dictionary with telemetry data or None
        """
        try:
            ref = self.db.reference(f'/users/{user_id}/devices/{mac_address}/telemetry/latest')
            data = ref.get()
            return data
        except Exception as e:
            logger.error(f"Error getting telemetry: {e}")
            return None
    
    def get_device_config(self, mac_address):
        """
        Get device configuration
        
        Args:
            mac_address: Device MAC address
            
        Returns:
            Dictionary with device settings
        """
        try:
            user_id = self.get_user_id_by_mac(mac_address)
            if not user_id:
                return None
            
            ref = self.db.reference(f'/users/{user_id}/devices/{mac_address}/settings')
            config = ref.get()
            
            # Default config if none exists
            if not config:
                config = {
                    'uploadInterval': 5,
                    'fireDetectionEnabled': True,
                    'smokeDetectionEnabled': True,
                    'vibrationDetectionEnabled': True,
                    'emergencyContacts': []
                }
            
            # Get emergency contacts
            contacts_ref = self.db.reference(f'/users/{user_id}/devices/{mac_address}/contacts')
            contacts = contacts_ref.get()
            if contacts:
                config['emergencyContacts'] = [contact['phoneNumber'] for contact in contacts.values()]
            
            return config
        except Exception as e:
            logger.error(f"Error getting device config: {e}")
            return None
    
    def update_device_settings(self, user_id, mac_address, settings):
        """
        Update device settings
        
        Args:
            user_id: Firebase user ID
            mac_address: Device MAC address
            settings: Dictionary with settings to update
            
        Returns:
            bool: Success status
        """
        try:
            ref = self.db.reference(f'/users/{user_id}/devices/{mac_address}/settings')
            ref.update(settings)
            logger.info(f"Settings updated for device {mac_address}")
            return True
        except Exception as e:
            logger.error(f"Error updating settings: {e}")
            return False
    
    def add_contact(self, user_id, mac_address, contact_data):
        """
        Add emergency contact
        
        Args:
            user_id: Firebase user ID
            mac_address: Device MAC address
            contact_data: Dictionary with contact info
            
        Returns:
            str: Contact ID or None
        """
        try:
            ref = self.db.reference(f'/users/{user_id}/devices/{mac_address}/contacts')
            new_contact = ref.push(contact_data)
            logger.info(f"Contact added for device {mac_address}")
            return new_contact.key
        except Exception as e:
            logger.error(f"Error adding contact: {e}")
            return None
    
    def update_contact(self, user_id, mac_address, contact_id, contact_data):
        """Update emergency contact"""
        try:
            ref = self.db.reference(f'/users/{user_id}/devices/{mac_address}/contacts/{contact_id}')
            ref.update(contact_data)
            return True
        except Exception as e:
            logger.error(f"Error updating contact: {e}")
            return False
    
    def delete_contact(self, user_id, mac_address, contact_id):
        """Delete emergency contact"""
        try:
            ref = self.db.reference(f'/users/{user_id}/devices/{mac_address}/contacts/{contact_id}')
            ref.delete()
            return True
        except Exception as e:
            logger.error(f"Error deleting contact: {e}")
            return False
    
    def migrate_old_telemetry_to_firestore(self):
        """
        Migrate telemetry data older than 1 hour from RTDB to Firestore
        This should be called periodically by a scheduler
        """
        try:
            cutoff_time = datetime.utcnow() - timedelta(seconds=Config.TELEMETRY_RETENTION_SECONDS)
            cutoff_key = cutoff_time.strftime('%Y%m%d_%H%M%S')
            
            # Get all users
            users_ref = self.db.reference('/users')
            users = users_ref.get()
            
            if not users:
                return
            
            migrated_count = 0
            for user_id, user_data in users.items():
                if 'devices' not in user_data:
                    continue
                
                for mac_address, device_data in user_data['devices'].items():
                    if 'telemetry' not in device_data or 'history' not in device_data['telemetry']:
                        continue
                    
                    history = device_data['telemetry']['history']
                    for timestamp_key, telemetry in history.items():
                        # Check if older than cutoff
                        if timestamp_key < cutoff_key:
                            # Store in Firestore
                            doc_ref = self.firestore_db.collection('telemetry_history') \
                                .document(user_id) \
                                .collection('devices') \
                                .document(mac_address) \
                                .collection('data') \
                                .document(timestamp_key)
                            doc_ref.set(telemetry)
                            
                            # Delete from RTDB
                            old_ref = self.db.reference(
                                f'/users/{user_id}/devices/{mac_address}/telemetry/history/{timestamp_key}'
                            )
                            old_ref.delete()
                            
                            migrated_count += 1
            
            if migrated_count > 0:
                logger.info(f"Migrated {migrated_count} telemetry records to Firestore")
        except Exception as e:
            logger.error(f"Error migrating telemetry: {e}")
    
    def verify_firebase_token(self, id_token):
        """
        Verify Firebase ID token from mobile app
        
        Args:
            id_token: Firebase ID token from Authorization header
            
        Returns:
            Decoded token with user info or None
        """
        try:
            decoded_token = auth.verify_id_token(id_token)
            return decoded_token
        except Exception as e:
            logger.error(f"Token verification error: {e}")
            return None
    
    def get_trip_history(self, user_id, mac_address, page=1, limit=20):
        """
        Get trip history from Firestore (paginated)
        
        Args:
            user_id: Firebase user ID
            mac_address: Device MAC address
            page: Page number
            limit: Items per page
            
        Returns:
            Dictionary with trips and pagination info
        """
        try:
            # This would query trips collection in Firestore
            # For MVP, return empty for now
            return {
                'trips': [],
                'page': page,
                'totalPages': 0
            }
        except Exception as e:
            logger.error(f"Error getting trip history: {e}")
            return None
    
    def get_alerts_history(self, user_id, mac_address, page=1, limit=20):
        """
        Get alerts history from Firestore (paginated)
        
        Args:
            user_id: Firebase user ID
            mac_address: Device MAC address
            page: Page number
            limit: Items per page
            
        Returns:
            Dictionary with alerts and pagination info
        """
        try:
            # Query alerts from Firestore
            collection_ref = self.firestore_db.collection('alerts') \
                .document(user_id) \
                .collection('devices') \
                .document(mac_address) \
                .collection('data') \
                .order_by('timestamp', direction=firestore.Query.DESCENDING) \
                .limit(limit)
            
            docs = collection_ref.stream()
            alerts = [doc.to_dict() for doc in docs]
            
            return {
                'alerts': alerts,
                'page': page,
                'totalPages': 1  # Simplified for MVP
            }
        except Exception as e:
            logger.error(f"Error getting alerts: {e}")
            return {'alerts': [], 'page': page, 'totalPages': 0}
    
    def store_alert(self, user_id, mac_address, alert_data):
        """
        Store alert in Firestore
        
        Args:
            user_id: Firebase user ID
            mac_address: Device MAC address
            alert_data: Alert information
            
        Returns:
            bool: Success status
        """
        try:
            timestamp = datetime.utcnow().isoformat()
            alert_data['timestamp'] = timestamp
            alert_data['deviceId'] = mac_address
            alert_data['acknowledged'] = False
            
            doc_ref = self.firestore_db.collection('alerts') \
                .document(user_id) \
                .collection('devices') \
                .document(mac_address) \
                .collection('data') \
                .document()
            
            doc_ref.set(alert_data)
            logger.info(f"Alert stored for device {mac_address}")
            return True
        except Exception as e:
            logger.error(f"Error storing alert: {e}")
            return False
