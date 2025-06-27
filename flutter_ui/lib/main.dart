import 'package:flutter/material.dart';

void main() {
  runApp(const ClapJulesApp());
}

class ClapJulesApp extends StatelessWidget {
  const ClapJulesApp({super.key});

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      title: 'CLAP Jules Plugin UI',
      theme: ThemeData(
        colorScheme: ColorScheme.fromSeed(seedColor: Colors.deepPurple),
        useMaterial3: true,
      ),
      home: const PluginInterface(),
    );
  }
}

class PluginInterface extends StatefulWidget {
  const PluginInterface({super.key});

  @override
  State<PluginInterface> createState() => _PluginInterfaceState();
}

class _PluginInterfaceState extends State<PluginInterface> {
  double _gain = 0.5;
  bool _bypass = false;

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        backgroundColor: Theme.of(context).colorScheme.inversePrimary,
        title: const Text('CLAP Jules Plugin'),
      ),
      body: Padding(
        padding: const EdgeInsets.all(16.0),
        child: Column(
          mainAxisAlignment: MainAxisAlignment.center,
          children: &lt;Widget&gt;[
            const Text(
              'Plugin Controls',
              style: TextStyle(fontSize: 24, fontWeight: FontWeight.bold),
            ),
            const SizedBox(height: 32),
            Row(
              children: [
                const Text('Gain: '),
                Expanded(
                  child: Slider(
                    value: _gain,
                    onChanged: (value) {
                      setState(() {
                        _gain = value;
                      });
                      // TODO: Send gain value to CLAP plugin
                    },
                    min: 0.0,
                    max: 1.0,
                  ),
                ),
                Text('${(_gain * 100).round()}%'),
              ],
            ),
            const SizedBox(height: 16),
            Row(
              children: [
                const Text('Bypass: '),
                Switch(
                  value: _bypass,
                  onChanged: (value) {
                    setState(() {
                      _bypass = value;
                    });
                    // TODO: Send bypass state to CLAP plugin
                  },
                ),
              ],
            ),
            const SizedBox(height: 32),
            ElevatedButton(
              onPressed: () {
                // TODO: Close UI and notify plugin
                Navigator.of(context).pop();
              },
              child: const Text('Close'),
            ),
          ],
        ),
      ),
    );
  }
}