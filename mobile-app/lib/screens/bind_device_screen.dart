import 'package:flutter/material.dart';
import 'package:provider/provider.dart';
import '../services/telemetry_service.dart';
import '../providers/user_provider.dart';

class BindDeviceScreen extends StatefulWidget {
  const BindDeviceScreen({Key? key}) : super(key: key);

  @override
  State<BindDeviceScreen> createState() => _BindDeviceScreenState();
}

class _BindDeviceScreenState extends State<BindDeviceScreen> {
  final _formKey = GlobalKey<FormState>();
  final _macController = TextEditingController();
  final _nameController = TextEditingController();
  final _telemetryService = TelemetryService();
  bool _isLoading = false;

  @override
  void dispose() {
    _macController.dispose();
    _nameController.dispose();
    super.dispose();
  }

  Future<void> _bindDevice() async {
    if (!_formKey.currentState!.validate()) return;

    setState(() => _isLoading = true);

    try {
      final macAddress = _macController.text.trim().toUpperCase();
      final deviceName = _nameController.text.trim();

      print('Attempting to bind device: $macAddress');
      
      await _telemetryService.bindDevice(
        macAddress,
        deviceName: deviceName.isEmpty ? null : deviceName,
      ).timeout(
        const Duration(seconds: 30),
        onTimeout: () {
          throw Exception('Connection timeout. Please check your network and backend.');
        },
      );

      print('Device bound successfully');
      
      if (mounted) {
        // Store bound device in user provider
        final userProvider = Provider.of<UserProvider>(context, listen: false);
        userProvider.setBoundDevice(macAddress, deviceName.isEmpty ? 'My Device' : deviceName);
        
        ScaffoldMessenger.of(context).showSnackBar(
          const SnackBar(
            content: Text('Device bound successfully!'),
            backgroundColor: Colors.green,
          ),
        );
        Navigator.pop(context, true);
      }
    } catch (e) {
      print('Bind device error: $e');
      if (mounted) {
        ScaffoldMessenger.of(context).showSnackBar(
          SnackBar(
            content: Text('Failed to bind device: $e'),
            backgroundColor: Colors.red,
            duration: const Duration(seconds: 5),
          ),
        );
      }
    } finally {
      if (mounted) setState(() => _isLoading = false);
    }
  }

  String? _validateMacAddress(String? value) {
    if (value == null || value.isEmpty) {
      return 'Please enter MAC address';
    }

    // Remove colons and check format
    final cleanMac = value.replaceAll(':', '').toUpperCase();
    if (cleanMac.length != 12) {
      return 'MAC address must be 12 hex digits';
    }

    // Check if valid hex
    final hexPattern = RegExp(r'^[0-9A-F]{12}$');
    if (!hexPattern.hasMatch(cleanMac)) {
      return 'Invalid MAC address format';
    }

    return null;
  }

  String _formatMacAddress(String value) {
    // Remove existing colons
    final clean = value.replaceAll(':', '').toUpperCase();
    
    // Add colons every 2 characters
    final buffer = StringBuffer();
    for (int i = 0; i < clean.length; i++) {
      if (i > 0 && i % 2 == 0 && i < 12) {
        buffer.write(':');
      }
      buffer.write(clean[i]);
    }
    return buffer.toString();
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      backgroundColor: const Color(0xFFF8FAFC),
      appBar: AppBar(
        title: const Text(
          'Bind Device',
          style: TextStyle(fontWeight: FontWeight.bold),
        ),
        elevation: 0,
        backgroundColor: Colors.white,
        foregroundColor: const Color(0xFF1E293B),
      ),
      body: SingleChildScrollView(
        padding: const EdgeInsets.all(20.0),
        child: Form(
          key: _formKey,
          child: Column(
            crossAxisAlignment: CrossAxisAlignment.stretch,
            children: [
              // Hero Card
              Container(
                decoration: BoxDecoration(
                  gradient: const LinearGradient(
                    colors: [Color(0xFF6366F1), Color(0xFF8B5CF6)],
                    begin: Alignment.topLeft,
                    end: Alignment.bottomRight,
                  ),
                  borderRadius: BorderRadius.circular(20),
                ),
                padding: const EdgeInsets.all(32.0),
                child: Column(
                  children: [
                    Container(
                      padding: const EdgeInsets.all(16),
                      decoration: BoxDecoration(
                        color: Colors.white.withOpacity(0.2),
                        shape: BoxShape.circle,
                      ),
                      child: const Icon(
                        Icons.devices,
                        size: 48,
                        color: Colors.white,
                      ),
                    ),
                    const SizedBox(height: 16),
                    const Text(
                      'Add Your IoT Device',
                      textAlign: TextAlign.center,
                      style: TextStyle(
                        fontSize: 24,
                        fontWeight: FontWeight.bold,
                        color: Colors.white,
                      ),
                    ),
                    const SizedBox(height: 8),
                    const Text(
                      'Connect your ESP32 device to start monitoring',
                      textAlign: TextAlign.center,
                      style: TextStyle(
                        fontSize: 14,
                        color: Colors.white70,
                      ),
                    ),
                  ],
                ),
              ),
              const SizedBox(height: 24),
              
              // Form Card
              Container(
                decoration: BoxDecoration(
                  color: Colors.white,
                  borderRadius: BorderRadius.circular(16),
                  border: Border.all(color: const Color(0xFFE2E8F0)),
                ),
                padding: const EdgeInsets.all(20),
                child: Column(
                  crossAxisAlignment: CrossAxisAlignment.stretch,
                  children: [
                    // MAC Address Field
                    TextFormField(
                      controller: _macController,
                      decoration: InputDecoration(
                        labelText: 'MAC Address',
                        labelStyle: const TextStyle(
                          color: Color(0xFF64748B),
                          fontSize: 14,
                          fontWeight: FontWeight.w500,
                        ),
                        hintText: '4C:C3:82:0C:7E:CC',
                        hintStyle: TextStyle(color: Colors.grey.shade400),
                        prefixIcon: const Icon(
                          Icons.wifi,
                          color: Color(0xFF6366F1),
                        ),
                        border: OutlineInputBorder(
                          borderRadius: BorderRadius.circular(12),
                          borderSide: const BorderSide(color: Color(0xFFE2E8F0)),
                        ),
                        enabledBorder: OutlineInputBorder(
                          borderRadius: BorderRadius.circular(12),
                          borderSide: const BorderSide(color: Color(0xFFE2E8F0)),
                        ),
                        focusedBorder: OutlineInputBorder(
                          borderRadius: BorderRadius.circular(12),
                          borderSide: const BorderSide(
                            color: Color(0xFF6366F1),
                            width: 2,
                          ),
                        ),
                        filled: true,
                        fillColor: const Color(0xFFF8FAFC),
                        helperText: 'Format: XX:XX:XX:XX:XX:XX or XXXXXXXXXXXX',
                        helperStyle: const TextStyle(
                          fontSize: 12,
                          color: Color(0xFF94A3B8),
                        ),
                      ),
                      style: const TextStyle(
                        fontSize: 15,
                        fontWeight: FontWeight.w500,
                        color: Color(0xFF1E293B),
                      ),
                      textCapitalization: TextCapitalization.characters,
                      maxLength: 17,
                      validator: _validateMacAddress,
                    ),
                    const SizedBox(height: 16),
                    
                    // Device Name Field (Optional)
                    TextFormField(
                      controller: _nameController,
                      decoration: InputDecoration(
                        labelText: 'Device Name (Optional)',
                        labelStyle: const TextStyle(
                          color: Color(0xFF64748B),
                          fontSize: 14,
                          fontWeight: FontWeight.w500,
                        ),
                        hintText: 'My Car',
                        hintStyle: TextStyle(color: Colors.grey.shade400),
                        prefixIcon: const Icon(
                          Icons.label_outline,
                          color: Color(0xFF6366F1),
                        ),
                        border: OutlineInputBorder(
                          borderRadius: BorderRadius.circular(12),
                          borderSide: const BorderSide(color: Color(0xFFE2E8F0)),
                        ),
                        enabledBorder: OutlineInputBorder(
                          borderRadius: BorderRadius.circular(12),
                          borderSide: const BorderSide(color: Color(0xFFE2E8F0)),
                        ),
                        focusedBorder: OutlineInputBorder(
                          borderRadius: BorderRadius.circular(12),
                          borderSide: const BorderSide(
                            color: Color(0xFF6366F1),
                            width: 2,
                          ),
                        ),
                        filled: true,
                        fillColor: const Color(0xFFF8FAFC),
                      ),
                      style: const TextStyle(
                        fontSize: 15,
                        fontWeight: FontWeight.w500,
                        color: Color(0xFF1E293B),
                      ),
                      textCapitalization: TextCapitalization.words,
                    ),
                  ],
                ),
              ),
              const SizedBox(height: 20),
              
              // Info Card
              Container(
                padding: const EdgeInsets.all(16),
                decoration: BoxDecoration(
                  color: const Color(0xFF6366F1).withOpacity(0.05),
                  borderRadius: BorderRadius.circular(12),
                  border: Border.all(
                    color: const Color(0xFF6366F1).withOpacity(0.2),
                  ),
                ),
                child: Column(
                  crossAxisAlignment: CrossAxisAlignment.start,
                  children: const [
                    Row(
                      children: [
                        Icon(
                          Icons.info_outline,
                          color: Color(0xFF6366F1),
                          size: 20,
                        ),
                        SizedBox(width: 8),
                        Text(
                          'How to find MAC address:',
                          style: TextStyle(
                            fontWeight: FontWeight.bold,
                            fontSize: 14,
                            color: Color(0xFF1E293B),
                          ),
                        ),
                      ],
                    ),
                    SizedBox(height: 12),
                    Text(
                      '1. Power on your ESP32 device',
                      style: TextStyle(fontSize: 13, color: Color(0xFF475569)),
                    ),
                    SizedBox(height: 4),
                    Text(
                      '2. Open Arduino Serial Monitor (115200 baud)',
                      style: TextStyle(fontSize: 13, color: Color(0xFF475569)),
                    ),
                    SizedBox(height: 4),
                    Text(
                      '3. Look for "Device MAC: XX:XX:XX:XX:XX:XX"',
                      style: TextStyle(fontSize: 13, color: Color(0xFF475569)),
                    ),
                    SizedBox(height: 4),
                    Text(
                      '4. Copy and paste it here',
                      style: TextStyle(fontSize: 13, color: Color(0xFF475569)),
                    ),
                  ],
                ),
              ),
              const SizedBox(height: 24),
              
              // Bind Button
              ElevatedButton(
                onPressed: _isLoading ? null : _bindDevice,
                style: ElevatedButton.styleFrom(
                  backgroundColor: const Color(0xFF6366F1),
                  foregroundColor: Colors.white,
                  padding: const EdgeInsets.symmetric(vertical: 16),
                  shape: RoundedRectangleBorder(
                    borderRadius: BorderRadius.circular(12),
                  ),
                  elevation: 0,
                ),
                child: _isLoading
                    ? const SizedBox(
                        height: 20,
                        width: 20,
                        child: CircularProgressIndicator(
                          strokeWidth: 2,
                          valueColor: AlwaysStoppedAnimation<Color>(Colors.white),
                        ),
                      )
                    : const Text(
                        'Bind Device',
                        style: TextStyle(
                          fontSize: 16,
                          fontWeight: FontWeight.w600,
                        ),
                      ),
              ),
            ],
          ),
        ),
      ),
    );
  }
}
