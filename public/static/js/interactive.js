document.addEventListener("DOMContentLoaded", function () {
    const input = document.createElement("input");
    input.type = "text";
    input.placeholder = "Type something...";

    const button = document.createElement("button");
    button.textContent = "Submit";

    const output = document.createElement("div");

    button.addEventListener("click", () => {
        output.textContent = `You typed: ${input.value}`;
    });

    document.body.append(input, button, output);
});