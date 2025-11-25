import 'dart:convert';
import 'package:http/http.dart' as http;
import 'package:firebase_auth/firebase_auth.dart';

class ApiService {
  static const String baseUrl = 'http://10.161.234.199:5000';
  final FirebaseAuth _auth = FirebaseAuth.instance;

  // Get fresh Firebase ID token
  Future<String?> getToken() async {
    try {
      final user = _auth.currentUser;
      if (user == null) return null;
      
      // Get fresh ID token (Firebase handles caching internally)
      return await user.getIdToken();
    } catch (e) {
      return null;
    }
  }

  // Set authentication token (deprecated - kept for compatibility)
  Future<void> setToken(String token) async {
    // No longer needed - Firebase handles token management
  }

  // Clear authentication token (deprecated - kept for compatibility)
  Future<void> clearToken() async {
    // No longer needed - Firebase handles token management
  }

  // Generic GET request
  Future<dynamic> get(String endpoint) async {
    try {
      final token = await getToken();
      final response = await http.get(
        Uri.parse('$baseUrl$endpoint'),
        headers: {
          'Content-Type': 'application/json',
          if (token != null) 'Authorization': 'Bearer $token',
        },
      );

      if (response.statusCode == 200) {
        return json.decode(response.body);
      } else if (response.statusCode == 401) {
        await clearToken();
        throw Exception('Unauthorized');
      } else {
        throw Exception('Failed to load data: ${response.statusCode}');
      }
    } catch (e) {
      throw Exception('Network error: $e');
    }
  }

  // Generic POST request
  Future<dynamic> post(String endpoint, Map<String, dynamic> data) async {
    try {
      print('POST request to: $baseUrl$endpoint');
      print('Data: $data');
      
      final token = await getToken();
      
      if (token == null) {
        print('Error: No authentication token available');
        throw Exception('Not authenticated. Please sign in.');
      }
      
      print('Token obtained, sending request...');
      
      final response = await http.post(
        Uri.parse('$baseUrl$endpoint'),
        headers: {
          'Content-Type': 'application/json',
          'Authorization': 'Bearer $token',
        },
        body: json.encode(data),
      ).timeout(const Duration(seconds: 30));

      print('Response status: ${response.statusCode}');
      print('Response body: ${response.body}');

      if (response.statusCode == 200 || response.statusCode == 201) {
        return json.decode(response.body);
      } else if (response.statusCode == 401) {
        throw Exception('Session expired. Please sign in again.');
      } else {
        final errorBody = response.body.isNotEmpty 
            ? json.decode(response.body) 
            : {'error': 'Unknown error'};
        throw Exception(errorBody['error'] ?? 'Request failed: ${response.statusCode}');
      }
    } catch (e) {
      print('POST request error: $e');
      if (e.toString().contains('SocketException') || e.toString().contains('Failed host lookup')) {
        throw Exception('Cannot reach backend. Make sure it\'s running at $baseUrl');
      }
      throw Exception('Network error: $e');
    }
  }

  // Generic PUT request
  Future<dynamic> put(String endpoint, Map<String, dynamic> data) async {
    try {
      final token = await getToken();
      final response = await http.put(
        Uri.parse('$baseUrl$endpoint'),
        headers: {
          'Content-Type': 'application/json',
          if (token != null) 'Authorization': 'Bearer $token',
        },
        body: json.encode(data),
      );

      if (response.statusCode == 200) {
        return json.decode(response.body);
      } else if (response.statusCode == 401) {
        await clearToken();
        throw Exception('Unauthorized');
      } else {
        throw Exception('Request failed: ${response.statusCode}');
      }
    } catch (e) {
      throw Exception('Network error: $e');
    }
  }

  // Generic DELETE request
  Future<dynamic> delete(String endpoint) async {
    try {
      final token = await getToken();
      final response = await http.delete(
        Uri.parse('$baseUrl$endpoint'),
        headers: {
          'Content-Type': 'application/json',
          if (token != null) 'Authorization': 'Bearer $token',
        },
      );

      if (response.statusCode == 200 || response.statusCode == 204) {
        if (response.body.isNotEmpty) {
          return json.decode(response.body);
        }
        return null;
      } else if (response.statusCode == 401) {
        await clearToken();
        throw Exception('Unauthorized');
      } else {
        throw Exception('Request failed: ${response.statusCode}');
      }
    } catch (e) {
      throw Exception('Network error: $e');
    }
  }
}
