import { APIService } from "../services/APIService";
import { disableInputElements, enableInputElements } from "../util/inputElements";

const LED_STATUS = {
  '0': "Off",
  '1': "On"
};

/**
 * @typedef {keyof LED_STATUS} LedStatus
 */

export class LedController {
  /** @type {HTMLButtonElement} */
  #ledControlElements;

  /** @type {APIService} */
  #api;

  /** @param {APIService} apiService */
  constructor(apiService) {
    this.#api = apiService;
    this.#ledControlElements = document.querySelectorAll('[name="lightcontrol"]');
  }

  async init() {
    this.#ledControlElements.forEach((element) =>
      element.addEventListener("change", this.#handleLedChange.bind(this))
    );
    const ledStatus = await this.#api.getLedStatus();
    this.updateLedStatus(ledStatus);
  }

  /** @param {LedStatus} status */
  updateLedStatus(status) {
    this.#ledControlElements.forEach((element) => {
      if (element.value === status) {
        element.checked = true;
      }
    });
  }

  /** @param {Event} event */
  async #handleLedChange(event) {
    /** @type {LedStatus} */
    const value = event.currentTarget.value;
    disableInputElements(this.#ledControlElements);
    await this.#api.changeLedStatus(value);
    enableInputElements(this.#ledControlElements);
  }
}
