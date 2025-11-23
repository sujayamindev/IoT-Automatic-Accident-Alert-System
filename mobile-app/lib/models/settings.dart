class DeviceSettings {
  final String deviceName;
  final int uploadInterval; // in seconds
  final bool fireDetectionEnabled;
  final bool smokeDetectionEnabled;
  final bool vibrationDetectionEnabled;
  final List<EmergencyContact> emergencyContacts;

  DeviceSettings({
    required this.deviceName,
    required this.uploadInterval,
    required this.fireDetectionEnabled,
    required this.smokeDetectionEnabled,
    required this.vibrationDetectionEnabled,
    required this.emergencyContacts,
  });

  factory DeviceSettings.fromJson(Map<String, dynamic> json) {
    return DeviceSettings(
      deviceName: json['deviceName'] ?? 'My Device',
      uploadInterval: json['uploadInterval'] ?? 5,
      fireDetectionEnabled: json['fireDetectionEnabled'] ?? true,
      smokeDetectionEnabled: json['smokeDetectionEnabled'] ?? true,
      vibrationDetectionEnabled: json['vibrationDetectionEnabled'] ?? true,
      emergencyContacts: json['emergencyContacts'] != null
          ? (json['emergencyContacts'] as List)
              .map((e) => EmergencyContact.fromJson(e))
              .toList()
          : [],
    );
  }

  Map<String, dynamic> toJson() {
    return {
      'deviceName': deviceName,
      'uploadInterval': uploadInterval,
      'fireDetectionEnabled': fireDetectionEnabled,
      'smokeDetectionEnabled': smokeDetectionEnabled,
      'vibrationDetectionEnabled': vibrationDetectionEnabled,
      'emergencyContacts': emergencyContacts.map((e) => e.toJson()).toList(),
    };
  }

  DeviceSettings copyWith({
    String? deviceName,
    int? uploadInterval,
    bool? fireDetectionEnabled,
    bool? smokeDetectionEnabled,
    bool? vibrationDetectionEnabled,
    List<EmergencyContact>? emergencyContacts,
  }) {
    return DeviceSettings(
      deviceName: deviceName ?? this.deviceName,
      uploadInterval: uploadInterval ?? this.uploadInterval,
      fireDetectionEnabled: fireDetectionEnabled ?? this.fireDetectionEnabled,
      smokeDetectionEnabled: smokeDetectionEnabled ?? this.smokeDetectionEnabled,
      vibrationDetectionEnabled: vibrationDetectionEnabled ?? this.vibrationDetectionEnabled,
      emergencyContacts: emergencyContacts ?? this.emergencyContacts,
    );
  }
}

class EmergencyContact {
  final String id;
  final String name;
  final String phoneNumber;
  final bool isPrimary;

  EmergencyContact({
    required this.id,
    required this.name,
    required this.phoneNumber,
    this.isPrimary = false,
  });

  factory EmergencyContact.fromJson(Map<String, dynamic> json) {
    return EmergencyContact(
      id: json['id'] ?? '',
      name: json['name'] ?? '',
      phoneNumber: json['phoneNumber'] ?? '',
      isPrimary: json['isPrimary'] ?? false,
    );
  }

  Map<String, dynamic> toJson() {
    return {
      'id': id,
      'name': name,
      'phoneNumber': phoneNumber,
      'isPrimary': isPrimary,
    };
  }

  EmergencyContact copyWith({
    String? id,
    String? name,
    String? phoneNumber,
    bool? isPrimary,
  }) {
    return EmergencyContact(
      id: id ?? this.id,
      name: name ?? this.name,
      phoneNumber: phoneNumber ?? this.phoneNumber,
      isPrimary: isPrimary ?? this.isPrimary,
    );
  }
}
