import 'package:flutter/material.dart';
import 'package:firebase_core/firebase_core.dart';
import 'firebase_options.dart';
import 'package:provider/provider.dart';

import 'providers/user_provider.dart';
import 'providers/telemetry_provider.dart';
import 'providers/settings_provider.dart';

import 'screens/auth/login.dart';
import 'screens/auth/register.dart';
import 'screens/home.dart';
import 'screens/live_tracking.dart';
import 'screens/history.dart';
import 'screens/settings.dart';
import 'screens/profile.dart';
import 'screens/bind_device_screen.dart';

void main() async {
  WidgetsFlutterBinding.ensureInitialized();
  
  // Initialize Firebase
  await Firebase.initializeApp(
    options: DefaultFirebaseOptions.currentPlatform,
  );
  
  runApp(const MyApp());
}

class MyApp extends StatelessWidget {
  const MyApp({super.key});

  @override
  Widget build(BuildContext context) {
    return MultiProvider(
      providers: [
        ChangeNotifierProvider(create: (_) => UserProvider()),
        ChangeNotifierProvider(create: (_) => TelemetryProvider()),
        ChangeNotifierProvider(create: (_) => SettingsProvider()),
      ],
      child: Consumer<UserProvider>(
        builder: (context, userProvider, _) {
          return MaterialApp(
            title: 'Accident Alert System',
            debugShowCheckedModeBanner: false,
            theme: ThemeData(
              colorScheme: ColorScheme.fromSeed(
                seedColor: const Color(0xFF6366F1),
                brightness: Brightness.light,
                primary: const Color(0xFF6366F1),
                secondary: const Color(0xFF8B5CF6),
              ),
              useMaterial3: true,
              scaffoldBackgroundColor: const Color(0xFFF8FAFC),
              appBarTheme: const AppBarTheme(
                centerTitle: true,
                elevation: 0,
                backgroundColor: Colors.white,
                foregroundColor: Color(0xFF1E293B),
              ),
              cardTheme: CardThemeData(
                elevation: 0,
                color: Colors.white,
                shape: RoundedRectangleBorder(
                  borderRadius: BorderRadius.circular(16),
                ),
                margin: const EdgeInsets.symmetric(vertical: 8),
              ),
              elevatedButtonTheme: ElevatedButtonThemeData(
                style: ElevatedButton.styleFrom(
                  elevation: 0,
                  padding: const EdgeInsets.symmetric(vertical: 16, horizontal: 24),
                  shape: RoundedRectangleBorder(
                    borderRadius: BorderRadius.circular(12),
                  ),
                ),
              ),
            ),
            home: userProvider.isAuthenticated 
                ? const HomeScreen() 
                : const LoginScreen(),
            routes: {
              '/login': (context) => const LoginScreen(),
              '/register': (context) => const RegisterScreen(),
              '/home': (context) => const HomeScreen(),
              '/tracking': (context) => const LiveTrackingScreen(),
              '/history': (context) => const HistoryScreen(),
              '/settings': (context) => const SettingsScreen(),
              '/profile': (context) => const ProfileScreen(),
              '/bind-device': (context) => const BindDeviceScreen(),
            },
          );
        },
      ),
    );
  }
}
