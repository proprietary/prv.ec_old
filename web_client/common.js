"use strict";

export function listenForUrlHashChange(cb) {
    window.addEventListener('hashchange', function (evt) {
        const fullUrl = new URL(evt.newURL);
        const urlHash = fullUrl.hash;
        if (urlHash.length < 2) {
            return;
        }
        if (urlHash[0] != '#') {
            return;
        }
        const token = urlHash.slice(1);
        cb(token);
    });
}

export function getUrlPass() {
    return window.location.hash.slice(1);
}
