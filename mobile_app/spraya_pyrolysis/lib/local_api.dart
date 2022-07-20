import 'package:flutter/cupertino.dart';
import 'package:get/get.dart';
import 'package:http/http.dart' as http;

class APIController extends GetxController {
  APIController();
  static const String _endpoint = 'http://192.168.4.1';
  static const String _paramKey = 'fun';
  // final _obj = ''.obs;
  // set obj(value) => _obj.value = value;
  // get obj => _obj.value;

  //make a get request start the process
  Future<void> startProcess() async {
    try {
      var url = Uri.parse(_endpoint + '/start');
      final response = await http.get(url);
      if (response.statusCode == 200) {
        debugPrint('Process started');
      } else {
        debugPrint('StatusCode: ' + response.statusCode.toString());
      }
      debugPrint(response.body);
    } catch (e) {
      debugPrint(e.toString());
    }
  }

  //make a get request to stop the process
  Future<void> stopProcess() async {
    try {
      var url = Uri.parse(_endpoint + '/stop');
      final response = await http.get(url);
      if (response.statusCode == 200) {
        debugPrint('Process stopped');
      } else {
        debugPrint('StatusCode: ' + response.statusCode.toString());
      }
      debugPrint(response.body);
    } catch (e) {
      debugPrint(e.toString());
    }
  }

  //make a post request to set the temp
  Future<void> setTemperature(int value) async {
    try {
      final uri = Uri.parse(_endpoint + '/temp').replace(queryParameters: {
        _paramKey: value.toString(),
      });
      final response = await http.post(uri);

      if (response.statusCode == 200) {
        debugPrint('Temp set to: ');
      } else {
        debugPrint('StatusCode: ' + response.statusCode.toString());
      }
      debugPrint(response.body);
    } catch (e) {
      debugPrint(e.toString());
    }
  }

  //make a post request to set the pump speed
  Future<void> setPumpSpeed(int value) async {
    try {
      final uri = Uri.parse(_endpoint + '/pump').replace(queryParameters: {
        _paramKey: value.toString(),
      });
      final response = await http.post(uri);

      if (response.statusCode == 200) {
        debugPrint('Pump speed set to: ');
      } else {
        debugPrint('StatusCode: ' + response.statusCode.toString());
      }
      debugPrint(response.body);
    } catch (e) {
      debugPrint(e.toString());
    }
  }

  //make a post request to set the plotter speed
  Future<void> setPlotterSpeed(int value) async {
    try {
      final uri = Uri.parse(_endpoint + '/plotter').replace(queryParameters: {
        _paramKey: value.toString(),
      });
      final response = await http.post(uri);

      if (response.statusCode == 200) {
        debugPrint('Plotter speed set to: ');
      } else {
        debugPrint('StatusCode: ' + response.statusCode.toString());
      }
      debugPrint(response.body);
    } catch (e) {
      debugPrint(e.toString());
    }
  }

  //make a post request to set SSR on time
  Future<void> setSSROnTime(int value) async {
    try {
      final uri = Uri.parse(_endpoint + '/ssrontime').replace(queryParameters: {
        _paramKey: value.toString(),
      });
      final response = await http.post(uri);

      if (response.statusCode == 200) {
        debugPrint('SSR on time set to: ');
      } else {
        debugPrint('StatusCode: ' + response.statusCode.toString());
      }
      debugPrint(response.body);
    } catch (e) {
      debugPrint(e.toString());
    }
  }
}
