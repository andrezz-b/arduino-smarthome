/**
 * @typedef {HTMLInputElement | HTMLButtonElement} InputElement
 */

/**
 * @param {InputElement | InputElement[]} elements 
 */
export function disableInputElements(elements) {
  const arrayElements = Array.isArray(elements) ? elements : [elements];
  arrayElements.forEach((element) => {
    element.disabled = true;
  });
}

/**
 * @param {InputElement | InputElement[]} elements 
 */
export function enableInputElements(elements) {
  const arrayElements = Array.isArray(elements) ? elements : [elements];
  arrayElements.forEach((element) => {
    element.disabled = false;
  });
}

/**
 * @param {InputElement | InputElement[]} elements 
 */
export function toggleDisabledInputElements(elements) {
  const arrayElements = Array.isArray(elements) ? elements : [elements];
  arrayElements.forEach((element) => {
    element.disabled = !element.disabled;
  });
}