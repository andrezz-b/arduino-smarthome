import { APIService } from "./services/APIService";
import { TemperatureController } from "./controller/TemperatureController";
import { FanController } from "./controller/FanController";
import { LedController } from "./controller/LedController";
import { TVRemoteController } from "./controller/TVRemoteController";

const SERVER_BASE_URL = "http://192.168.0.162";


const apiService = new APIService(SERVER_BASE_URL);

const temperatureController = new TemperatureController(apiService);
temperatureController.init();

const fanController = new FanController(apiService);
fanController.init();

const ledController = new LedController(apiService);
ledController.init();

const tvRemoteController = new TVRemoteController(apiService);
tvRemoteController.init();