export function checkWebCryptoSupport() {
    return typeof window.crypto != "undefined" && typeof window.crypto.subtle != "undefined" && typeof window.crypto.subtle.encrypt != "undefined" && typeof window.crypto.subtle.decrypt != "undefined" && typeof window.crypto.subtle.deriveKey != "undefined" && typeof window.crypto.getRandomValues != "undefined";
}