import { APIService } from "../services/APIService";

export class TVRemoteController {
  /** @type {HTMLElement[]} */
  #remoteElements;
  /** @type {APIService} */
  #api;

  /**
   * @param {APIService} apiService
   */
  constructor(apiService) {
    this.#remoteElements = document.querySelectorAll("[data-command]");
    this.#api = apiService;
  }

  init() {
    this.#remoteElements.forEach((element) =>
      element.addEventListener("click", this.handleCommand.bind(this))
    );
  }

  /** @param {Event} event */
  async handleCommand(event) {
    const command = event.currentTarget.dataset.command;
    await this.#api.sendTVRemoteCommand(command);
  }
}
