import { APIService } from "../services/APIService";
import { disableInputElements, enableInputElements } from "../util/inputElements";

/**
 * @readonly
 * @enum {string}
 */
export const FanMode = {
  Auto: "auto",
  Manual: "manual",
};

/**
 * @readonly
 * @enum {string}
 */
export const FanLevelCommand = {
  Inc: "inc",
  Dec: "dec",
  Max: "max",
  Min: "min",
};

export class FanController {
  /** @type {HTMLInputElement[]} */
  #fanModeElements;
  /** @type {HTMLButtonElement[]} */
  #fanSpeedControls;
  /** @type {HTMLParagraphElement} */
  #fanSpeedElement;
  /** @type {HTMLDivElement} */
  #manualFanControlElement;
  /** @type {FanSpeedBar} */
  #fanSpeedBar;
  /** @type {FanMode} */
  #fanMode;
  /** @type {number} */
  #fanSpeed;
  /** @type {APIService} */
  #api;

  get fanLevel() {
    return this.#fanSpeedBar.currentLevel;
  }

  /** @param {APIService} apiService */
  constructor(apiService) {
    this.#api = apiService;
    this.#fanSpeedElement = document.getElementById("fan-speed");
    this.#fanModeElements = document.querySelectorAll('[name="fan-mode"]');
    this.#fanSpeedControls = document.querySelectorAll("[data-level]");
    this.#fanSpeedBar = new FanSpeedBar(5);
    this.#manualFanControlElement = document.getElementById("manual-fan-control");
    this.#fanMode = FanMode.Auto;
    this.displayManualFanControl(false);
  }

  /**
   * @param {boolean} enable
   */
  displayManualFanControl(enable) {
    if (enable) {
      this.#manualFanControlElement.classList.remove("d-none");
    } else {
      this.#manualFanControlElement.classList.add("d-none");
    }
  }

  /** @param {number} fanSpeed */
  displayFanSpeed(fanSpeed) {
    this.#fanSpeedElement.textContent = `${fanSpeed} RPM`;
  }

  /** @param {FanMode} fanMode */
  displayFanMode(fanMode) {
    this.#fanModeElements.forEach((element) => {
      if (element.value === fanMode) {
        element.checked = true;
      }
    });
  }

  async init(refreshIntervalMs = 2000) {
    this.#fanModeElements.forEach((element) =>
      element.addEventListener("change", this.handleFanMode.bind(this))
    );
    this.#fanSpeedControls.forEach((element) =>
      element.addEventListener("click", this.handleFanSpeed.bind(this))
    );
    await this.updateFanStatus();
    setInterval(() => this.updateFanStatus(), refreshIntervalMs);
    this.displayFanMode(this.#fanMode);
  }

  /**
   * @param {Event} event
   */
  async handleFanMode(event) {
    /** @type {FanMode} */
    const mode = event.currentTarget.value;
    disableInputElements(this.#fanModeElements);
    await this.#api.changeFanMode(mode);
    this.displayManualFanControl(mode === FanMode.Manual);
    enableInputElements(this.#fanModeElements);
  }

  /**
   * @param {Event} event
   */
  async handleFanSpeed(event) {
    /** @type {FanLevelCommand} */
    const level = event.currentTarget.dataset.level;
    disableInputElements(this.#fanSpeedControls);
    await this.#api.changeFanLevel(level);
    await this.updateFanStatus();
    enableInputElements(this.#fanSpeedControls);
  }

  async updateFanStatus() {
    try {
      const { fanSpeed, fanLevel, fanMode } = await this.#api.getFanStatus();

      if (this.#fanMode !== fanMode) {
        this.displayFanMode(fanMode);
      }
      this.#fanMode = fanMode;
      this.#fanSpeed = fanSpeed;
      this.#fanSpeedBar.setLevel(fanLevel);
      this.displayManualFanControl(fanMode === FanMode.Manual);
      this.displayFanSpeed(this.#fanSpeed);
    } catch (error) {
      console.error(`Failed getting fan status ${error}`);
    }
  }
}

class FanSpeedBar {
  /**
   * @type {HTMLElement}
   */
  fanSpeedBarElement;
  /**
   * @type {number}
   */
  currentLevel = 0;
  /**
   * @type {number}
   */
  currentWidth = 0;
  /**
   * @type {number}
   */
  minLevel;
  /**
   * @type {number}
   */
  maxLevel;
  /**
   * @type {number}
   */
  widthPerLevel;
  /**
   * As the fan can't be off completely, the minimum width is 10%
   * @type {number}
   */
  #minWidth = 10;
  /**
   * @param {number} maxLevels
   * @param {number} minLevel
   */
  constructor(maxLevels, minLevel = 0) {
    // Levels are 0 indexed
    this.maxLevel = maxLevels - 1;
    this.minLevel = minLevel;
    this.currentLevel = minLevel;

    this.fanSpeedBarElement = document.getElementById("fan-speed-bar");
    this.widthPerLevel = 100 / this.maxLevel;

    this.currentWidth = this.calculateWidth(this.currentLevel);
    this.changeElementWidth(this.currentWidth);
  }

  increase() {
    if (this.currentLevel >= this.maxLevel) {
      return;
    }
    this.currentLevel++;
    this.currentWidth = this.calculateWidth(this.currentLevel);
    this.changeElementWidth(this.currentWidth);
  }

  decrease() {
    if (this.currentLevel <= this.minLevel) {
      return;
    }
    this.currentLevel--;
    this.currentWidth = this.calculateWidth(this.currentLevel);
    this.changeElementWidth(this.currentWidth);
  }

  /** @param {number} level */
  setLevel(level) {
    if (level < this.minLevel) {
      level = this.minLevel;
    }
    if (level > this.maxLevel) {
      level = this.maxLevel;
    }
    this.currentLevel = level;
    this.currentWidth = this.calculateWidth(this.currentLevel);
    this.changeElementWidth(this.currentWidth);
  }

  /**
   * @param {number} level
   */
  calculateWidth(level) {
    let width = level * this.widthPerLevel;
    if (width > 100) {
      width = 100;
    }
    if (width < 0) {
      width = 0;
    }
    return width;
  }

  /**
   * @param {number} width
   */
  changeElementWidth(width) {
    const widthToSet = width < this.#minWidth ? this.#minWidth : width;
    this.fanSpeedBarElement.style.width = `${widthToSet}%`;
  }
}
