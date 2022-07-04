import 'package:flutter/cupertino.dart';
import 'package:get/get.dart';
import 'package:http/http.dart' as http;

class APIController extends GetxController {
  APIController();
  final String endpoint = 'http://192.168.4.1';

  // final _obj = ''.obs;
  // set obj(value) => _obj.value = value;
  // get obj => _obj.value;

  //make a post request start the process
  Future<void> startProcess() async {
    try {
      var url = Uri.parse(endpoint + '/start');
      final response = await http.post(url, body: {'fun': 'start'});
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

  //make a post request to stop the process
  Future<void> stopProcess() async {
    try {
      var url = Uri.parse(endpoint + '/stop');
      final response = await http.post(url, body: {'fun': 'stop'});
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

  //make a get request to get the current status of the process
  Future<void> getStatus() async {
    try {
      final uri = Uri.parse(endpoint + '/get')
          .replace(queryParameters: {'message': 'starts'});
      final response = await http.get(uri);

      if (response.statusCode == 200) {
        debugPrint('Process Status: ');
      } else {
        debugPrint('StatusCode: ' + response.statusCode.toString());
      }
      debugPrint(response.body);
    } catch (e) {
      debugPrint(e.toString());
    }
  }
}
