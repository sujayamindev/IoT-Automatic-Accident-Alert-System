"""
Test App - Uses Mock Firebase for testing without credentials
Run this instead of app.py if you don't have firebase-service-account.json yet
"""
from flask import Flask, jsonify
from flask_cors import CORS
from config import Config
from mock_firebase import MockFirebaseService
from middleware import init_middleware
from routes.device_routes import device_bp, init_routes as init_device_routes
from routes.app_routes import app_bp, init_routes as init_app_routes
import logging

# Configure logging
logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s - %(name)s - %(levelname)s - %(message)s'
)
logger = logging.getLogger(__name__)

# Initialize Flask app
app = Flask(__name__)
app.config.from_object(Config)

# Enable CORS
CORS(app, origins=Config.CORS_ORIGINS)

# Initialize MOCK Firebase service
logger.warning("=" * 60)
logger.warning("RUNNING IN TEST MODE WITH MOCK FIREBASE")
logger.warning("Data will NOT be saved to Firebase!")
logger.warning("=" * 60)

firebase_service = MockFirebaseService()

# Initialize middleware
init_middleware(firebase_service)

# Initialize routes
init_device_routes(firebase_service)
init_app_routes(firebase_service)

# Register blueprints
app.register_blueprint(device_bp)
app.register_blueprint(app_bp)

@app.route('/')
def index():
    """Root endpoint"""
    return jsonify({
        'service': 'IoT Accident Alert System Backend',
        'status': 'running',
        'mode': 'TEST MODE (Mock Firebase)',
        'version': '1.0.0',
        'warning': 'Data will not persist - using mock Firebase service',
        'endpoints': {
            'device': {
                'uploadTelemetry': 'POST /api/device/uploadTelemetry',
                'fetchConfig': 'GET /api/device/fetchConfig',
                'ping': 'GET /api/device/ping'
            },
            'app': {
                'bindDevice': 'POST /api/app/bindDevice',
                'unbindDevice': 'POST /api/app/unbindDevice',
                'getLiveData': 'GET /api/app/getLiveData',
                'getTripHistory': 'GET /api/app/getTripHistory',
                'getAlertsHistory': 'GET /api/app/getAlertsHistory',
                'getSettings': 'GET /api/app/getSettings',
                'updateSettings': 'PUT /api/app/updateSettings',
                'addContact': 'POST /api/app/addContact',
                'updateContact': 'PUT /api/app/updateContact/<id>',
                'deleteContact': 'DELETE /api/app/deleteContact/<id>'
            }
        }
    }), 200

@app.route('/health')
def health():
    """Health check"""
    return jsonify({
        'status': 'healthy',
        'firebase': 'mock',
        'mode': 'test'
    }), 200

@app.errorhandler(404)
def not_found(error):
    """Handle 404"""
    return jsonify({'error': 'Endpoint not found'}), 404

@app.errorhandler(500)
def internal_error(error):
    """Handle 500"""
    logger.error(f"Internal server error: {error}")
    return jsonify({'error': 'Internal server error'}), 500

if __name__ == '__main__':
    port = Config.PORT
    
    logger.info("=" * 60)
    logger.info("Starting Flask TEST server")
    logger.info(f"Port: {port}")
    logger.info(f"Mode: TEST (Mock Firebase)")
    logger.info("=" * 60)
    logger.info("\nTest the API:")
    logger.info(f"  Root: http://localhost:{port}/")
    logger.info(f"  Health: http://localhost:{port}/health")
    logger.info(f"  Device Ping: http://localhost:{port}/api/device/ping")
    logger.info("\nRun test_api.py in another terminal to test all endpoints")
    logger.info("=" * 60)
    
    app.run(
        host='0.0.0.0',
        port=port,
        debug=True
    )
