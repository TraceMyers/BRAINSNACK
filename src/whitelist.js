document.addEventListener("keydown", e => {
    // whitelist the f12 key, so it won't be consumed by the game / sdl
    // opens up the dev console in chrome
    if (e.key === "F12") {
        e.stopImmediatePropagation();
    }
}, true);
