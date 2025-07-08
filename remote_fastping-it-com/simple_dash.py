# Just create the file with nano:
nano simple_dash.py

# Then paste in:
from flask import Flask

app = Flask(__name__)

@app.route('/login')
def login():
    return "<h1>LOGIN WORKS!</h1>"

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=8080, debug=True)