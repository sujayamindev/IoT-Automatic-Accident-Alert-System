import '../models/telemetry.dart';
import '../models/trip.dart';
import '../models/alert.dart';
import '../models/settings.dart';

class MockDataService {
  // Mock live telemetry data
  static Telemetry getMockTelemetry() {
    return Telemetry(
      latitude: 6.9271, // Colombo, Sri Lanka
      longitude: 79.8612,
      speed: 45.5,
      fireDetected: false,
      smokeDetected: false,
      vibrationDetected: false,
      batteryPercentage: 85,
      gsmSignalLevel: 4,
      timestamp: DateTime.now(),
    );
  }

  // Mock trip history
  static List<Trip> getMockTrips() {
    final now = DateTime.now();
    return [
      Trip(
        id: '1',
        startTime: now.subtract(const Duration(hours: 2)),
        endTime: now.subtract(const Duration(hours: 1, minutes: 30)),
        distance: 15.5,
        maxSpeed: 65.0,
        avgSpeed: 45.0,
        route: [
          TripPoint(
            latitude: 6.9271,
            longitude: 79.8612,
            speed: 0,
            timestamp: now.subtract(const Duration(hours: 2)),
          ),
          TripPoint(
            latitude: 6.9350,
            longitude: 79.8700,
            speed: 45,
            timestamp: now.subtract(const Duration(hours: 1, minutes: 45)),
          ),
          TripPoint(
            latitude: 6.9450,
            longitude: 79.8800,
            speed: 50,
            timestamp: now.subtract(const Duration(hours: 1, minutes: 30)),
          ),
        ],
      ),
      Trip(
        id: '2',
        startTime: now.subtract(const Duration(days: 1, hours: 3)),
        endTime: now.subtract(const Duration(days: 1, hours: 2)),
        distance: 22.3,
        maxSpeed: 70.0,
        avgSpeed: 52.0,
      ),
      Trip(
        id: '3',
        startTime: now.subtract(const Duration(days: 2, hours: 4)),
        endTime: now.subtract(const Duration(days: 2, hours: 3)),
        distance: 8.7,
        maxSpeed: 55.0,
        avgSpeed: 38.0,
      ),
      Trip(
        id: '4',
        startTime: now.subtract(const Duration(days: 3, hours: 5)),
        endTime: now.subtract(const Duration(days: 3, hours: 4)),
        distance: 18.2,
        maxSpeed: 68.0,
        avgSpeed: 48.0,
      ),
      Trip(
        id: '5',
        startTime: now.subtract(const Duration(days: 4, hours: 2)),
        endTime: now.subtract(const Duration(days: 4, hours: 1)),
        distance: 12.9,
        maxSpeed: 60.0,
        avgSpeed: 42.0,
      ),
    ];
  }

  // Mock alerts history
  static List<Alert> getMockAlerts() {
    final now = DateTime.now();
    return [
      Alert(
        id: '1',
        type: AlertType.vibration,
        timestamp: now.subtract(const Duration(hours: 3)),
        latitude: 6.9271,
        longitude: 79.8612,
        description: 'High vibration detected',
        acknowledged: true,
      ),
      Alert(
        id: '2',
        type: AlertType.smoke,
        timestamp: now.subtract(const Duration(days: 1, hours: 5)),
        latitude: 6.9350,
        longitude: 79.8700,
        description: 'Smoke sensor triggered',
        acknowledged: true,
      ),
      Alert(
        id: '3',
        type: AlertType.fire,
        timestamp: now.subtract(const Duration(days: 2, hours: 8)),
        latitude: 6.9450,
        longitude: 79.8800,
        description: 'Fire detected in engine compartment',
        acknowledged: false,
      ),
      Alert(
        id: '4',
        type: AlertType.vibration,
        timestamp: now.subtract(const Duration(days: 3, hours: 2)),
        latitude: 6.9200,
        longitude: 79.8500,
        description: 'Impact detected',
        acknowledged: true,
      ),
      Alert(
        id: '5',
        type: AlertType.accident,
        timestamp: now.subtract(const Duration(days: 5, hours: 4)),
        latitude: 6.9100,
        longitude: 79.8400,
        description: 'Severe impact - possible accident',
        acknowledged: true,
      ),
    ];
  }

  // Mock device settings
  static DeviceSettings getMockSettings() {
    return DeviceSettings(
      deviceName: 'My Vehicle',
      uploadInterval: 5,
      fireDetectionEnabled: true,
      smokeDetectionEnabled: true,
      vibrationDetectionEnabled: true,
      emergencyContacts: [
        EmergencyContact(
          id: '1',
          name: 'Emergency Services',
          phoneNumber: '+94112691111',
          isPrimary: true,
        ),
        EmergencyContact(
          id: '2',
          name: 'John Doe',
          phoneNumber: '+94771234567',
          isPrimary: false,
        ),
        EmergencyContact(
          id: '3',
          name: 'Jane Smith',
          phoneNumber: '+94779876543',
          isPrimary: false,
        ),
      ],
    );
  }

  // Simulate random sensor changes for live demo
  static Telemetry getRandomTelemetry() {
    final random = DateTime.now().millisecond;
    return Telemetry(
      latitude: 6.9271 + (random % 100) / 10000,
      longitude: 79.8612 + (random % 100) / 10000,
      speed: 30.0 + (random % 40).toDouble(),
      fireDetected: random % 20 == 0,
      smokeDetected: random % 15 == 0,
      vibrationDetected: random % 10 == 0,
      batteryPercentage: 70 + (random % 30),
      gsmSignalLevel: 3 + (random % 2),
      timestamp: DateTime.now(),
    );
  }
}
