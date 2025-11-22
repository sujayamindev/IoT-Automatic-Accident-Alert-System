enum AlertType {
  fire,
  smoke,
  vibration,
  accident,
}

class Alert {
  final String id;
  final AlertType type;
  final DateTime timestamp;
  final double? latitude;
  final double? longitude;
  final String? description;
  final bool acknowledged;

  Alert({
    required this.id,
    required this.type,
    required this.timestamp,
    this.latitude,
    this.longitude,
    this.description,
    this.acknowledged = false,
  });

  String get typeString {
    switch (type) {
      case AlertType.fire:
        return 'Fire';
      case AlertType.smoke:
        return 'Smoke';
      case AlertType.vibration:
        return 'Vibration';
      case AlertType.accident:
        return 'Accident';
    }
  }

  factory Alert.fromJson(Map<String, dynamic> json) {
    return Alert(
      id: json['id'] ?? '',
      type: _parseAlertType(json['type']),
      timestamp: DateTime.parse(json['timestamp'] ?? DateTime.now().toIso8601String()),
      latitude: json['latitude']?.toDouble(),
      longitude: json['longitude']?.toDouble(),
      description: json['description'],
      acknowledged: json['acknowledged'] ?? false,
    );
  }

  static AlertType _parseAlertType(String? type) {
    switch (type?.toLowerCase()) {
      case 'fire':
        return AlertType.fire;
      case 'smoke':
        return AlertType.smoke;
      case 'vibration':
        return AlertType.vibration;
      case 'accident':
        return AlertType.accident;
      default:
        return AlertType.accident;
    }
  }

  Map<String, dynamic> toJson() {
    return {
      'id': id,
      'type': type.name,
      'timestamp': timestamp.toIso8601String(),
      'latitude': latitude,
      'longitude': longitude,
      'description': description,
      'acknowledged': acknowledged,
    };
  }
}
