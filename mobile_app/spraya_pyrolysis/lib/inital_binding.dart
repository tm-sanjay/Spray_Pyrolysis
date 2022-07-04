import 'package:get/get.dart';
import 'package:spraya_pyrolysis/local_api.dart';

class InitialBinding extends Bindings {
  @override
  void dependencies() {
    Get.lazyPut<APIController>(() => APIController());
  }
}
