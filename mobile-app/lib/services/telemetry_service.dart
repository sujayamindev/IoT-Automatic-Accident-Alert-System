import '../models/telemetry.dart';
import '../models/trip.dart';
import '../models/alert.dart';
import '../models/settings.dart';
import 'api_service.dart';

class TelemetryService {
  final ApiService _apiService = ApiService();

  // Get live telemetry data
  Future<Telemetry> getLiveData(String macAddress) async {
    try {
      print('TelemetryService: Calling API with MAC: $macAddress');
      final response = await _apiService.get('/api/app/getLiveData?macAddress=$macAddress');
      print('TelemetryService: API Response: $response');
      // Backend returns telemetry data directly, not wrapped in 'data' field
      return Telemetry.fromJson(response);
    } catch (e) {
      print('TelemetryService: Error getting live data: $e');
      throw Exception('Failed to get live data: $e');
    }
  }

  // Get trip history
  Future<List<Trip>> getTripHistory(String macAddress, {int page = 1, int limit = 20}) async {
    try {
      final response = await _apiService.get('/api/app/getTripHistory?macAddress=$macAddress&page=$page&limit=$limit');
      final List<dynamic> trips = response['trips'] ?? [];
      return trips.map((trip) => Trip.fromJson(trip)).toList();
    } catch (e) {
      throw Exception('Failed to get trip history: $e');
    }
  }

  // Get trip details
  Future<Trip> getTripDetails(String tripId) async {
    try {
      final response = await _apiService.get('/getTripDetails/$tripId');
      return Trip.fromJson(response['trip']);
    } catch (e) {
      throw Exception('Failed to get trip details: $e');
    }
  }

  // Get alerts history
  Future<List<Alert>> getAlertsHistory(String macAddress, {int page = 1, int limit = 20}) async {
    try {
      final response = await _apiService.get('/api/app/getAlertsHistory?macAddress=$macAddress&page=$page&limit=$limit');
      final List<dynamic> alerts = response['alerts'] ?? [];
      return alerts.map((alert) => Alert.fromJson(alert)).toList();
    } catch (e) {
      throw Exception('Failed to get alerts history: $e');
    }
  }

  // Get device settings
  Future<DeviceSettings> getSettings(String macAddress) async {
    try {
      final response = await _apiService.get('/api/app/getSettings?macAddress=$macAddress');
      return DeviceSettings.fromJson(response['settings']);
    } catch (e) {
      throw Exception('Failed to get settings: $e');
    }
  }

  // Update device settings
  Future<bool> updateSettings(DeviceSettings settings) async {
    try {
      await _apiService.put('/api/app/updateSettings', settings.toJson());
      return true;
    } catch (e) {
      throw Exception('Failed to update settings: $e');
    }
  }

  // Add emergency contact
  Future<EmergencyContact> addContact(EmergencyContact contact) async {
    try {
      final response = await _apiService.post('/api/app/addContact', contact.toJson());
      return EmergencyContact.fromJson(response['contact']);
    } catch (e) {
      throw Exception('Failed to add contact: $e');
    }
  }

  // Update emergency contact
  Future<bool> updateContact(EmergencyContact contact) async {
    try {
      await _apiService.put('/api/app/updateContact/${contact.id}', contact.toJson());
      return true;
    } catch (e) {
      throw Exception('Failed to update contact: $e');
    }
  }

  // Delete emergency contact
  Future<bool> deleteContact(String contactId) async {
    try {
      await _apiService.delete('/api/app/deleteContact/$contactId');
      return true;
    } catch (e) {
      throw Exception('Failed to delete contact: $e');
    }
  }

  // Bind device to user
  Future<bool> bindDevice(String macAddress, {String? deviceName}) async {
    try {
      await _apiService.post('/api/app/bindDevice', {
        'macAddress': macAddress,
        'deviceName': deviceName ?? 'My Device',
      });
      return true;
    } catch (e) {
      throw Exception('Failed to bind device: $e');
    }
  }

  // Unbind device from user
  Future<bool> unbindDevice() async {
    try {
      await _apiService.post('/api/app/unbindDevice', {});
      return true;
    } catch (e) {
      throw Exception('Failed to unbind device: $e');
    }
  }
}
