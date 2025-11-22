import '../models/telemetry.dart';
import '../models/trip.dart';
import '../models/alert.dart';
import '../models/settings.dart';
import 'api_service.dart';

class TelemetryService {
  final ApiService _apiService = ApiService();

  // Get live telemetry data
  Future<Telemetry> getLiveData() async {
    try {
      final response = await _apiService.get('/getLiveData');
      return Telemetry.fromJson(response['data']);
    } catch (e) {
      throw Exception('Failed to get live data: $e');
    }
  }

  // Get trip history
  Future<List<Trip>> getTripHistory({int page = 1, int limit = 20}) async {
    try {
      final response = await _apiService.get('/getTripHistory?page=$page&limit=$limit');
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
  Future<List<Alert>> getAlertsHistory({int page = 1, int limit = 20}) async {
    try {
      final response = await _apiService.get('/getAlertsHistory?page=$page&limit=$limit');
      final List<dynamic> alerts = response['alerts'] ?? [];
      return alerts.map((alert) => Alert.fromJson(alert)).toList();
    } catch (e) {
      throw Exception('Failed to get alerts history: $e');
    }
  }

  // Get device settings
  Future<DeviceSettings> getSettings() async {
    try {
      final response = await _apiService.get('/getSettings');
      return DeviceSettings.fromJson(response['settings']);
    } catch (e) {
      throw Exception('Failed to get settings: $e');
    }
  }

  // Update device settings
  Future<bool> updateSettings(DeviceSettings settings) async {
    try {
      await _apiService.put('/updateSettings', settings.toJson());
      return true;
    } catch (e) {
      throw Exception('Failed to update settings: $e');
    }
  }

  // Add emergency contact
  Future<EmergencyContact> addContact(EmergencyContact contact) async {
    try {
      final response = await _apiService.post('/addContact', contact.toJson());
      return EmergencyContact.fromJson(response['contact']);
    } catch (e) {
      throw Exception('Failed to add contact: $e');
    }
  }

  // Update emergency contact
  Future<bool> updateContact(EmergencyContact contact) async {
    try {
      await _apiService.put('/updateContact/${contact.id}', contact.toJson());
      return true;
    } catch (e) {
      throw Exception('Failed to update contact: $e');
    }
  }

  // Delete emergency contact
  Future<bool> deleteContact(String contactId) async {
    try {
      await _apiService.delete('/deleteContact/$contactId');
      return true;
    } catch (e) {
      throw Exception('Failed to delete contact: $e');
    }
  }

  // Bind device to user
  Future<bool> bindDevice(String deviceId) async {
    try {
      await _apiService.post('/bindDeviceToUser', {'deviceId': deviceId});
      return true;
    } catch (e) {
      throw Exception('Failed to bind device: $e');
    }
  }

  // Unbind device from user
  Future<bool> unbindDevice() async {
    try {
      await _apiService.post('/unbindDevice', {});
      return true;
    } catch (e) {
      throw Exception('Failed to unbind device: $e');
    }
  }
}
