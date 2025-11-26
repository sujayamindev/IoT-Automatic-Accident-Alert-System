"""
Firebase Service for database operations
"""
import firebase_admin
from firebase_admin import credentials, db, firestore, auth
from datetime import datetime, timedelta, timezone
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
        
        Note: Searches through all users to find which user owns this device.
        This is necessary since we removed the /devices path to eliminate duplication.
        """
        try:
            # Query all users to find who owns this device
            users_ref = self.db.reference('/users')
            all_users = users_ref.get()
            
            if not all_users:
                return None
            
            for user_id, user_data in all_users.items():
                if 'devices' in user_data and mac_address in user_data['devices']:
                    return user_id
            
            return None
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
        
        Note: Only stores in /users path. The /devices path is redundant since we can 
        always look up user->device mapping from /users/{userId}/devices/{macAddress}
        """
        try:
            # Store ONLY in /users/{userId}/devices/{macAddress}
            # This eliminates the /devices/{macAddress} duplication
            user_device_ref = self.db.reference(f'/users/{user_id}/devices/{mac_address}')
            # Convert to Sri Lankan time (UTC+5:30)
            bound_time = datetime.now(timezone.utc) + timedelta(hours=5, minutes=30)
            user_device_ref.set({
                'deviceName': device_name,
                'boundAt': bound_time.isoformat(),
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
            # Remove only from /users (no more /devices path)
            user_device_ref = self.db.reference(f'/users/{user_id}/devices/{mac_address}')
            user_device_ref.delete()
            
            logger.info(f"Device {mac_address} unbound from user {user_id}")
            return True
        except Exception as e:
            logger.error(f"Error unbinding device: {e}")
            return False
    
    def store_telemetry_with_sticky_impact(self, user_id, mac_address, telemetry_data):
        """
        Store telemetry data with sticky impact flag logic.
        Once hasHighImpact is set to true, it stays true for 60 seconds even if firmware sends impact=0.
        This prevents the impact indicator from disappearing too quickly on the mobile app.
        
        Args:
            user_id: Firebase user ID
            mac_address: Device MAC address
            telemetry_data: Dictionary with sensor data (includes 'impactDetected' from firmware)
            
        Returns:
            bool: Success status
        """
        try:
            # Convert to Sri Lankan time (UTC+5:30)
            timestamp_utc = datetime.now(timezone.utc)
            timestamp = timestamp_utc + timedelta(hours=5, minutes=30)
            
            # Get previous telemetry to check if impact flag should remain sticky
            ref = self.db.reference(f'/users/{user_id}/devices/{mac_address}/telemetry/latest')
            previous_data = ref.get()
            
            # Determine hasHighImpact with sticky logic
            impact_detected = telemetry_data.get('impactDetected', False)
            has_high_impact = impact_detected  # Start with current detection state
            
            # If not currently detecting impact, check if we should keep it sticky
            if not impact_detected and previous_data:
                # Check if previous telemetry had hasHighImpact=true
                if previous_data.get('hasHighImpact', False):
                    # Check how long ago the impact was detected (keep sticky for 60 seconds)
                    impact_timestamp_str = previous_data.get('impactTimestamp')
                    if impact_timestamp_str:
                        try:
                            impact_timestamp = datetime.fromisoformat(impact_timestamp_str)
                            time_since_impact = (timestamp - impact_timestamp).total_seconds()
                            if time_since_impact < 10:  # Keep sticky for 10 seconds
                                has_high_impact = True
                                logger.info(f"Impact flag still sticky ({time_since_impact:.1f}s since detection)")
                        except Exception as e:
                            logger.warning(f"Error parsing impact timestamp: {e}")
            
            # Set hasHighImpact in telemetry data
            telemetry_data['hasHighImpact'] = has_high_impact
            logger.info(f"Setting hasHighImpact={has_high_impact} for device {mac_address} (impactDetected={impact_detected})")
            
            # If impact is newly detected, record the timestamp
            if impact_detected and (not previous_data or not previous_data.get('hasHighImpact', False)):
                telemetry_data['impactTimestamp'] = timestamp.isoformat()
                logger.info(f"New impact detected, setting timestamp")
            elif has_high_impact and previous_data:
                # Preserve the original impact timestamp while sticky
                telemetry_data['impactTimestamp'] = previous_data.get('impactTimestamp', timestamp.isoformat())
            
            # Call the original store_telemetry method
            return self.store_telemetry(user_id, mac_address, telemetry_data)
            
        except Exception as e:
            logger.error(f"Error in store_telemetry_with_sticky_impact: {e}", exc_info=True)
            return False
    
    def store_telemetry(self, user_id, mac_address, telemetry_data):
        """
        Store telemetry data in Firebase RTDB (latest) and Firestore (history)
        
        Args:
            user_id: Firebase user ID
            mac_address: Device MAC address
            telemetry_data: Dictionary with sensor data
            
        Returns:
            bool: Success status
        """
        try:
            # Convert to Sri Lankan time (UTC+5:30)
            timestamp_utc = datetime.now(timezone.utc)
            timestamp_lk = timestamp_utc + timedelta(hours=5, minutes=30)
            timestamp_iso = timestamp_lk.isoformat()
            telemetry_data['timestamp'] = timestamp_iso
            telemetry_data['deviceId'] = mac_address
            
            logger.info(f"Storing telemetry for user {user_id}, device {mac_address}")
            
            # Store latest telemetry in RTDB (for real-time access)
            ref = self.db.reference(f'/users/{user_id}/devices/{mac_address}/telemetry/latest')
            ref.set(telemetry_data)
            logger.info(f"✓ RTDB updated for {mac_address}")
            
            # Store historical telemetry directly in Firestore
            firestore_data = {
                **telemetry_data,
                'createdAt': timestamp_iso,
                'userId': user_id,
                'macAddress': mac_address
            }
            
            # Add document to Firestore
            update_time, doc_ref = self.firestore_db.collection('users').document(user_id)\
                .collection('devices').document(mac_address)\
                .collection('telemetry').add(firestore_data)
            
            logger.info(f"✓ Firestore document created: {doc_ref.id} at {update_time}")
            
            # Update last seen with Sri Lankan time
            device_ref = self.db.reference(f'/users/{user_id}/devices/{mac_address}')
            device_ref.update({'lastSeen': timestamp_iso})
            
            logger.info(f"✓ Telemetry stored successfully for device {mac_address}")
            return True
        except Exception as e:
            logger.error(f"✗ Error storing telemetry: {e}", exc_info=True)
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
    
    def get_telemetry_history(self, user_id, mac_address, limit=50):
        """
        Get telemetry history from Firestore
        
        Args:
            user_id: Firebase user ID
            mac_address: Device MAC address
            limit: Number of records to return
            
        Returns:
            List of telemetry records
        """
        try:
            docs = self.firestore_db.collection('users').document(user_id)\
                .collection('devices').document(mac_address)\
                .collection('telemetry')\
                .order_by('createdAt', direction=firestore.Query.DESCENDING)\
                .limit(limit)\
                .stream()
            
            telemetry_list = []
            for doc in docs:
                data = doc.to_dict()
                data['id'] = doc.id
                # createdAt is already stored as ISO string
                telemetry_list.append(data)
            
            return telemetry_list
        except Exception as e:
            logger.error(f"Error getting telemetry history: {e}")
            return []
    
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
            # Convert to Sri Lankan time (UTC+5:30)
            timestamp_utc = datetime.now(timezone.utc)
            timestamp_lk = timestamp_utc + timedelta(hours=5, minutes=30)
            timestamp = timestamp_lk.isoformat()
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
