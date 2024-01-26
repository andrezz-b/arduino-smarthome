import PQueue from "p-queue";
import { FanLevelCommand, FanMode } from "../controller/FanController";

export class APIService {
  /** @type {string} */
  #BASE_URL;
  /** @type {PQueue} */
  #requestQueue;

  /**
   * @param {string} baseUrl
   */
  constructor(baseUrl) {
    this.#BASE_URL = baseUrl;
    this.#requestQueue = new PQueue({ concurrency: 1 });
  }

  /**
   * @param {string} command
   */
  async sendTVRemoteCommand(command) {
    return this.#requestQueue.add(() => this.#postTVRemoteCommand(command));
  }
  async getTemperature() {
    return this.#requestQueue.add(() => this.#getTemperature());
  }
  async getFanStatus() {
    return this.#requestQueue.add(() => this.#getFanStatus());
  }

  async getLedStatus() {
    return this.#requestQueue.add(() => this.#getLedStatus());
  }

  /**
   * @param {import("../controller/LedController").LedStatus} status
   */
  async changeLedStatus(status) {
    return this.#requestQueue.add(() => this.#changeLedStatus(status));
  }

  /**
   * @param {FanMode} mode
   */
  async changeFanMode(mode) {
    return this.#requestQueue.add(() => this.#putChangeFanMode(mode));
  }

  /**
   * @returns {Promise<string>} `'1'` if led is on, `'0'` if led is off	
   */
  async #getLedStatus() {
    const response = await fetch(`${this.#BASE_URL}/led/status`);
    return response.text();;
  }

  /**
   * @param {import("../controller/LedController").LedStatus} status
   * @returns {Promise<"success"|"invalid_status">}
   */
  async #changeLedStatus(status) {
    const response = await fetch(`${this.#BASE_URL}/led/change?status=${status}`);
    return response.text();
  }

  /**
   * @param {FanLevelCommand} level
   */
  async changeFanLevel(level) {
    return this.#requestQueue.add(() => this.#changeFanLevel(level));
  }

  /**
   * @param {FanLevelCommand} level
   * @returns {Promise<"level_changed"|"invalid_level">}
   * @private
   * */
  async #changeFanLevel(level) {
    const response = await fetch(`${this.#BASE_URL}/fan/level?level=${level}`);
    return response.text();
  }

  /**
   * @param {FanMode} mode
   * @returns {Promise<"mode_changed"|"invalid_mode">}
   */
  async #putChangeFanMode(mode) {
    const response = await fetch(`${this.#BASE_URL}/fan/mode?mode=${mode}`);
    return response.text();
  }

  async #getFanStatus() {
    const response = await fetch(`${this.#BASE_URL}/fan/status`);
    const data = await response.text();
    const dataSplitArray = data.split(",");
    const fanSpeed = parseInt(dataSplitArray[0]);
    const fanLevel = parseInt(dataSplitArray[1]);
    const fanMode = dataSplitArray[2] === "0" ? FanMode.Auto : FanMode.Manual;
    return { fanSpeed, fanLevel, fanMode };
  }

  /** @returns {Promise<number>} */
  async #getTemperature() {
    const response = await fetch(`${this.#BASE_URL}/temp`);
    const data = await response.text();
    return parseFloat(data);
  }

  /**
   * @param {string} command
   * @returns {Promise<"success"|"inavlid_command">}
   */
  async #postTVRemoteCommand(command) {
    const res = await fetch(`${this.#BASE_URL}/remote?command=${command}`);
    return res.text();
  }
}
