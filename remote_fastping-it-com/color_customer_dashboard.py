from flask import Flask, request, jsonify, render_template_string
from flask_bootstrap import Bootstrap

app = Flask(__name__)
Bootstrap(app)

dashboard_html = """
<!doctype html>
<html lang="en">
  <head>
    <title>Customer Dashboard</title>
    <link rel="stylesheet" href="{{ url_for('static', filename='bootstrap.min.css') }}">
  </head>
  <body class="bg-dark text-light">
    <div class="container mt-5">
      <h1 class="display-4">Customer Dashboard</h1>
      <hr class="border-light">
      <p>Status: <strong style="color: lime;">Online</strong></p>
      <form id="createForm">
        <div class="form-group">
          <input type="text" class="form-control" id="name" placeholder="Name" required>
          <input type="email" class="form-control mt-2" id="email" placeholder="Email" required>
          <button type="submit" class="btn btn-success mt-3">Create Customer</button>
        </div>
      </form>
      <div id="result" class="mt-4"></div>
    </div>
    
    <script>
      document.getElementById('createForm').addEventListener('submit', async (e) => {
        e.preventDefault();
        const name = document.getElementById('name').value;
        const email = document.getElementById('email').value;
        const res = await fetch('/admin/create_customer', {
          method: 'POST',
          headers: { 'Content-Type': 'application/json' },
          body: JSON.stringify({ name, email })
        });
        const data = await res.json();
        document.getElementById('result').innerHTML = '<pre>' + JSON.stringify(data, null, 2) + '</pre>';
      });
    </script>
  </body>
</html>
"""

@app.route("/")
def home():
    return render_template_string(dashboard_html)

@app.route("/admin/create_customer", methods=["POST"])
def create_customer():
    data = request.json
    return jsonify({"message": "Customer created", "id": 1, "name": data.get("name"), "email": data.get("email")})

if __name__ == "__main__":
    app.run(debug=True, port=8090, host="0.0.0.0")
