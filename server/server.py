from flask import Flask
app = Flask(__name__)

@app.route("/")
def ytfs_hello():
    return "Hiya! This is the Flask web server powering the YTFS data store."

@app.route("/upload", methods=["POST"])
def upload():
    pass

@app.route("/get_file", methods=["GET"])
def get_file():
    pass
