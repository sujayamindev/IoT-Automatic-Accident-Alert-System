import 'package:flutter/material.dart';
import 'package:provider/provider.dart';
import '../providers/settings_provider.dart';
import '../models/settings.dart';

class SettingsScreen extends StatefulWidget {
  const SettingsScreen({super.key});

  @override
  State<SettingsScreen> createState() => _SettingsScreenState();
}

class _SettingsScreenState extends State<SettingsScreen> {
  @override
  void initState() {
    super.initState();
    _loadSettings();
  }

  Future<void> _loadSettings() async {
    final settingsProvider = Provider.of<SettingsProvider>(context, listen: false);
    await settingsProvider.fetchSettings();
  }

  @override
  Widget build(BuildContext context) {
    final settingsProvider = Provider.of<SettingsProvider>(context);
    final settings = settingsProvider.settings;

    return Scaffold(
      appBar: AppBar(
        title: const Text('Settings'),
      ),
      body: settingsProvider.isLoading && settings == null
          ? const Center(child: CircularProgressIndicator())
          : settings == null
              ? const Center(child: Text('Failed to load settings'))
              : ListView(
                  padding: const EdgeInsets.all(16.0),
                  children: [
                    // Device Name
                    Card(
                      child: ListTile(
                        leading: const Icon(Icons.devices),
                        title: const Text('Device Name'),
                        subtitle: Text(settings.deviceName),
                        trailing: const Icon(Icons.edit),
                        onTap: () => _editDeviceName(context, settings.deviceName),
                      ),
                    ),
                    const SizedBox(height: 16),

                    // Upload Interval
                    Card(
                      child: ListTile(
                        leading: const Icon(Icons.upload),
                        title: const Text('Upload Interval'),
                        subtitle: Text('${settings.uploadInterval} seconds'),
                        trailing: DropdownButton<int>(
                          value: settings.uploadInterval,
                          items: [3, 5, 10].map((int value) {
                            return DropdownMenuItem<int>(
                              value: value,
                              child: Text('$value s'),
                            );
                          }).toList(),
                          onChanged: (value) {
                            if (value != null) {
                              settingsProvider.updateUploadInterval(value);
                            }
                          },
                        ),
                      ),
                    ),
                    const SizedBox(height: 16),

                    // Sensor Toggles
                    Text(
                      'Sensor Detection',
                      style: Theme.of(context).textTheme.titleMedium,
                    ),
                    const SizedBox(height: 8),
                    Card(
                      child: Column(
                        children: [
                          SwitchListTile(
                            secondary: const Icon(Icons.local_fire_department),
                            title: const Text('Fire Detection'),
                            value: settings.fireDetectionEnabled,
                            onChanged: (value) {
                              settingsProvider.toggleFireDetection(value);
                            },
                          ),
                          const Divider(height: 1),
                          SwitchListTile(
                            secondary: const Icon(Icons.cloud),
                            title: const Text('Smoke Detection'),
                            value: settings.smokeDetectionEnabled,
                            onChanged: (value) {
                              settingsProvider.toggleSmokeDetection(value);
                            },
                          ),
                          const Divider(height: 1),
                          SwitchListTile(
                            secondary: const Icon(Icons.vibration),
                            title: const Text('Vibration Detection'),
                            value: settings.vibrationDetectionEnabled,
                            onChanged: (value) {
                              settingsProvider.toggleVibrationDetection(value);
                            },
                          ),
                        ],
                      ),
                    ),
                    const SizedBox(height: 16),

                    // Emergency Contacts
                    Row(
                      mainAxisAlignment: MainAxisAlignment.spaceBetween,
                      children: [
                        Text(
                          'Emergency Contacts',
                          style: Theme.of(context).textTheme.titleMedium,
                        ),
                        IconButton(
                          icon: const Icon(Icons.add),
                          onPressed: () => _addContact(context),
                        ),
                      ],
                    ),
                    const SizedBox(height: 8),
                    if (settings.emergencyContacts.isEmpty)
                      const Card(
                        child: Padding(
                          padding: EdgeInsets.all(16.0),
                          child: Text(
                            'No emergency contacts added yet',
                            textAlign: TextAlign.center,
                          ),
                        ),
                      )
                    else
                      ...settings.emergencyContacts.map((contact) {
                        return Card(
                          child: ListTile(
                            leading: CircleAvatar(
                              backgroundColor: contact.isPrimary
                                  ? Colors.blue
                                  : Colors.grey,
                              child: const Icon(Icons.person, color: Colors.white),
                            ),
                            title: Text(contact.name),
                            subtitle: Text(contact.phoneNumber),
                            trailing: IconButton(
                              icon: const Icon(Icons.delete),
                              onPressed: () {
                                _deleteContact(context, contact);
                              },
                            ),
                            onTap: () => _editContact(context, contact),
                          ),
                        );
                      }),
                  ],
                ),
    );
  }

  Future<void> _editDeviceName(BuildContext context, String currentName) async {
    final controller = TextEditingController(text: currentName);
    final result = await showDialog<String>(
      context: context,
      builder: (context) => AlertDialog(
        title: const Text('Edit Device Name'),
        content: TextField(
          controller: controller,
          decoration: const InputDecoration(
            labelText: 'Device Name',
          ),
        ),
        actions: [
          TextButton(
            onPressed: () => Navigator.pop(context),
            child: const Text('Cancel'),
          ),
          TextButton(
            onPressed: () => Navigator.pop(context, controller.text),
            child: const Text('Save'),
          ),
        ],
      ),
    );

    if (result != null && result.isNotEmpty && mounted) {
      final settingsProvider = Provider.of<SettingsProvider>(context, listen: false);
      await settingsProvider.updateDeviceName(result);
    }
  }

  Future<void> _addContact(BuildContext context) async {
    final nameController = TextEditingController();
    final phoneController = TextEditingController();

    final result = await showDialog<Map<String, dynamic>>(
      context: context,
      builder: (context) => AlertDialog(
        title: const Text('Add Emergency Contact'),
        content: Column(
          mainAxisSize: MainAxisSize.min,
          children: [
            TextField(
              controller: nameController,
              decoration: const InputDecoration(
                labelText: 'Name',
              ),
            ),
            const SizedBox(height: 16),
            TextField(
              controller: phoneController,
              keyboardType: TextInputType.phone,
              decoration: const InputDecoration(
                labelText: 'Phone Number',
              ),
            ),
          ],
        ),
        actions: [
          TextButton(
            onPressed: () => Navigator.pop(context),
            child: const Text('Cancel'),
          ),
          TextButton(
            onPressed: () => Navigator.pop(context, {
              'name': nameController.text,
              'phone': phoneController.text,
            }),
            child: const Text('Add'),
          ),
        ],
      ),
    );

    if (result != null && mounted) {
      final settingsProvider = Provider.of<SettingsProvider>(context, listen: false);
      await settingsProvider.addContact(result['name'], result['phone']);
    }
  }

  Future<void> _editContact(BuildContext context, EmergencyContact contact) async {
    final nameController = TextEditingController(text: contact.name);
    final phoneController = TextEditingController(text: contact.phoneNumber);

    final result = await showDialog<Map<String, dynamic>>(
      context: context,
      builder: (context) => AlertDialog(
        title: const Text('Edit Contact'),
        content: Column(
          mainAxisSize: MainAxisSize.min,
          children: [
            TextField(
              controller: nameController,
              decoration: const InputDecoration(
                labelText: 'Name',
              ),
            ),
            const SizedBox(height: 16),
            TextField(
              controller: phoneController,
              keyboardType: TextInputType.phone,
              decoration: const InputDecoration(
                labelText: 'Phone Number',
              ),
            ),
          ],
        ),
        actions: [
          TextButton(
            onPressed: () => Navigator.pop(context),
            child: const Text('Cancel'),
          ),
          TextButton(
            onPressed: () => Navigator.pop(context, {
              'name': nameController.text,
              'phone': phoneController.text,
            }),
            child: const Text('Save'),
          ),
        ],
      ),
    );

    if (result != null && mounted) {
      final settingsProvider = Provider.of<SettingsProvider>(context, listen: false);
      final updatedContact = contact.copyWith(
        name: result['name'],
        phoneNumber: result['phone'],
      );
      await settingsProvider.updateContact(updatedContact);
    }
  }

  Future<void> _deleteContact(BuildContext context, EmergencyContact contact) async {
    final confirmed = await showDialog<bool>(
      context: context,
      builder: (context) => AlertDialog(
        title: const Text('Delete Contact'),
        content: Text('Are you sure you want to delete ${contact.name}?'),
        actions: [
          TextButton(
            onPressed: () => Navigator.pop(context, false),
            child: const Text('Cancel'),
          ),
          TextButton(
            onPressed: () => Navigator.pop(context, true),
            child: const Text('Delete', style: TextStyle(color: Colors.red)),
          ),
        ],
      ),
    );

    if (confirmed == true && mounted) {
      final settingsProvider = Provider.of<SettingsProvider>(context, listen: false);
      await settingsProvider.deleteContact(contact.id);
    }
  }
}
