"""
Backend Configuration
"""
import os
from dotenv import load_dotenv

load_dotenv()

class Config:
    """Application configuration"""
    
    # Flask
    SECRET_KEY = os.getenv('SECRET_KEY', 'dev-secret-key-change-in-production')
    FLASK_ENV = os.getenv('FLASK_ENV', 'development')
    PORT = int(os.getenv('PORT', 5000))
    
    # Firebase
    FIREBASE_PROJECT_ID = os.getenv('FIREBASE_PROJECT_ID', 'iot-project-fa54e')
    FIREBASE_DATABASE_URL = os.getenv('FIREBASE_DATABASE_URL', 
                                       'https://iot-project-fa54e-default-rtdb.asia-southeast1.firebasedatabase.app')
    FIREBASE_CREDENTIALS_PATH = os.getenv('FIREBASE_CREDENTIALS_PATH', './firebase-service-account.json')
    
    # Device Authentication
    DEVICE_SECRET_KEY = os.getenv('DEVICE_SECRET_KEY', '')
        
    # CORS
    CORS_ORIGINS = ['*']  # Update in production to specific domains
