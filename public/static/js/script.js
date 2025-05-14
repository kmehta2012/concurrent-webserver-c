document.addEventListener("DOMContentLoaded", function () {
    console.log("script.js loaded successfully.");
    const p = document.createElement("p");
    p.textContent = "This paragraph was added by script.js.";
    document.body.appendChild(p);
});