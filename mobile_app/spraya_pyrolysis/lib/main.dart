import 'package:flutter/material.dart';
import 'package:get/get.dart';
import 'package:spraya_pyrolysis/inital_binding.dart';
import 'package:spraya_pyrolysis/local_api.dart';

void main() {
  runApp(const MyApp());
}

class MyApp extends StatelessWidget {
  const MyApp({Key? key}) : super(key: key);

  @override
  Widget build(BuildContext context) {
    return GetMaterialApp(
      debugShowCheckedModeBanner: false,
      title: 'Spray Pyrolysis',
      home: MyHomePage(),
      initialBinding: InitialBinding(),
    );
  }
}

class MyHomePage extends StatefulWidget {
  const MyHomePage({Key? key}) : super(key: key);

  @override
  _MyHomePageState createState() => _MyHomePageState();
}

class _MyHomePageState extends State<MyHomePage> {
  bool _isRunning = false;

  final APIController _apiController = Get.find();

  void _startAction() {
    _apiController.getStatus();
    // _apiController.startProcess();
    setState(() {
      _isRunning = true;
    });
  }

  void _stopAction() {
    _apiController.stopProcess();
    setState(() {
      _isRunning = false;
    });
  }

  @override
  Widget build(BuildContext context) {
    return SafeArea(
      child: Scaffold(
        drawer: Drawer(
          child: ListView(
            children: <Widget>[
              DrawerHeader(
                child: Text('Spray Pyrolysis',
                    style: TextStyle(
                        color: Colors.white,
                        fontSize: 24,
                        fontWeight: FontWeight.bold)),
                decoration: BoxDecoration(
                  color: Colors.blue,
                ),
              ),
              ListTile(
                leading: Icon(Icons.thermostat),
                title: Text('Temperature'),
                onTap: () {},
              ),
              ListTile(
                leading: Icon(Icons.water),
                title: Text('Pump'),
                onTap: () {},
              ),
              ListTile(
                leading: Icon(Icons.speed),
                title: Text('Plotter Speed'),
                onTap: () {},
              ),
            ],
          ),
        ),
        appBar: AppBar(
          title: Text('Spray Pyrolysis'),
        ),
        body: Center(
          child: Column(
            mainAxisAlignment: MainAxisAlignment.spaceAround,
            children: <Widget>[
              Text(
                _isRunning ? 'Running' : 'Stopped',
                style: Theme.of(context).textTheme.headline4,
              ),
              Row(
                mainAxisAlignment: MainAxisAlignment.spaceAround,
                children: [
                  iconButton(
                    "Start",
                    Icons.run_circle_outlined,
                    _startAction,
                    _isRunning ? Colors.grey.shade600 : Colors.green,
                  ),
                  iconButton(
                    "Stop",
                    Icons.stop_circle_outlined,
                    _stopAction,
                    !_isRunning ? Colors.grey.shade600 : Colors.redAccent,
                  ),
                ],
              ),
            ],
          ),
        ),
      ),
    );
  }

  Column iconButton(String title, IconData icon, Function action, Color color) {
    return Column(
      children: [
        ElevatedButton(
          onPressed: () {
            action();
          },
          child: Icon(icon, color: Colors.white, size: 100),
          style: ElevatedButton.styleFrom(
            elevation: 5,
            shape: CircleBorder(),
            padding: EdgeInsets.all(2),
            primary: color,
            // <-- Splash color
          ),
        ),
        SizedBox(height: 10),
        Text(title, style: Theme.of(context).textTheme.headline6),
      ],
    );
  }
}
