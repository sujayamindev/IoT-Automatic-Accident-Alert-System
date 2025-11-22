class Telemetry {
  final double latitude;
  final double longitude;
  final double speed;
  final bool fireDetected;
  final bool smokeDetected;
  final bool vibrationDetected;
  final int batteryPercentage;
  final int gsmSignalLevel;
  final DateTime timestamp;

  Telemetry({
    required this.latitude,
    required this.longitude,
    required this.speed,
    required this.fireDetected,
    required this.smokeDetected,
    required this.vibrationDetected,
    required this.batteryPercentage,
    required this.gsmSignalLevel,
    required this.timestamp,
  });

  factory Telemetry.fromJson(Map<String, dynamic> json) {
    return Telemetry(
      latitude: json['latitude']?.toDouble() ?? 0.0,
      longitude: json['longitude']?.toDouble() ?? 0.0,
      speed: json['speed']?.toDouble() ?? 0.0,
      fireDetected: json['fireDetected'] ?? false,
      smokeDetected: json['smokeDetected'] ?? false,
      vibrationDetected: json['vibrationDetected'] ?? false,
      batteryPercentage: json['batteryPercentage'] ?? 0,
      gsmSignalLevel: json['gsmSignalLevel'] ?? 0,
      timestamp: DateTime.parse(json['timestamp'] ?? DateTime.now().toIso8601String()),
    );
  }

  Map<String, dynamic> toJson() {
    return {
      'latitude': latitude,
      'longitude': longitude,
      'speed': speed,
      'fireDetected': fireDetected,
      'smokeDetected': smokeDetected,
      'vibrationDetected': vibrationDetected,
      'batteryPercentage': batteryPercentage,
      'gsmSignalLevel': gsmSignalLevel,
      'timestamp': timestamp.toIso8601String(),
    };
  }
}
