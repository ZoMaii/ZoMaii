document.addEventListener("DOMContentLoaded", () => {
    loadLinks();
    loadMessages();

    // Back to top button
    const backToTop = document.getElementById("back-to-top");
    window.addEventListener("scroll", () => {
        if (window.scrollY > 200) {
            backToTop.style.display = "block";
        } else {
            backToTop.style.display = "none";
        }
    });

    backToTop.addEventListener("click", () => {
        window.scrollTo({ top: 0, behavior: "smooth" });
    });

    // Submit message form
    document.getElementById("message-form").addEventListener("submit", (e) => {
        e.preventDefault();
        const username = document.getElementById("username").value;
        const message = document.getElementById("message").value;

        fetch("php/add_message.php", {
            method: "POST",
            body: JSON.stringify({ name: username, content: message }),
            headers: { "Content-Type": "application/json" },
        })
            .then((response) => response.json())
            .then((data) => {
                if (data.success) {
                    loadMessages();
                }
            });
    });
});

function loadLinks() {
    fetch("php/get_links.php")
        .then((response) => response.json())
        .then((links) => {
            const carousel = document.getElementById("links-carousel");
            carousel.innerHTML = links
                .map(
                    (link) =>
                        `<div class="card">
                            <h3>${link.name}</h3>
                            <p>${link.description}</p>
                            <a href="${link.url}" target="_blank">Visit</a>
                        </div>`
                )
                .join("");
        });
}

function loadMessages() {
    fetch("php/get_messages.php")
        .then((response) => response.json())
        .then((messages) => {
            const container = document.getElementById("message-container");
            container.innerHTML = messages
                .map(
                    (message) =>
                        `<div class="message">
                            <p><strong>${message.name}</strong></p>
                            <p>${message.content}</p>
                            <p><em>${message.created_at}</em></p>
                        </div>`
                )
                .join("");
        });
}
