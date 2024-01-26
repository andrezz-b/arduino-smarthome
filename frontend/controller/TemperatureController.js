import { APIService } from "../services/APIService";

export class TemperatureController {
  /** @type {number} */
  #currentTemperature = undefined;
  /** @type {HTMLSpanElement} */
  #tempElement;
  /** @type {APIService} */
  #api;

  /**
   * @param {APIService} apiService
   */
  constructor(apiService) {
    this.#tempElement = document.getElementById("temperature");
    this.#api = apiService;
  }

  get currentTemperature() {
    return this.#currentTemperature;
  }

  async init(refreshIntervalMs = 5000) {
    await this.updateTemperature();
    setInterval(() => this.updateTemperature(), refreshIntervalMs);
  }

  async updateTemperature() {
    try {
      this.#currentTemperature = await this.#api.getTemperature();
      this.#tempElement.textContent = this.#currentTemperature;
    } catch (error) {
      console.error(`Failed getting temperature ${error}`);
    }
  }
}
