import 'package:flutter/material.dart';
import 'package:provider/provider.dart';
import 'package:flutter_map/flutter_map.dart';
import 'package:latlong2/latlong.dart';
import '../providers/telemetry_provider.dart';

class LiveTrackingScreen extends StatefulWidget {
  const LiveTrackingScreen({super.key});

  @override
  State<LiveTrackingScreen> createState() => _LiveTrackingScreenState();
}

class _LiveTrackingScreenState extends State<LiveTrackingScreen> {
  final MapController _mapController = MapController();

  @override
  void initState() {
    super.initState();
    _loadData();
  }

  Future<void> _loadData() async {
    final telemetryProvider = Provider.of<TelemetryProvider>(context, listen: false);
    await telemetryProvider.fetchLiveData();
  }

  @override
  Widget build(BuildContext context) {
    final telemetryProvider = Provider.of<TelemetryProvider>(context);
    final telemetry = telemetryProvider.currentTelemetry;

    return Scaffold(
      appBar: AppBar(
        title: const Text('Live Tracking'),
        actions: [
          IconButton(
            icon: const Icon(Icons.refresh),
            onPressed: _loadData,
          ),
        ],
      ),
      body: telemetryProvider.isLoading && telemetry == null
          ? const Center(child: CircularProgressIndicator())
          : Column(
              children: [
                // Map View
                Expanded(
                  flex: 2,
                  child: telemetry != null
                      ? FlutterMap(
                          mapController: _mapController,
                          options: MapOptions(
                            initialCenter: LatLng(telemetry.latitude, telemetry.longitude),
                            initialZoom: 15.0,
                          ),
                          children: [
                            TileLayer(
                              urlTemplate: 'https://tile.openstreetmap.org/{z}/{x}/{y}.png',
                              userAgentPackageName: 'com.iotaccidentalert.accident_alert_app',
                            ),
                            MarkerLayer(
                              markers: [
                                Marker(
                                  point: LatLng(telemetry.latitude, telemetry.longitude),
                                  width: 80,
                                  height: 80,
                                  child: const Icon(
                                    Icons.location_pin,
                                    color: Colors.red,
                                    size: 40,
                                  ),
                                ),
                              ],
                            ),
                          ],
                        )
                      : const Center(
                          child: Text('No location data available'),
                        ),
                ),

                // Telemetry Data
                Expanded(
                  flex: 1,
                  child: Container(
                    padding: const EdgeInsets.all(16.0),
                    decoration: BoxDecoration(
                      color: Colors.white,
                      boxShadow: [
                        BoxShadow(
                          color: Colors.grey.withOpacity(0.3),
                          spreadRadius: 2,
                          blurRadius: 5,
                          offset: const Offset(0, -3),
                        ),
                      ],
                    ),
                    child: telemetry != null
                        ? Column(
                            crossAxisAlignment: CrossAxisAlignment.start,
                            children: [
                              Text(
                                'Telemetry Data',
                                style: Theme.of(context).textTheme.titleLarge,
                              ),
                              const SizedBox(height: 16),
                              Row(
                                mainAxisAlignment: MainAxisAlignment.spaceAround,
                                children: [
                                  _buildDataItem(
                                    'Speed',
                                    '${telemetry.speed.toStringAsFixed(1)} km/h',
                                    Icons.speed,
                                  ),
                                  _buildDataItem(
                                    'Battery',
                                    '${telemetry.batteryPercentage}%',
                                    Icons.battery_full,
                                  ),
                                  _buildDataItem(
                                    'Signal',
                                    '${telemetry.gsmSignalLevel}',
                                    Icons.signal_cellular_alt,
                                  ),
                                ],
                              ),
                              const SizedBox(height: 16),
                              Row(
                                mainAxisAlignment: MainAxisAlignment.spaceAround,
                                children: [
                                  _buildAlertIndicator(
                                    'Fire',
                                    telemetry.fireDetected,
                                    Icons.local_fire_department,
                                    Colors.red,
                                  ),
                                  _buildAlertIndicator(
                                    'Smoke',
                                    telemetry.smokeDetected,
                                    Icons.cloud,
                                    Colors.orange,
                                  ),
                                  _buildAlertIndicator(
                                    'Vibration',
                                    telemetry.vibrationDetected,
                                    Icons.vibration,
                                    Colors.purple,
                                  ),
                                ],
                              ),
                            ],
                          )
                        : const Center(
                            child: Text('No data available'),
                          ),
                  ),
                ),
              ],
            ),
    );
  }

  Widget _buildDataItem(String label, String value, IconData icon) {
    return Column(
      children: [
        Icon(icon, color: Colors.blue),
        const SizedBox(height: 4),
        Text(
          value,
          style: const TextStyle(
            fontSize: 16,
            fontWeight: FontWeight.bold,
          ),
        ),
        Text(
          label,
          style: TextStyle(
            fontSize: 12,
            color: Colors.grey[600],
          ),
        ),
      ],
    );
  }

  Widget _buildAlertIndicator(String label, bool isActive, IconData icon, Color color) {
    return Column(
      children: [
        Icon(
          icon,
          color: isActive ? color : Colors.grey,
          size: 24,
        ),
        const SizedBox(height: 4),
        Text(
          label,
          style: TextStyle(
            fontSize: 12,
            color: isActive ? color : Colors.grey,
          ),
        ),
      ],
    );
  }
}
