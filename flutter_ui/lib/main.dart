import 'package:flutter/material.dart';

void main() {
  runApp(const MyApp());
}

class MyApp extends StatelessWidget {
  const MyApp({super.key});

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      title: 'CLAP Jules Plugin UI',
      theme: ThemeData(
        colorScheme: ColorScheme.fromSeed(seedColor: Colors.deepPurple),
        useMaterial3: true,
      ),
      home: const ClapPluginInterface(title: 'CLAP Jules Plugin'),
    );
  }
}

class ClapPluginInterface extends StatefulWidget {
  const ClapPluginInterface({super.key, required this.title});

  final String title;

  @override
  State<ClapPluginInterface> createState() => _ClapPluginInterfaceState();
}

class _ClapPluginInterfaceState extends State<ClapPluginInterface> {
  double _volume = 0.5;
  double _frequency = 440.0;
  bool _enabled = true;

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        backgroundColor: Theme.of(context).colorScheme.inversePrimary,
        title: Text(widget.title),
      ),
      body: Padding(
        padding: const EdgeInsets.all(16.0),
        child: Column(
          crossAxisAlignment: CrossAxisAlignment.start,
          children: <Widget>[
            const Text(
              'CLAP Jules Plugin Control Interface',
              style: TextStyle(fontSize: 20, fontWeight: FontWeight.bold),
            ),
            const SizedBox(height: 20),
            
            // Plugin enabled switch
            Row(
              mainAxisAlignment: MainAxisAlignment.spaceBetween,
              children: [
                const Text('Plugin Enabled:', style: TextStyle(fontSize: 16)),
                Switch(
                  value: _enabled,
                  onChanged: (bool value) {
                    setState(() {
                      _enabled = value;
                    });
                  },
                ),
              ],
            ),
            const SizedBox(height: 20),
            
            // Volume control
            const Text('Volume:', style: TextStyle(fontSize: 16)),
            Slider(
              value: _volume,
              min: 0.0,
              max: 1.0,
              divisions: 100,
              label: '${(_volume * 100).round()}%',
              onChanged: (double value) {
                setState(() {
                  _volume = value;
                });
              },
            ),
            Text('${(_volume * 100).round()}%', 
                 style: const TextStyle(fontSize: 14)),
            const SizedBox(height: 20),
            
            // Frequency control
            const Text('Frequency:', style: TextStyle(fontSize: 16)),
            Slider(
              value: _frequency,
              min: 20.0,
              max: 20000.0,
              divisions: 1000,
              label: '${_frequency.round()} Hz',
              onChanged: (double value) {
                setState(() {
                  _frequency = value;
                });
              },
            ),
            Text('${_frequency.round()} Hz', 
                 style: const TextStyle(fontSize: 14)),
            const SizedBox(height: 30),
            
            // Info section
            Card(
              child: Padding(
                padding: const EdgeInsets.all(16.0),
                child: Column(
                  crossAxisAlignment: CrossAxisAlignment.start,
                  children: [
                    const Text('Plugin Info:', 
                              style: TextStyle(fontSize: 16, fontWeight: FontWeight.bold)),
                    const SizedBox(height: 8),
                    const Text('Name: My First CLAP Plugin'),
                    const Text('Version: 0.0.1'),
                    const Text('Vendor: My Company'),
                    Text('Status: ${_enabled ? 'Active' : 'Inactive'}'),
                  ],
                ),
              ),
            ),
          ],
        ),
      ),
    );
  }
}
