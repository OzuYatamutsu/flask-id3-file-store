from flask import Flask, request
from werkzeug import secure_filename
from os import path
import MySQLdb # Case-sensitive. Worst name ever 

PORT = 9880 # Debug
DATA_DIR = "./data"

# Database credentials
DB_HOST = "localhost"
DB_USER = "root" # TODO this is awful
DB_PASSWD = "team14" # TODO this is more awful
DB_DB = "ytfs"

app = Flask(__name__)
app.config["UPLOAD_FOLDER"] = DATA_DIR
db = None
cursor = None

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

def db_connect():
    """Attempts to connect to the configured database.
    Returns a db and a cursor object."""
    db = MySQLdb.connect(
        host = DB_HOST,
        user = DB_USER,
        passwd = DB_PASSWD,
        db = DB_DB
    )

    return db, db.cursor()

def db_insert_file(filename, file):
    """Reads file metadata and inserts it into the database."""
    # TODO: Get metadata below
    track = 0
    title = "TITLE"
    artist = "ARTIST"
    album = "ALBUM"
    year = "YEAR"
    genre = "GENRE"
    track_comment = "COMMENT"
    
    cursor.execute(
        """INSERT INTO ytfs_meta (
            filename, track, title, artist, 
            album, year, genre, track_comment
        ) VALUES (
            '%s', %s, '%s', '%s',
            '%s', '%s', '%s', '%s'
        )""", (
            filename, track, title, artist,
            album, year, genre, track_comment
        )
    )

if __name__ == "__main__":
    db, cursor = db_connect()
    if db: print("Connected to database!")
    app.run(
        host = "0.0.0.0",
        port = PORT
    )
