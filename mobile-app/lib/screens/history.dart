import 'package:flutter/material.dart';
import 'package:provider/provider.dart';
import 'package:intl/intl.dart';
import '../providers/telemetry_provider.dart';
import '../models/trip.dart';
import '../models/alert.dart';

class HistoryScreen extends StatefulWidget {
  const HistoryScreen({super.key});

  @override
  State<HistoryScreen> createState() => _HistoryScreenState();
}

class _HistoryScreenState extends State<HistoryScreen>
    with SingleTickerProviderStateMixin {
  late TabController _tabController;

  @override
  void initState() {
    super.initState();
    _tabController = TabController(length: 2, vsync: this);
    _loadData();
  }

  @override
  void dispose() {
    _tabController.dispose();
    super.dispose();
  }

  Future<void> _loadData() async {
    final telemetryProvider = Provider.of<TelemetryProvider>(context, listen: false);
    await Future.wait([
      telemetryProvider.fetchTripHistory(),
      telemetryProvider.fetchAlertsHistory(),
    ]);
  }

  Future<void> _handleRefresh() async {
    await _loadData();
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        title: const Text('History'),
        bottom: TabBar(
          controller: _tabController,
          tabs: const [
            Tab(text: 'Trips', icon: Icon(Icons.route)),
            Tab(text: 'Alerts', icon: Icon(Icons.warning)),
          ],
        ),
      ),
      body: TabBarView(
        controller: _tabController,
        children: [
          _buildTripHistory(),
          _buildAlertHistory(),
        ],
      ),
    );
  }

  Widget _buildTripHistory() {
    final telemetryProvider = Provider.of<TelemetryProvider>(context);

    if (telemetryProvider.isLoading && telemetryProvider.trips.isEmpty) {
      return const Center(child: CircularProgressIndicator());
    }

    if (telemetryProvider.trips.isEmpty) {
      return const Center(child: Text('No trips recorded yet'));
    }

    return RefreshIndicator(
      onRefresh: _handleRefresh,
      child: ListView.builder(
        padding: const EdgeInsets.all(8.0),
        itemCount: telemetryProvider.trips.length,
        itemBuilder: (context, index) {
          final trip = telemetryProvider.trips[index];
          return _buildTripCard(trip);
        },
      ),
    );
  }

  Widget _buildTripCard(Trip trip) {
    final dateFormat = DateFormat('MMM dd, yyyy');
    final timeFormat = DateFormat('HH:mm');

    return Card(
      margin: const EdgeInsets.symmetric(vertical: 4.0, horizontal: 8.0),
      child: ListTile(
        leading: const CircleAvatar(
          backgroundColor: Colors.blue,
          child: Icon(Icons.directions_car, color: Colors.white),
        ),
        title: Text(
          dateFormat.format(trip.startTime),
          style: const TextStyle(fontWeight: FontWeight.bold),
        ),
        subtitle: Column(
          crossAxisAlignment: CrossAxisAlignment.start,
          children: [
            Text('${timeFormat.format(trip.startTime)} - ${timeFormat.format(trip.endTime)}'),
            const SizedBox(height: 4),
            Row(
              children: [
                Icon(Icons.route, size: 14, color: Colors.grey[600]),
                const SizedBox(width: 4),
                Text('${trip.distance.toStringAsFixed(1)} km'),
                const SizedBox(width: 16),
                Icon(Icons.speed, size: 14, color: Colors.grey[600]),
                const SizedBox(width: 4),
                Text('Max: ${trip.maxSpeed.toStringAsFixed(0)} km/h'),
              ],
            ),
          ],
        ),
        trailing: const Icon(Icons.chevron_right),
        onTap: () {
          Navigator.of(context).pushNamed('/trip-details', arguments: trip.id);
        },
      ),
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
