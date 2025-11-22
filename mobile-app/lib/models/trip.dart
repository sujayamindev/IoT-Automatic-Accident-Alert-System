class Trip {
  final String id;
  final DateTime startTime;
  final DateTime endTime;
  final double distance; // in kilometers
  final double maxSpeed; // in km/h
  final double avgSpeed; // in km/h
  final List<TripPoint>? route;

  Trip({
    required this.id,
    required this.startTime,
    required this.endTime,
    required this.distance,
    required this.maxSpeed,
    required this.avgSpeed,
    this.route,
  });

  Duration get duration => endTime.difference(startTime);

  factory Trip.fromJson(Map<String, dynamic> json) {
    return Trip(
      id: json['id'] ?? '',
      startTime: DateTime.parse(json['startTime'] ?? DateTime.now().toIso8601String()),
      endTime: DateTime.parse(json['endTime'] ?? DateTime.now().toIso8601String()),
      distance: json['distance']?.toDouble() ?? 0.0,
      maxSpeed: json['maxSpeed']?.toDouble() ?? 0.0,
      avgSpeed: json['avgSpeed']?.toDouble() ?? 0.0,
      route: json['route'] != null
          ? (json['route'] as List).map((e) => TripPoint.fromJson(e)).toList()
          : null,
    );
  }

  Map<String, dynamic> toJson() {
    return {
      'id': id,
      'startTime': startTime.toIso8601String(),
      'endTime': endTime.toIso8601String(),
      'distance': distance,
      'maxSpeed': maxSpeed,
      'avgSpeed': avgSpeed,
      'route': route?.map((e) => e.toJson()).toList(),
    };
  }
}

class TripPoint {
  final double latitude;
  final double longitude;
  final double speed;
  final DateTime timestamp;

  TripPoint({
    required this.latitude,
    required this.longitude,
    required this.speed,
    required this.timestamp,
  });

  factory TripPoint.fromJson(Map<String, dynamic> json) {
    return TripPoint(
      latitude: json['latitude']?.toDouble() ?? 0.0,
      longitude: json['longitude']?.toDouble() ?? 0.0,
      speed: json['speed']?.toDouble() ?? 0.0,
      timestamp: DateTime.parse(json['timestamp'] ?? DateTime.now().toIso8601String()),
    );
  }

  Map<String, dynamic> toJson() {
    return {
      'latitude': latitude,
      'longitude': longitude,
      'speed': speed,
      'timestamp': timestamp.toIso8601String(),
    };
  }
}
