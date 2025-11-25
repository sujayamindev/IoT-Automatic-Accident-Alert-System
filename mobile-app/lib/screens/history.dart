import 'package:flutter/material.dart';
import 'package:provider/provider.dart';
import 'package:intl/intl.dart';
import '../providers/telemetry_provider.dart';
import '../providers/user_provider.dart';
import '../models/alert.dart';

class HistoryScreen extends StatefulWidget {
  const HistoryScreen({super.key});

  @override
  State<HistoryScreen> createState() => _HistoryScreenState();
}

class _HistoryScreenState extends State<HistoryScreen> {
  @override
  void initState() {
    super.initState();
    _loadData();
  }

  Future<void> _loadData() async {
    final telemetryProvider = Provider.of<TelemetryProvider>(context, listen: false);
    final userProvider = Provider.of<UserProvider>(context, listen: false);
    await telemetryProvider.fetchAlertsHistory(userProvider);
  }

  Future<void> _handleRefresh() async {
    await _loadData();
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        title: const Text('Alert History'),
      ),
      body: _buildAlertHistory(),
    );
  }

  Widget _buildAlertHistory() {
    final telemetryProvider = Provider.of<TelemetryProvider>(context);

    if (telemetryProvider.isLoading && telemetryProvider.alerts.isEmpty) {
      return const Center(child: CircularProgressIndicator());
    }

    if (telemetryProvider.alerts.isEmpty) {
      return const Center(child: Text('No alerts recorded yet'));
    }

    return RefreshIndicator(
      onRefresh: _handleRefresh,
      child: ListView.builder(
        padding: const EdgeInsets.all(8.0),
        itemCount: telemetryProvider.alerts.length,
        itemBuilder: (context, index) {
          final alert = telemetryProvider.alerts[index];
          return _buildAlertCard(alert);
        },
      ),
    );
  }

  Widget _buildAlertCard(Alert alert) {
    final dateFormat = DateFormat('MMM dd, yyyy HH:mm');
    Color color;
    IconData icon;

    switch (alert.type) {
      case AlertType.fire:
        color = Colors.red;
        icon = Icons.local_fire_department;
        break;
      case AlertType.smoke:
        color = Colors.orange;
        icon = Icons.cloud;
        break;
      case AlertType.vibration:
        color = Colors.purple;
        icon = Icons.vibration;
        break;
      case AlertType.accident:
        color = Colors.red;
        icon = Icons.car_crash;
        break;
    }

    return Card(
      margin: const EdgeInsets.symmetric(vertical: 4.0, horizontal: 8.0),
      child: ListTile(
        leading: CircleAvatar(
          backgroundColor: color.withOpacity(0.2),
          child: Icon(icon, color: color),
        ),
        title: Text(
          alert.typeString,
          style: TextStyle(
            fontWeight: FontWeight.bold,
            color: color,
          ),
        ),
        subtitle: Column(
          crossAxisAlignment: CrossAxisAlignment.start,
          children: [
            Text(dateFormat.format(alert.timestamp)),
            if (alert.description != null) ...[
              const SizedBox(height: 4),
              Text(alert.description!),
            ],
          ],
        ),
        trailing: alert.latitude != null && alert.longitude != null
            ? Icon(Icons.location_on, color: Colors.grey[600])
            : null,
      ),
    );
  }
}
