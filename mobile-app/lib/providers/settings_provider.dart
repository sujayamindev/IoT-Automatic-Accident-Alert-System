import 'package:flutter/foundation.dart';
import '../models/settings.dart';
import '../services/telemetry_service.dart';
import '../services/mock_data_service.dart';

// Set to true to use mock data (no backend required)
const bool useMockData = true;

class SettingsProvider with ChangeNotifier {
  final TelemetryService _telemetryService = TelemetryService();

  DeviceSettings? _settings;
  bool _isLoading = false;
  String? _errorMessage;

  DeviceSettings? get settings => _settings;
  bool get isLoading => _isLoading;
  String? get errorMessage => _errorMessage;

  // Fetch device settings
  Future<void> fetchSettings(String macAddress) async {
    _isLoading = true;
    _errorMessage = null;
    notifyListeners();

    try {
      if (useMockData) {
        // Simulate network delay
        await Future.delayed(const Duration(milliseconds: 500));
        _settings = MockDataService.getMockSettings();
      } else {
        _settings = await _telemetryService.getSettings(macAddress);
      }
      _isLoading = false;
      notifyListeners();
    } catch (e) {
      _errorMessage = e.toString();
      _isLoading = false;
      notifyListeners();
    }
  }

  // Update device settings
  Future<bool> updateSettings(DeviceSettings newSettings) async {
    _isLoading = true;
    _errorMessage = null;
    notifyListeners();

    try {
      if (useMockData) {
        // Simulate network delay
        await Future.delayed(const Duration(milliseconds: 300));
        _settings = newSettings;
      } else {
        await _telemetryService.updateSettings(newSettings);
        _settings = newSettings;
      }
      _isLoading = false;
      notifyListeners();
      return true;
    } catch (e) {
      _errorMessage = e.toString();
      _isLoading = false;
      notifyListeners();
      return false;
    }
  }

  // Update device name
  Future<bool> updateDeviceName(String name) async {
    if (_settings == null) return false;
    return await updateSettings(_settings!.copyWith(deviceName: name));
  }

  // Update upload interval
  Future<bool> updateUploadInterval(int interval) async {
    if (_settings == null) return false;
    return await updateSettings(_settings!.copyWith(uploadInterval: interval));
  }

  // Toggle sensor detection
  Future<bool> toggleFireDetection(bool enabled) async {
    if (_settings == null) return false;
    return await updateSettings(_settings!.copyWith(fireDetectionEnabled: enabled));
  }

  Future<bool> toggleSmokeDetection(bool enabled) async {
    if (_settings == null) return false;
    return await updateSettings(_settings!.copyWith(smokeDetectionEnabled: enabled));
  }

  Future<bool> toggleVibrationDetection(bool enabled) async {
    if (_settings == null) return false;
    return await updateSettings(_settings!.copyWith(vibrationDetectionEnabled: enabled));
  }

  // Add emergency contact
  Future<bool> addContact(String name, String phoneNumber, {bool isPrimary = false}) async {
    if (_settings == null) return false;

    try {
      _isLoading = true;
      notifyListeners();

      final contact = EmergencyContact(
        id: DateTime.now().millisecondsSinceEpoch.toString(),
        name: name,
        phoneNumber: phoneNumber,
        isPrimary: isPrimary,
      );

      if (useMockData) {
        // Simulate network delay
        await Future.delayed(const Duration(milliseconds: 300));
        final updatedContacts = [..._settings!.emergencyContacts, contact];
        _settings = _settings!.copyWith(emergencyContacts: updatedContacts);
      } else {
        final newContact = await _telemetryService.addContact(contact);
        final updatedContacts = [..._settings!.emergencyContacts, newContact];
        _settings = _settings!.copyWith(emergencyContacts: updatedContacts);
      }
      
      _isLoading = false;
      notifyListeners();
      return true;
    } catch (e) {
      _errorMessage = e.toString();
      _isLoading = false;
      notifyListeners();
      return false;
    }
  }

  // Update emergency contact
  Future<bool> updateContact(EmergencyContact contact) async {
    if (_settings == null) return false;

    try {
      _isLoading = true;
      notifyListeners();

      if (useMockData) {
        // Simulate network delay
        await Future.delayed(const Duration(milliseconds: 300));
      } else {
        await _telemetryService.updateContact(contact);
      }
      
      final updatedContacts = _settings!.emergencyContacts
          .map((c) => c.id == contact.id ? contact : c)
          .toList();
      _settings = _settings!.copyWith(emergencyContacts: updatedContacts);
      
      _isLoading = false;
      notifyListeners();
      return true;
    } catch (e) {
      _errorMessage = e.toString();
      _isLoading = false;
      notifyListeners();
      return false;
    }
  }

  // Delete emergency contact
  Future<bool> deleteContact(String contactId) async {
    if (_settings == null) return false;

    try {
      _isLoading = true;
      notifyListeners();

      if (useMockData) {
        // Simulate network delay
        await Future.delayed(const Duration(milliseconds: 300));
      } else {
        await _telemetryService.deleteContact(contactId);
      }
      
      final updatedContacts = _settings!.emergencyContacts
          .where((c) => c.id != contactId)
          .toList();
      _settings = _settings!.copyWith(emergencyContacts: updatedContacts);
      
      _isLoading = false;
      notifyListeners();
      return true;
    } catch (e) {
      _errorMessage = e.toString();
      _isLoading = false;
      notifyListeners();
      return false;
    }
  }

  // Clear error message
  void clearError() {
    _errorMessage = null;
    notifyListeners();
  }

  // Clear settings (on logout)
  void clearSettings() {
    _settings = null;
    _errorMessage = null;
    notifyListeners();
  }
}
