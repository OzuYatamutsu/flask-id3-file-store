from flask import Flask, request
from werkzeug import secure_filename
from os import path

PORT = 9880 # Debug
DATA_DIR = "./data"

app = Flask(__name__)
app.config["UPLOAD_FOLDER"] = DATA_DIR

@app.route("/")
def ytfs_hello():
    return "Hiya! This is the Flask web server powering the YTFS data store."

@app.route("/upload", methods=["POST"])
def upload():
    """Uploads a file to the YTFS data store.
    POST to /upload with a file_data field."""
    file = request.files["file_data"]
    if file:
        filename = secure_filename(file.filename)
        file.save(path.join(app.config["UPLOAD_FOLDER"], filename))
        return "Got data! Filename=" + filename # Debug
    return "Upload failure!"

@app.route("/get_file", methods=["GET"])
def get_file():
    pass

if __name__ == "__main__":
    app.run(
        host = "0.0.0.0",
        port = PORT
    )
