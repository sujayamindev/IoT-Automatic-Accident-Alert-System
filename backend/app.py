"""
Main Flask Application
IoT Accident Alert System Backend
"""
from flask import Flask, jsonify
from flask_cors import CORS
from apscheduler.schedulers.background import BackgroundScheduler
from config import Config
from firebase_service import FirebaseService
from middleware import init_middleware
from routes.device_routes import device_bp, init_routes as init_device_routes
from routes.app_routes import app_bp, init_routes as init_app_routes
import logging
import atexit

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

# Initialize Firebase service
try:
    firebase_service = FirebaseService()
    logger.info("Firebase service initialized")
except Exception as e:
    logger.error(f"Failed to initialize Firebase: {e}")
    raise

# Initialize middleware
init_middleware(firebase_service)

# Initialize routes
init_device_routes(firebase_service)
init_app_routes(firebase_service)

# Register blueprints
app.register_blueprint(device_bp)
app.register_blueprint(app_bp)


# Background scheduler for data migration
scheduler = BackgroundScheduler()
scheduler.add_job(
    func=firebase_service.migrate_old_telemetry_to_firestore,
    trigger="interval",
    minutes=15,  # Run every 15 minutes
    id='migrate_telemetry',
    name='Migrate old telemetry to Firestore',
    replace_existing=True
)
scheduler.start()
logger.info("Background scheduler started for telemetry migration")

# Shut down the scheduler when exiting the app
atexit.register(lambda: scheduler.shutdown())


@app.route('/')
def index():
    """Root endpoint - API status"""
    return jsonify({
        'service': 'IoT Accident Alert System Backend',
        'status': 'running',
        'version': '1.0.0',
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
    """Health check endpoint"""
    return jsonify({
        'status': 'healthy',
        'firebase': 'connected'
    }), 200


@app.errorhandler(404)
def not_found(error):
    """Handle 404 errors"""
    return jsonify({'error': 'Endpoint not found'}), 404


@app.errorhandler(500)
def internal_error(error):
    """Handle 500 errors"""
    logger.error(f"Internal server error: {error}")
    return jsonify({'error': 'Internal server error'}), 500


if __name__ == '__main__':
    port = Config.PORT
    debug = Config.FLASK_ENV == 'development'
    
    logger.info(f"Starting Flask server on port {port}")
    logger.info(f"Environment: {Config.FLASK_ENV}")
    logger.info(f"Firebase Project: {Config.FIREBASE_PROJECT_ID}")
    
    app.run(
        host='0.0.0.0',
        port=port,
        debug=debug
    )
