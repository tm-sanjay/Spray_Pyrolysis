import 'dart:async';

import 'package:flutter/material.dart';
import 'package:get/get.dart';
import 'package:spraya_pyrolysis/inital_binding.dart';
import 'package:spraya_pyrolysis/local_api.dart';
import 'package:spraya_pyrolysis/number_widget.dart';

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
  String _statusTitle = 'Ready to Use';

  int ssrOnTime = 5;
  int plotterSpeed = 1;
  int pumpSpeed = 70;
  int temprature = 400;

  final APIController _apiController = Get.find();

  late Timer _timer;
  int _timeCounter = 0;

  //start the timer
  void _startTimer() {
    _timeCounter = 0;
    _timer = Timer.periodic(Duration(seconds: 1), (timer) {
      setState(() {
        _timeCounter++;
      });
    });
  }

  //stop timer
  void _stopTimer() {
    _timer.cancel();
  }

  void _startAction() {
    //start timer
    _startTimer();
    _apiController.startProcess();
    setState(() {
      _isRunning = true;
      _statusTitle = 'Processing...';
    });
  }

  void _stopAction() {
    _stopTimer();
    _apiController.stopProcess();
    setState(() {
      _isRunning = false;
      _statusTitle = 'Time taken: $_timeCounter sec';
    });
  }

  @override
  Widget build(BuildContext context) {
    return SafeArea(
      child: Scaffold(
        drawer: Drawer(
          child: ListView(
            physics: NeverScrollableScrollPhysics(),
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
                leading: Icon(Icons.timeline_sharp),
                title: Text('SSR On Time'),
                onTap: onSSROnTimePressed,
              ),
              ListTile(
                leading: Icon(Icons.thermostat),
                title: Text('Temperature'),
                onTap: onTempPressed,
              ),
              ListTile(
                leading: Icon(Icons.water),
                title: Text('Liquid Speed'),
                onTap: onLiquidSpeedPressed,
              ),
              ListTile(
                leading: Icon(Icons.speed),
                title: Text('Plotter Speed'),
                onTap: onPlotterSpeedPressed,
              ),
            ],
          ),
        ),
        appBar: AppBar(
          title: Text('Spray Pyrolysis'),
        ),
        body: Stack(
          children: [
            _statusTitle == "Ready to Use"
                ? Image(image: AssetImage('assets/bg.jpeg'))
                : Container(),
            Column(
              mainAxisAlignment: MainAxisAlignment.center,
              children: <Widget>[
                _statusTitle != "Ready to Use"
                    ? Container(
                        padding: const EdgeInsets.all(8),
                        decoration: BoxDecoration(
                          borderRadius: BorderRadius.circular(10),
                          color: _isRunning
                              ? Color.fromARGB(204, 255, 82, 82)
                              : Color.fromARGB(118, 33, 149, 243),
                        ),
                        child: Text(
                          _statusTitle,
                          style: TextStyle(
                            color: Colors.white,
                            fontSize: 32,
                            fontWeight: FontWeight.w900,
                          ),
                        ),
                      )
                    : Container(),
                _isRunning
                    ? Container(
                        padding: const EdgeInsets.all(0),
                        height: MediaQuery.of(context).size.height * 0.25,
                        child: Column(
                          mainAxisAlignment: MainAxisAlignment.spaceEvenly,
                          crossAxisAlignment: CrossAxisAlignment.center,
                          children: [
                            Text('Plotter Speed: $plotterSpeed cm/s',
                                style: TextStyle(
                                  fontSize: 20,
                                  fontFamily: 'Roboto',
                                  // fontWeight: FontWeight.bold,
                                )),
                            Text('Liquid Speed: $pumpSpeed ml/min',
                                style: TextStyle(
                                  fontSize: 20,
                                  // fontWeight: FontWeight.bold,
                                )),
                            Text('Temperature: $temprature' '°C',
                                style: TextStyle(
                                  fontSize: 20,
                                  // fontWeight: FontWeight.bold,
                                )),
                          ],
                        ),
                      )
                    : Container(
                        padding: const EdgeInsets.all(0),
                        height: MediaQuery.of(context).size.height * 0.25,
                      ),
                _statusTitle == "Ready to Use"
                    ? Container(
                        padding: const EdgeInsets.all(8),
                        decoration: BoxDecoration(
                          borderRadius: BorderRadius.circular(10),
                          color: Color.fromARGB(118, 33, 149, 243),
                        ),
                        child: Text(
                          "Ready to Use",
                          style: TextStyle(
                            color: Colors.white,
                            fontSize: 32,
                            fontWeight: FontWeight.w900,
                          ),
                        ),
                      )
                    : Container(),
                SizedBox(height: 20),
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
          ],
        ),
      ),
    );
  }

  void onSSROnTimePressed() {
    int value = 0;
    Get.defaultDialog(
      title: 'Set SSR on Time',
      content: NumberStepButton(
        minValue: 5,
        maxValue: 20,
        onChanged: (_value) {
          value = _value;
        },
      ),
      actions: <Widget>[
        TextButton(
          child: Text('Set'),
          onPressed: () {
            setState(() {
              ssrOnTime = value;
            });
            _apiController.setSSROnTime(value);
            Get.back();
          },
        ),
        TextButton(
          child: Text('Cancel'),
          onPressed: () {
            Get.back();
          },
        ),
      ],
    );
  }

  void onPlotterSpeedPressed() {
    int value = 0;
    Get.defaultDialog(
      title: 'Set Plotter Speed',
      content: NumberStepButton(
        minValue: 1,
        onChanged: (_value) {
          value = _value;
        },
      ),
      actions: <Widget>[
        TextButton(
          child: Text('Set'),
          onPressed: () {
            setState(() {
              plotterSpeed = value;
            });
            _apiController.setPlotterSpeed(value);
            Get.back();
          },
        ),
        TextButton(
          child: Text('Cancel'),
          onPressed: () {
            Get.back();
          },
        ),
      ],
    );
  }

  void onTempPressed() {
    int value = 400;
    Get.defaultDialog(
      title: 'Set Temperature',
      content: NumberStepButton(
        minValue: 400,
        maxValue: 450,
        steps: 5,
        onChanged: (_value) {
          value = _value;
        },
      ),
      actions: <Widget>[
        TextButton(
          child: Text('Set'),
          onPressed: () {
            setState(() {
              temprature = value;
            });
            _apiController.setTemperature(value);
            Get.back();
          },
        ),
        TextButton(
          child: Text('Cancel'),
          onPressed: () {
            Get.back();
          },
        ),
      ],
    );
  }

  void onLiquidSpeedPressed() {
    int value = 2;
    Get.defaultDialog(
      title: 'Set Liquid Speed',
      content: NumberStepButton(
        minValue: 2,
        maxValue: 17,
        steps: 1,
        onChanged: (_value) {
          value = _value;
        },
      ),
      actions: <Widget>[
        TextButton(
          child: Text('Set'),
          onPressed: () {
            setState(() {
              pumpSpeed = value;
            });
            _apiController.setPumpSpeed(value);
            Get.back();
          },
        ),
        TextButton(
          child: Text('Cancel'),
          onPressed: () {
            Get.back();
          },
        ),
      ],
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

// bool checkExpiration() {
//   final expirationDate = DateTime(2022, 7, 6);
//   final now = DateTime.now();
//   bool isExpired = now.isAfter(expirationDate);
//   debugPrint('isExpired: $isExpired');
//   return isExpired;
// }

// class ExpiredScreen extends StatelessWidget {
//   const ExpiredScreen({Key? key}) : super(key: key);

//   @override
//   Widget build(BuildContext context) {
//     return Scaffold(
//       body: Center(
//         child: Text('Something went wrong'),
//       ),
//     );
//   }
// }