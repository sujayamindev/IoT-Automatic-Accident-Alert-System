class Telemetry {
  final double latitude;
  final double longitude;
  final double speed;
  final bool fireDetected;
  final bool smokeDetected;
  final int gsmSignalLevel;
  final DateTime timestamp;
  
  // MPU6050 data
  final int accelX;
  final int accelY;
  final int accelZ;
  final int gyroX;
  final int gyroY;
  final int gyroZ;
  
  // Environmental sensors
  final double temperature;
  final double humidity;
  final int coLevel;
  
  // GPS timestamp
  final String? gpsTimestamp;
  
  // Impact detection (sticky flag from backend)
  final bool hasHighImpact;

  Telemetry({
    required this.latitude,
    required this.longitude,
    required this.speed,
    required this.fireDetected,
    required this.smokeDetected,
    required this.gsmSignalLevel,
    required this.timestamp,
    this.accelX = 0,
    this.accelY = 0,
    this.accelZ = 0,
    this.gyroX = 0,
    this.gyroY = 0,
    this.gyroZ = 0,
    this.temperature = 0.0,
    this.humidity = 0.0,
    this.coLevel = 0,
    this.gpsTimestamp,
    this.hasHighImpact = false,
  });

  factory Telemetry.fromJson(Map<String, dynamic> json) {
    return Telemetry(
      latitude: json['latitude']?.toDouble() ?? 0.0,
      longitude: json['longitude']?.toDouble() ?? 0.0,
      speed: json['speed']?.toDouble() ?? 0.0,
      fireDetected: json['fireDetected'] ?? false,
      smokeDetected: json['smokeDetected'] ?? false,
      gsmSignalLevel: json['gsmSignalLevel'] ?? 0,
      timestamp: DateTime.parse(json['timestamp'] ?? DateTime.now().toIso8601String()),
      // Read accelerometer values - backend may store as int or double
      accelX: json['accel_x'] is int ? json['accel_x'] : (json['accel_x']?.toInt() ?? 0),
      accelY: json['accel_y'] is int ? json['accel_y'] : (json['accel_y']?.toInt() ?? 0),
      accelZ: json['accel_z'] is int ? json['accel_z'] : (json['accel_z']?.toInt() ?? 0),
      gyroX: json['gyro_x'] is int ? json['gyro_x'] : (json['gyro_x']?.toInt() ?? 0),
      gyroY: json['gyro_y'] is int ? json['gyro_y'] : (json['gyro_y']?.toInt() ?? 0),
      gyroZ: json['gyro_z'] is int ? json['gyro_z'] : (json['gyro_z']?.toInt() ?? 0),
      temperature: json['temperature']?.toDouble() ?? 0.0,
      humidity: json['humidity']?.toDouble() ?? 0.0,
      coLevel: json['coLevel'] ?? 0,
      gpsTimestamp: json['gpsTimestamp'],
      hasHighImpact: json['hasHighImpact'] ?? false,
    );
  }

  Map<String, dynamic> toJson() {
    return {
      'latitude': latitude,
      'longitude': longitude,
      'speed': speed,
      'fireDetected': fireDetected,
      'smokeDetected': smokeDetected,
      'gsmSignalLevel': gsmSignalLevel,
      'timestamp': timestamp.toIso8601String(),
      'accel_x': accelX,
      'accel_y': accelY,
      'accel_z': accelZ,
      'gyro_x': gyroX,
      'gyro_y': gyroY,
      'gyro_z': gyroZ,
      'temperature': temperature,
      'humidity': humidity,
      'coLevel': coLevel,
      'gpsTimestamp': gpsTimestamp,
      'hasHighImpact': hasHighImpact,
    };
  }
}
