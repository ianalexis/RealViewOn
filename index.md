<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Real-View-On-Releases</title>
</head>
<body>
    <h1>Real-View-On-Releases</h1>
    <label for="language-select">Select Language:</label>
    <select id="language-select" onchange="loadContent()">
        <option value="en" selected>English</option>
        <option value="es">Español</option>
    </select>
    <div id="content"></div>

    <script>
        function loadContent() {
            const language = document.getElementById('language-select').value;
            fetch(`docs/${language}.md`)
                .then(response => response.text())
                .then(data => {
                    document.getElementById('content').innerHTML = marked(data);
                });
        }

        // Load default content
        document.addEventListener('DOMContentLoaded', loadContent);
    </script>
    <script src="https://cdn.jsdelivr.net/npm/marked/marked.min.js"></script>
</body>
</html>
