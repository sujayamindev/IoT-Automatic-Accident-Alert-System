import 'package:flutter/foundation.dart';
import 'package:firebase_auth/firebase_auth.dart';
import 'package:shared_preferences/shared_preferences.dart';
import '../services/auth_service.dart';

// Set to true to use mock authentication (no Firebase required)
const bool useMockAuth = false;

class UserProvider with ChangeNotifier {
  final AuthService _authService = AuthService();
  
  User? _user;
  bool _isLoading = false;
  String? _errorMessage;
  String? _boundDeviceMac;
  String? _boundDeviceName;

  User? get user => _user;
  bool get isLoading => _isLoading;
  String? get errorMessage => _errorMessage;
  String? get boundDeviceMac => _boundDeviceMac;
  String? get boundDeviceName => _boundDeviceName;
  // In mock mode, track authentication separately since we don't have a real User object
  bool _isMockAuthenticated = false;
  bool get isAuthenticated => useMockAuth ? _isMockAuthenticated : _user != null;

  UserProvider() {
    _initAuthListener();
    _loadBoundDevice();
  }

  // Initialize authentication state listener
  void _initAuthListener() {
    _authService.authStateChanges.listen((User? user) {
      _user = user;
      notifyListeners();
    });
  }

  // Load bound device from shared preferences
  Future<void> _loadBoundDevice() async {
    try {
      final prefs = await SharedPreferences.getInstance();
      _boundDeviceMac = prefs.getString('boundDeviceMac');
      _boundDeviceName = prefs.getString('boundDeviceName');
      print('UserProvider: Loaded bound device - MAC: $_boundDeviceMac, Name: $_boundDeviceName');
      notifyListeners();
    } catch (e) {
      print('Error loading bound device: $e');
    }
  }

  // Register new user
  Future<bool> register(String email, String password) async {
    _isLoading = true;
    _errorMessage = null;
    notifyListeners();

    try {
      if (useMockAuth) {
        // Mock authentication - simulate network delay
        await Future.delayed(const Duration(milliseconds: 500));
        // Set authenticated state for mock mode
        _isMockAuthenticated = true;
        _isLoading = false;
        notifyListeners();
        return true;
      } else {
        _user = await _authService.register(email, password);
        _isLoading = false;
        notifyListeners();
        return true;
      }
    } catch (e) {
      _errorMessage = e.toString();
      _isLoading = false;
      notifyListeners();
      return false;
    }
  }

  // Sign in existing user
  Future<bool> signIn(String email, String password) async {
    _isLoading = true;
    _errorMessage = null;
    notifyListeners();

    try {
      if (useMockAuth) {
        // Mock authentication - simulate network delay
        await Future.delayed(const Duration(milliseconds: 500));
        // Set authenticated state for mock mode
        _isMockAuthenticated = true;
        _isLoading = false;
        notifyListeners();
        return true;
      } else {
        _user = await _authService.signIn(email, password);
        _isLoading = false;
        notifyListeners();
        return true;
      }
    } catch (e) {
      _errorMessage = e.toString();
      _isLoading = false;
      notifyListeners();
      return false;
    }
  }

  // Sign out
  Future<void> signOut() async {
    _isLoading = true;
    notifyListeners();

    try {
      if (useMockAuth) {
        await Future.delayed(const Duration(milliseconds: 300));
        _isMockAuthenticated = false;
      } else {
        await _authService.signOut();
      }
      _user = null;
      _isLoading = false;
      notifyListeners();
    } catch (e) {
      _errorMessage = e.toString();
      _isLoading = false;
      notifyListeners();
    }
  }

  // Reset password
  Future<bool> resetPassword(String email) async {
    _isLoading = true;
    _errorMessage = null;
    notifyListeners();

    try {
      await _authService.resetPassword(email);
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

  // Change password
  Future<bool> changePassword(String newPassword) async {
    _isLoading = true;
    _errorMessage = null;
    notifyListeners();

    try {
      await _authService.changePassword(newPassword);
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

  // Set bound device
  Future<void> setBoundDevice(String macAddress, String? deviceName) async {
    _boundDeviceMac = macAddress;
    _boundDeviceName = deviceName;
    print('UserProvider: Setting bound device - MAC: $macAddress, Name: $deviceName');
    notifyListeners();
    
    // Save to shared preferences
    try {
      final prefs = await SharedPreferences.getInstance();
      await prefs.setString('boundDeviceMac', macAddress);
      if (deviceName != null) {
        await prefs.setString('boundDeviceName', deviceName);
      }
      print('UserProvider: Successfully saved to SharedPreferences');
    } catch (e) {
      print('Error saving bound device: $e');
    }
  }

  // Clear bound device
  Future<void> clearBoundDevice() async {
    _boundDeviceMac = null;
    _boundDeviceName = null;
    notifyListeners();
    
    // Clear from shared preferences
    try {
      final prefs = await SharedPreferences.getInstance();
      await prefs.remove('boundDeviceMac');
      await prefs.remove('boundDeviceName');
    } catch (e) {
      print('Error clearing bound device: $e');
    }
  }

  // Clear error message
  void clearError() {
    _errorMessage = null;
    notifyListeners();
  }
}
