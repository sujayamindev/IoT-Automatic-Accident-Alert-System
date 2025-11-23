import 'package:flutter/foundation.dart';
import 'package:firebase_auth/firebase_auth.dart';
import '../services/auth_service.dart';

// Set to true to use mock authentication (no Firebase required)
const bool useMockAuth = false;

class UserProvider with ChangeNotifier {
  final AuthService _authService = AuthService();
  
  User? _user;
  bool _isLoading = false;
  String? _errorMessage;

  User? get user => _user;
  bool get isLoading => _isLoading;
  String? get errorMessage => _errorMessage;
  // In mock mode, track authentication separately since we don't have a real User object
  bool _isMockAuthenticated = false;
  bool get isAuthenticated => useMockAuth ? _isMockAuthenticated : _user != null;

  UserProvider() {
    _initAuthListener();
  }

  // Initialize authentication state listener
  void _initAuthListener() {
    _authService.authStateChanges.listen((User? user) {
      _user = user;
      notifyListeners();
    });
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

  // Clear error message
  void clearError() {
    _errorMessage = null;
    notifyListeners();
  }
}
