import 'package:flutter/foundation.dart';
import '../models/telemetry.dart';
import '../models/trip.dart';
import '../models/alert.dart';
import '../services/telemetry_service.dart';
import '../services/mock_data_service.dart';

// Set to true to use mock data (no backend required)
const bool useMockData = true;

class TelemetryProvider with ChangeNotifier {
  final TelemetryService _telemetryService = TelemetryService();

  Telemetry? _currentTelemetry;
  List<Trip> _trips = [];
  List<Alert> _alerts = [];
  bool _isLoading = false;
  String? _errorMessage;

  Telemetry? get currentTelemetry => _currentTelemetry;
  List<Trip> get trips => _trips;
  List<Alert> get alerts => _alerts;
  bool get isLoading => _isLoading;
  String? get errorMessage => _errorMessage;

  // Get live telemetry data
  Future<void> fetchLiveData() async {
    _isLoading = true;
    _errorMessage = null;
    notifyListeners();

    try {
      if (useMockData) {
        // Simulate network delay
        await Future.delayed(const Duration(milliseconds: 500));
        _currentTelemetry = MockDataService.getRandomTelemetry();
      } else {
        _currentTelemetry = await _telemetryService.getLiveData();
      }
      _isLoading = false;
      notifyListeners();
    } catch (e) {
      _errorMessage = e.toString();
      _isLoading = false;
      notifyListeners();
    }
  }

  // Get trip history
  Future<void> fetchTripHistory({int page = 1, int limit = 20}) async {
    _isLoading = true;
    _errorMessage = null;
    notifyListeners();

    try {
      if (useMockData) {
        // Simulate network delay
        await Future.delayed(const Duration(milliseconds: 500));
        final newTrips = MockDataService.getMockTrips();
        if (page == 1) {
          _trips = newTrips;
        } else {
          _trips.addAll(newTrips);
        }
      } else {
        final newTrips = await _telemetryService.getTripHistory(page: page, limit: limit);
        if (page == 1) {
          _trips = newTrips;
        } else {
          _trips.addAll(newTrips);
        }
      }
      _isLoading = false;
      notifyListeners();
    } catch (e) {
      _errorMessage = e.toString();
      _isLoading = false;
      notifyListeners();
    }
  }

  // Get trip details
  Future<Trip?> fetchTripDetails(String tripId) async {
    try {
      return await _telemetryService.getTripDetails(tripId);
    } catch (e) {
      _errorMessage = e.toString();
      notifyListeners();
      return null;
    }
  }

  // Get alerts history
  Future<void> fetchAlertsHistory({int page = 1, int limit = 20}) async {
    _isLoading = true;
    _errorMessage = null;
    notifyListeners();

    try {
      if (useMockData) {
        // Simulate network delay
        await Future.delayed(const Duration(milliseconds: 500));
        final newAlerts = MockDataService.getMockAlerts();
        if (page == 1) {
          _alerts = newAlerts;
        } else {
          _alerts.addAll(newAlerts);
        }
      } else {
        final newAlerts = await _telemetryService.getAlertsHistory(page: page, limit: limit);
        if (page == 1) {
          _alerts = newAlerts;
        } else {
          _alerts.addAll(newAlerts);
        }
      }
      _isLoading = false;
      notifyListeners();
    } catch (e) {
      _errorMessage = e.toString();
      _isLoading = false;
      notifyListeners();
    }
  }

  // Refresh all data
  Future<void> refreshAll() async {
    await Future.wait([
      fetchLiveData(),
      fetchTripHistory(),
      fetchAlertsHistory(),
    ]);
  }

  // Clear error message
  void clearError() {
    _errorMessage = null;
    notifyListeners();
  }

  // Clear all data (on logout)
  void clearData() {
    _currentTelemetry = null;
    _trips = [];
    _alerts = [];
    _errorMessage = null;
    notifyListeners();
  }
}
